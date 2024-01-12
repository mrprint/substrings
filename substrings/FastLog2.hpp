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

#include <cmath>
#include <array>
#include <bit>

namespace myfastmath
{
    constexpr int Log2PrecisionLevel = 14;
    constexpr int Log2TableSize = 1 << Log2PrecisionLevel;

    template<typename T>
    class Log2 final {};

    template<>
    class Log2<float> final
    {
    protected:
        std::array<float, Log2TableSize> log_table;
    public:
        Log2()
        {
            for (int i = 0; i < Log2TableSize; i++) {
                log_table[i] = std::log2(1 + static_cast<float>(i) / Log2TableSize);
            }
        }

        float log2(float x) { // x>0
            auto t = std::bit_cast<std::int32_t>(x);
            int exp = (t >> 23) - 0x7f;
            int mantissa = (t >> (23 - Log2PrecisionLevel)) & (Log2TableSize - 1);
            return exp + log_table[mantissa];
        }
    };

    template<>
    class Log2<double> final
    {
    protected:
        std::array<double, Log2TableSize> log_table;
    public:
        Log2()
        {
            for (int i = 0; i < Log2TableSize; i++) {
                log_table[i] = std::log2(1 + static_cast<double>(i) / Log2TableSize);
            }
        }

        double log2(double x) { // x>0
            auto t = std::bit_cast<std::int64_t>(x);
            int exp = (t >> 52) - 0x3ff;
            int mantissa = (t >> (52 - Log2PrecisionLevel)) & (Log2TableSize - 1);
            return exp + log_table[mantissa];
        }
    };
}