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

#include <chrono>
#include <string>
#include <iostream>
#include <format>

class TimeIt final {
protected:
#if defined(TIME_IT)
    std::chrono::high_resolution_clock::time_point stime;
    std::string msg;
#endif
public:
#if defined(TIME_IT)
    TimeIt() : stime(std::chrono::high_resolution_clock::now()) {}
    explicit TimeIt(const std::string& msg) : stime(std::chrono::high_resolution_clock::now()), msg(msg) {}
    ~TimeIt()
    {
        try
        {
            auto ftime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> dur = ftime - stime;
            if (!msg.empty())
                std::cerr << msg << " ";
            std::cerr << std::format("{:%T}", dur) << std::endl;
        }
        catch (...)
        {
        }
    }
#else
    TimeIt() {}
    explicit TimeIt(const std::string& msg) {}
    ~TimeIt() {}
#endif
};