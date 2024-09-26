// The MIT License (MIT)
// 
// Copyright (c) 2023 github.com/mrprint
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <ranges>
#include <fstream>
#include <thread>
#include <algorithm>
#include "Substrings.hpp"
#include "EntropyCache.hpp"
#include "Matcher.hpp"
#include "system.hpp"
#include "timeit.hpp"

using namespace std;
using namespace substrings;

Substrings::Substrings(size_t minl, size_t maxl) : minl(minl), maxl(maxl) {}

Substrings::~Substrings() {}

void Substrings::process_file(const string& path)
{
    {
        ifstream f(path, ios::in | ios::binary);
        const auto sz = filesystem::file_size(path);
        string fdata(sz, '\0');
        f.read(fdata.data(), sz);
        sdata.swap(fdata);
    }
    process(sdata);
}

void Substrings::process(DataView data, bool ascii)
{
    EntropyCache ecache;
    keys.clear();
    auto skip = [](auto i) { return i % TO_SKIP == 0; };
    for (size_t start : views::iota( 0u, data.length() - maxl))
    {
        for (size_t length : views::iota(minl, maxl + 1u) | views::filter(skip))
        {
            DataView subd = data.substr(start, length);
            if (ascii && !is_ascii(subd))
                break;
            float ent = ecache.estimate(subd, start, static_cast<unsigned>(length));
            if (ent >= MAX_ENT || ent <= MIN_ENT)
                break;
            keys[subd]++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SubstringsConcurrent::SubstringsConcurrent(size_t minl, size_t maxl, size_t amount) :
    Substrings(minl, maxl)
    , amount(amount)
    , trunc_cnt(0)
{
    ram_size = get_ram_size();
}

SubstringsConcurrent::~SubstringsConcurrent() {}

generator_ns::generator<ResultEl> SubstringsConcurrent::top_c()
{
    top_w(result, rkeys, amount);
    Matcher matcher(MATCH_RATIO);
    for (const auto& i :
        result
        | views::filter(
            [&](const auto& i)
            {
                bool matchs = matcher.get_close_matches(i.first);
                matcher.append(i.first);
                return !matchs;
            }
        )
        | views::take(amount))
        co_yield i;
}

void SubstringsConcurrent::process_c(const string& path, bool ascii, size_t scale)
{
    const unsigned procs_count = max(thread::hardware_concurrency() * 2, 1u);
    const auto estms = tune_on_size(path, procs_count, static_cast<unsigned>(scale));
    process_body(path, estms, ascii);
}

void SubstringsConcurrent::accumulate(ReducedKeys& rkeys, size_t drop)
{
    for (const auto& [key, value] : keys)
    {
        if (value > drop)
            rkeys[string(key)] += value;
    }
}

SubstringsConcurrent::Estimations SubstringsConcurrent::tune_on_size(const string& path, unsigned pool_size, unsigned scale) const
{
    const auto fsize = filesystem::file_size(path);
    if (fsize / pool_size <= maxl) {
        pool_size = 1;
        scale = 1;
    }
    else if (!scale) {
        if (ram_size) {
            auto ram = ram_size / WORK_MEM_DIV;
            scale = max(
                8u,
                static_cast<unsigned>((fsize * (maxl - minl + 1) * (sizeof(WorkEl) * 5 / 4)) / (ram * TO_SKIP))
            );
        }
        else
            scale = pool_size;
    }
    size_t psize = static_cast<size_t>(pool_size) * scale;
    size_t dv = fsize / psize;
    if (dv < maxl) {
        psize = fsize / maxl;
        dv = fsize / psize;
    }
    size_t md = fsize % psize;
    return { psize, dv, md, pool_size };
}

void SubstringsConcurrent::truncate()
{
    auto vol = calc_reserve();
    auto sz = rkeys.size();
    if (vol >= sz / 2 || sz < (ram_size / KEYS_MEM_DIV) / ((sizeof(ResultEl) * 5 / 4) + (minl + maxl) / 2))
        return;
    size_t minv = numeric_limits<size_t>::max();
    size_t maxv = 0;
    for (const auto& i : rkeys)
    {
        if (i.second < minv)
            minv = i.second;
        if (i.second > maxv)
            maxv = i.second;
    }
    auto lbnd = (maxv - minv) / sz * (sz - vol) + minv;

    // free up space within rkeys for new ones
    erase_if(rkeys, [=](const auto& i) { return i.second <= lbnd; });
}
