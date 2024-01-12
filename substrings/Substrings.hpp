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

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <ranges>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <experimental/generator>
#include <parallel_hashmap/phmap.h>
#include <tbb/concurrent_unordered_map.h>

namespace substrings
{

    constexpr auto MAX_ENT = 3.5f;
    constexpr auto MIN_ENT = 2.5f;
    constexpr auto TOSKIP = 3;

    using Data = std::string;
    using DataView = std::string_view;
    using DataSize = std::streamsize;
    using Keys = phmap::flat_hash_map<DataView, std::size_t>;
    using ResultEl = std::pair<Data, std::size_t>;
    using Result = std::vector<ResultEl>;
    using ReducedKeys = tbb::concurrent_unordered_map<Data, std::size_t>;

    class Substrings
    {
    protected:
        Data sdata;
        Keys keys;
        Result result;
        DataSize minl, maxl;
    public:
        Substrings(DataSize minl, DataSize maxl);
        void process_file(const std::string& path);
        void process(DataView data, bool ascii = false);
        auto top(std::size_t amount)
        {
            top_w(result, keys, amount);
            return std::ranges::subrange(result.begin(), result.begin() + amount); // for removing
        }
    protected:
        static bool is_ascii(DataView data)
        {
            return !std::ranges::any_of(data, [](std::uint8_t c) {return c > 127; });
        }
        void top_w(auto& result, const auto& keys, std::size_t amount)
        {
            result.resize(maxl * amount);
            partial_sort_copy(
                keys.begin(), keys.end(), result.begin(), result.end(),
                [](auto& l, auto& r) { return (l.second == r.second) ? l.first.length() > r.first.length() : l.second > r.second; }
            );
        }
    };

    class SubstringsConcurrent : public Substrings {
    protected:
        ReducedKeys rkeys;
    public:
        SubstringsConcurrent(DataSize minl, DataSize maxl);
        void process_c(const std::string& path, bool ascii = false, DataSize scale = 1);
        std::experimental::generator<ResultEl> top_c(std::size_t amount);
    protected:
        void accumulate(ReducedKeys& rkeys);
        static auto slice_file(const std::string& path, DataSize maxl, DataSize pool_size, DataSize scale = 1)
        {
            const DataSize psize = pool_size * scale;
            const DataSize fsize = std::filesystem::file_size(path);
            const auto dv = fsize / psize;
            const auto md = fsize % psize;
            return std::views::iota((DataSize)0, psize)
                | std::views::transform([=](auto i) {return std::pair<DataSize, DataSize>{
                (i == 0) ? i * dv : i * dv - maxl,
                    (i < psize - 1) ? ((i == 0) ? dv : dv + maxl) : dv + maxl + md}; });
        }
    };

}
