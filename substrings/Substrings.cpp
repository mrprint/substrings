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
#include <algorithm>
#include <array>
#include <thread>
#include <execution>
#include "Substrings.hpp"
#include "EntropyCache.hpp"
#include "Matcher.hpp"
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
    auto skip = [](auto i) { return i % TOSKIP == 0; };
    for (size_t start : views::iota( 0u, data.length() - maxl))
    {
        for (size_t length : views::iota((notsize_t)minl, (notsize_t)maxl + 1u) | views::filter(skip))
        {
            DataView subd = data.substr(start, length);
            if (ascii && !is_ascii(subd))
                break;
            float ent = ecache.estimate(subd, start, length);
            if (ent >= MAX_ENT || ent <= MIN_ENT)
                break;
            keys.try_emplace(subd, 0).first->second++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SubstringsConcurrent::SubstringsConcurrent(size_t minl, size_t maxl) : Substrings(minl, maxl) {}

SubstringsConcurrent::~SubstringsConcurrent() {}

void SubstringsConcurrent::process_c(const string& path, bool ascii, size_t scale)
{
    TimeIt time_it("Calculation time is");

    const unsigned procs_count = max(thread::hardware_concurrency(), 1u);
    auto slci = slice_file(path, maxl, procs_count, scale);
    for_each(
        execution::par,
        slci.begin(), slci.end(),
        [&, ascii](pair<size_t, size_t> rng)
        {
            string tdata;
            {
                ifstream f(path, ios::in | ios::binary);
                string fdata(rng.second, '\0');
                f.seekg(rng.first);
                f.read(fdata.data(), rng.second);
                tdata.swap(fdata);
            }
            SubstringsConcurrent subs(minl, maxl);
            subs.process(tdata, ascii);
            subs.accumulate(rkeys);
        }
    );
}

experimental::generator<ResultEl> SubstringsConcurrent::top_c(size_t amount)
{
    top_w(result, rkeys, amount);
    Matcher matcher(0.8);
    for (auto& i :
        result
        | views::filter(
            [&](auto& i)
            {
                bool matchs = matcher.get_close_matches(i.first);
                matcher.append(i.first);
                return !matchs;
            }
        )
        | views::take(amount))
        co_yield i;
}

void SubstringsConcurrent::accumulate(ReducedKeys& rkeys)
{
    for (auto& [key, value] : keys)
    {
        if (value > 1) {
            // tbb::concurrent_unordered_map has no try_emplace
            string skey{ key };
            if (rkeys.contains(skey))
                rkeys[skey] += value;
            else
                rkeys[skey] = value;
        }
    }
}


