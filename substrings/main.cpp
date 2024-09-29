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
#include <cxxopts.hpp>
#include <absl/strings/escaping.h>
#include "Substrings.hpp"
#include "timeit.hpp"

using namespace std;
using namespace substrings;

static bool handle_args(int argc, char* argv[]);

static string input_file;
static int64_t top;
static int64_t lmin, lmax;
static int64_t scale;
static unsigned skip, drop;
static bool ascii;
static bool nofilter;

int main(int argc, char* argv[])
{
    TimeIt time_it("Total time is");

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

static bool handle_args(int argc, char* argv[])
{
    cxxopts::Options options("substrings", "The tool designed to find the most frequently occurring sequences in a gigabyte binary file");
    options.add_options()
        ("input", "Input file", cxxopts::value<string>())
        ("t,top", format("Amount of values to get ( 0 < x < {} )", numeric_limits<unsigned>::max()), cxxopts::value<int64_t>()->default_value("30"))
        ("m,min", format("Minimal length of strings to search ( 6 < x < {} )", numeric_limits<unsigned>::max()), cxxopts::value<int64_t>()->default_value("15"))
        ("x,max", format("Maximal length of strings to search ( min < x < {} )", numeric_limits<unsigned>::max()), cxxopts::value<int64_t>()->default_value("30"))
        ("k,skip", format("Lengths to skip for probing step ( 0 < x < {} )", numeric_limits<unsigned>::max()), cxxopts::value<unsigned>()->default_value("3"))
        ("d,drop", format("Maximal volume of occurences to not accumulate ( 0 <= x < {} )", numeric_limits<unsigned>::max()), cxxopts::value<unsigned>()->default_value("1"))
        ("a,ascii", "Search for ascii strings only", cxxopts::value<bool>()->default_value("false"))
        ("f,nofilter", "Do not prefilter by entropy index", cxxopts::value<bool>()->default_value("false"))
        ("s,scale", "Multi-threaded load scaling factor. Using 0 means trying to calculate it heuristically", cxxopts::value<int64_t>()->default_value("0"));

    auto print_desc = [&]() { cerr << options.help() << endl; };

    try {
        options.parse_positional("input");
        auto result = options.parse(argc, argv);

        input_file = result["input"].as<string>();
        top = result["top"].as<int64_t>();
        lmin = result["min"].as<int64_t>();
        lmax = result["max"].as<int64_t>();
        skip = result["skip"].as<unsigned>();
        drop = result["drop"].as<unsigned>();
        ascii = result["ascii"].as<bool>();
        nofilter = result["nofilter"].as<bool>();
        scale = result["scale"].as<int64_t>();

        if (input_file.empty() || top < 1 || lmin < 7 || lmax < lmin || skip < 1) {
            print_desc();
            return false;
        }
    }
    catch (cxxopts::exceptions::exception&) {
        print_desc();
        return false;
    }
    return true;
}

