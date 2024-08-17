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

#include <algorithm>
#include "Matcher.hpp"

#define DIFFLIB_ENABLE_EXTERN_MACROS
#include "thirdpts/difflib.h"

using namespace std;

constexpr auto RESERVE = 100u;

DIFFLIB_INSTANTIATE_FOR_TYPE(substrings::DataView);

Matcher::Matcher(double ratio) : ratio(ratio)
{
    data.reserve(RESERVE);
}

Matcher::~Matcher() {}

void Matcher::append(substrings::DataView str)
{
    data.push_back(str);
}

bool Matcher::get_close_matches(substrings::DataView str)
{
    auto matcher = difflib::MakeSequenceMatcher(substrings::DataView(), str);
    for (const auto& i : data | views::reverse)
    {
        matcher.set_seq1(i);
        if (matcher.ratio() >= ratio)
            return true;
    }
    return false;
}