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
#include <string_view>
#include <ranges>
#include <algorithm>
#include <filesystem>

#if defined(_MSC_BUILD)
#include <experimental/generator>
namespace generator_ns = std::experimental;
#else
#include <generator>
namespace generator_ns = std;
#endif

#include <phmap.h>

namespace substrings
{

    constexpr auto MAX_ENT = 3.5f;
    constexpr auto MIN_ENT = 2.5f;
    constexpr auto MATCH_RATIO = 0.8;
    constexpr auto TO_SKIP = 3u;
    constexpr auto TRUNC_EVERY = 8u;
    constexpr auto WORK_MEM_DIV = 2u;
    constexpr auto KEYS_MEM_DIV = 5u;

    using Data = std::string;
    using DataView = std::string_view;
    using Keys = phmap::flat_hash_map<DataView, std::size_t>;
    using WorkEl = std::pair<DataView, std::size_t>;
    using ResultEl = std::pair<Data, std::size_t>;
    using Result = std::vector<ResultEl>;
    using ReducedKeys = phmap::flat_hash_map<Data, std::size_t>;

    class Substrings
    {
    protected:
        Data sdata;
        Keys keys;
        Result result;
        std::size_t minl, maxl;
    public:
        Substrings(std::size_t minl, std::size_t maxl);
        virtual ~Substrings();
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
        size_t calc_reserve(std::size_t amount) const
        {
            return amount * maxl * (maxl - minl + 1) / TO_SKIP;
        }
        void top_w(auto& result, const auto& keys, std::size_t amount)
        {
            result.resize(std::min(keys.size(), calc_reserve(amount)));
            partial_sort_copy(
                keys.begin(), keys.end(), result.begin(), result.end(),
                [](auto& l, auto& r) { return (l.second == r.second) ? l.first > r.first : l.second > r.second; }
            );
        }
    };

    class SubstringsConcurrent: public Substrings {
    protected:
        struct Estimations {
            std::size_t psize, dv, md;
            unsigned pool_size;
        };

        ReducedKeys rkeys;
        std::size_t ram_size;
        std::size_t amount;
        unsigned trunc_cnt;
    public:
        SubstringsConcurrent(std::size_t minl, std::size_t maxl, std::size_t amount);
        virtual ~SubstringsConcurrent();
        void process_c(const std::string& path, bool ascii = false, std::size_t scale = 1);
        generator_ns::generator<ResultEl> top_c();
    protected:
        size_t calc_reserve() const
        {
            return Substrings::calc_reserve(amount);
        }
        void process_body(const std::string& path, const Estimations& estms, bool ascii = false);
        void accumulate(ReducedKeys& rkeys, std::size_t drop = 1);
        Estimations tune_on_size(const std::string& path, unsigned pool_size, unsigned scale) const;
        void truncate();
        static auto slice(const Estimations estm, std::size_t maxl)
        {
            return std::views::iota(static_cast<std::size_t>(0), estm.psize)
                | std::views::transform([=](auto i) {return std::pair<std::size_t, std::size_t>{
                (i == 0u) ? i * estm.dv : i * estm.dv - maxl,
                    (i < estm.psize - 1u) ? ((i == 0u) ? estm.dv : estm.dv + maxl) : estm.dv + maxl + estm.md}; });
        }
    };

}
