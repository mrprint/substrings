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

#include <array>
#include <limits>
#include "EntropyCache.hpp"

using namespace std;
using namespace substrings;

constexpr auto RESTORE_PER = 1000;

myfastmath::Log2<float> fastlog2;

EntropyCache::EntropyCache() :
    freqs{},
    entropy(0.0f),
    cstart(numeric_limits<size_t>::max()),
    clength(numeric_limits<size_t>::max()),
    counter(0),
    chr(0)
{}

EntropyCache::~EntropyCache() {}

float EntropyCache::estimate(DataView data, size_t start, size_t length)
{
    bool need_save = counter % RESTORE_PER == 0;
    if (start - cstart == 1 && length == clength && !need_save) [[unlikely]] {
        ++counter;
        {
            auto& tfrq = freqs[chr];
            if (tfrq.freq > 0) [[unlikely]] {
                entropy += tfrq.entropy;
                tfrq.freq--;
                if (tfrq.freq > 0) [[likely]] {
                    float te = calculate(tfrq.freq, clength);
                    tfrq.entropy = te;
                    entropy -= te;
                }
                else [[unlikely]] {
                    tfrq.entropy = 0.0;
                }
            }
        }
        {
            auto& tfrq = freqs[static_cast<uint8_t>(data.back())];
            if (tfrq.freq > 0) [[unlikely]] {
                entropy += tfrq.entropy;
            }
            tfrq.freq++;
            float te = calculate(tfrq.freq, clength);
            tfrq.entropy = te;
            entropy -= te;
        }
        chr = data[0];
        cstart++;
    }
    else [[likely]] {
        if (start > cstart || cstart == numeric_limits<size_t>::max()) {
            ++counter;
            entropy = shannon_entropy_save(data);
            cstart = start;
            clength = length;
        }
        else {
            return shannon_entropy(data);
        }
    }
    return entropy;
}

float EntropyCache::shannon_entropy(DataView data)
{
    float ent = 0.0;
    array<size_t, 256> lfreqs{};

    if (data.length() < 2) [[unlikely]]
        return ent;
    float size = data.length();
    for (uint8_t c : data) [[likely]]
        lfreqs[c]++;
    for (size_t i : lfreqs)
    {
        if (i != 0) [[unlikely]]
            ent -= calculate(i, size);
    }
    return ent;
}

float EntropyCache::shannon_entropy_save(DataView data)
{
    float ent = 0.0;
    freqs = {};
    chr = data[0];

    if (data.length() < 2) [[unlikely]] {
        return ent;
    }
    float size = data.length();
    for (uint8_t c : data) [[likely]]
        freqs[c].freq++;
    for (size_t i = 0; i < freqs.size(); ++i)
    {
        auto& tfrq = freqs[i];
        if (tfrq.freq != 0) [[unlikely]] {
            tfrq.entropy = calculate(tfrq.freq, size);
            ent -= tfrq.entropy;
        }
    }
    return ent;
}