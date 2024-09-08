# The tool designed to find the most common sequences in a binary file of gigabytes in size

Originally, it was desighned to analyze a crush dump.
You require a c++ compiler with standard 23's generator support, e.g. g++14 or higher.

#### Usage

Run to view all options. You may require to set up your desired range of length
and limit the number of results.

#### Compiling

Initialize submodules with command
>git submodule update --init --recursive

Tt's known that this tool can be built in Windows, using MSVC or MinGW-w64,
as well as on Linux.
