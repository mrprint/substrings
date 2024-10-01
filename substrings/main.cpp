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

#include <iostream>
#include <fstream>
#include <filesystem>
#include <clocale>
// #include <format>
#include <absl/strings/escaping.h>
#include "Substrings.hpp"
#include "cli.hpp"
#include "timeit.hpp"

using namespace std;
using namespace substrings;

int main(int argc, char* argv[])
{
    TimeIt time_it("Total time is");

    ios_base::sync_with_stdio(false);
    setlocale(LC_ALL, "");

    try {
        if (!handle_args(argc, argv))
            return 1;

        SubstringsConcurrent subs(lmin, lmax, skip, drop, top);
#if !defined(_DEBUG) && !defined(DEBUG)
        subs.process_c(input_file, ascii, !nofilter, scale);
        for (auto&& [key, value] : subs.top_c())
        {
            cout << value << " \t" << absl::CHexEscape(key) << '\n';
            // cout << value << " \t" << format("[{:?}]", key) << '\n'; // requires c++23
        }
#else
        subs.process_file(input_file);
        for (auto&& [key, value] : subs.top(top))
        {
            cout << value << " \t" << absl::CHexEscape(key) << '\n';
            // cout << value << " \t" << format("[{:?}]", key) << '\n'; // requires c++23
        }
#endif
        cout.flush();
    }
    catch (const exception &ex) {
        cerr << "Exception occured: " << ex.what() << endl;
    }
    catch (...) {
        cerr << "Unknown exception occured!" << endl;
    }
    return 0;
}


