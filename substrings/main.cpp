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
#include <boost/program_options.hpp>
#include <absl/strings/escaping.h>
#include "Substrings.hpp"
#include "timeit.hpp"

namespace po = boost::program_options;

using namespace std;
using namespace substrings;

bool handle_args(int argc, char* argv[]);

static po::options_description desc("General options");
static po::positional_options_description pos_descr;
static po::variables_map vm;

static string input_file;
static int32_t lmin, lmax;
static int32_t top;
static int32_t scale;
static bool ascii;

int main(int argc, char* argv[])
{
    TimeIt time_it("Total time is");

    setlocale(LC_ALL, "");

    try {
        if (!handle_args(argc, argv))
            return 1;

        SubstringsConcurrent subs(lmin, lmax);
#if !defined(_DEBUG) && !defined(DEBUG)
        subs.process_c(input_file, ascii, scale);
        for (auto& [key, value] : subs.top_c(top))
        {
            cout << value << " \t" << absl::CHexEscape(key) << endl;
        }
#else
        subs.process_file(input_file);
        for (auto& [key, value] : subs.top(top))
        {
            cout << value << " \t" << absl::CHexEscape(key) << endl;
        }
#endif
    }
    catch (filesystem::filesystem_error ex) {
        cerr << "Exception occured: " << ex.what() << endl;
    }
    catch (ifstream::failure ex) {
        cerr << "Exception occured: " << ex.what() << endl;
    }
    catch (...) {
        cerr << "Unknown exception occured!" << endl;
    }
    return 0;
}

static bool handle_args(int argc, char* argv[])
{
    desc.add_options()
        ("input,i", po::value<string>(&input_file), "Input file")
        ("top,t", po::value<int32_t>(&top)->default_value(30), "Amount of values to get ( >0 )")
        ("min,m", po::value<int32_t>(&lmin)->default_value(15), "Minimal length of strings to search ( >6 )")
        ("max,x", po::value<int32_t>(&lmax)->default_value(30), "Maximal length of strings to search ( >min )")
        ("ascii,a", po::value<bool>(&ascii)->default_value(false), "Search for ascii strings only")
        ("scale,s", po::value<int32_t>(&scale)->default_value(8), "Multithread loading scale factor ( >0 )")
        ;

    pos_descr.add("input", -1);

    auto print_desc = []() { cerr << desc << endl; };

    try {
        po::parsed_options parsed =
            po::command_line_parser(argc, argv)
            .options(desc)
            .positional(pos_descr)
            .run();
        po::store(parsed, vm);
        po::notify(vm);
        if (input_file.empty() || top < 1 || lmin < 7 || lmax < lmin || scale < 1) {
            print_desc();
            return false;
        }
    }
    catch (exception&) {
        print_desc();
        return false;
    }
    return true;
}

