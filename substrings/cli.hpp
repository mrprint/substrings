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
#include <mutex>
#include "Substrings.hpp"

extern std::string input_file;
extern std::int64_t top;
extern std::int64_t lmin, lmax;
extern std::int64_t scale;
extern unsigned skip, drop;
extern bool ascii;
extern bool nofilter;

bool handle_args(int argc, char* argv[]);

class ProgressIndicator {
    std::mutex mtx;
    std::size_t total, progress, percent;
    bool updated;

public:

    enum class Phase {
        Work,
        Begin,
        End
    };

    ProgressIndicator() = delete;
    explicit ProgressIndicator(std::size_t total) : total(total), progress(0), percent(0), updated(false) {};
    void update(std::size_t val)
    {
        std::scoped_lock lock(mtx);
        if (val > progress) {
            progress = val;
            auto prc = progress * 100 / total;
            if (prc > percent) {
                percent = prc;
                updated = true;
            }
        }
    }
    void display(Phase phase=Phase::Work);
};
