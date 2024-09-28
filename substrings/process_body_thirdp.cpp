﻿// The MIT License (MIT)
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
#include <mutex>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include "Substrings.hpp"
#include "timeit.hpp"

using namespace std;
using namespace substrings;

void SubstringsConcurrent::process_body(const string& path, const Estimations& estms, bool ascii, bool filter)
{
    TimeIt time_it("Calculation time is");

    mutex iomtx, accmtx;
    tf::Executor executor(estms.pool_size);
    tf::Taskflow taskflow;

    auto slci = slice(estms, maxl);
    taskflow.for_each(slci.begin(), slci.end(),
        [&, ascii](const pair<size_t, size_t>& rng)
        {
            try
            {
                string tdata(rng.second, '\0');
                {
                    scoped_lock lock(iomtx); // make file access sequential
                    ifstream f(path, ios::in | ios::binary);
                    f.seekg(rng.first);
                    f.read(tdata.data(), rng.second);
                }

                SubstringsConcurrent subs(minl, maxl, to_skip, drop_volume, amount);
                subs.process(tdata, ascii, filter);

                {
                    scoped_lock lock(accmtx);
                    subs.accumulate(rkeys);
                    if (++trunc_cnt >= TRUNC_EVERY) {
                        trunc_cnt = 0;
                        truncate();
                    }
                }
            }
            catch (const exception& ex) {
                cerr << "Exception occured: " << ex.what() << endl;
                throw;
            }
            catch (...) {
                cerr << "Unknown exception occured!" << endl;
                throw;
            }
        });

    executor.run(taskflow).get();
    //taskflow.dump(std::cout);
}
