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
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include "Substrings.hpp"
#include "timeit.hpp"

using namespace std;
using namespace substrings;

void SubstringsConcurrent::process_c(const string& path, bool ascii, size_t scale)
{
    TimeIt time_it("Calculation time is");

    mutex iomtx, accmtx;
    tf::Executor executor;
    tf::Taskflow taskflow;

    const unsigned procs_count = max(thread::hardware_concurrency(), 1u);
    auto slci = slice_file(path, maxl, procs_count, scale);

    taskflow.for_each(slci.begin(), slci.end(),
        [&, ascii](const pair<size_t, size_t>& rng)
        {
            string tdata(rng.second, '\0');
            {
                lock_guard lock(iomtx); // make file access sequential
                ifstream f(path, ios::in | ios::binary);
                f.seekg(rng.first);
                f.read(tdata.data(), rng.second);
            }

            SubstringsConcurrent subs(minl, maxl);
            subs.process(tdata, ascii);

            {
                lock_guard lock(accmtx);
                subs.accumulate(rkeys);
            }
        });

    executor.run(taskflow).get();
    //taskflow.dump(std::cout);
}
