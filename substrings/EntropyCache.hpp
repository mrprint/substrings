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

#include <array>
#include <cmath>
#include "Substrings.hpp"
#include "FastLog2.hpp"

extern myfastmath::Log2<float> fastlog2;

class EntropyCache final
{
protected:

    struct Item {
        float entropy;
        std::size_t freq;
    };

    std::array<Item, 256> freqs;
    float entropy;
    substrings::DataSize cstart, clength;
    substrings::DataSize counter;
    uint8_t chr;
public:
    EntropyCache();
    ~EntropyCache();
    float estimate(substrings::DataView data, substrings::DataSize start, substrings::DataSize length);
protected:
    float shannon_entropy(substrings::DataView data);
    float shannon_entropy_save(substrings::DataView data);
    float calculate(std::size_t freq, auto length) {
        float frq = freq / static_cast<float>(length);
        return frq * fastlog2.log2(frq); // std::log(frq) / std::log(2.0) // std::log2 is too slow
    }
};

