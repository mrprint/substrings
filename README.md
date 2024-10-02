# The tool designed to find FAST the most common sequences in a binary file of gigabytes in size

It was originally intended for crash dump analysis, so the values ​​obtained are approximate.
However, you can ask the tool to be more precise.

You require a c++ compiler with standard 23's generator support, e.g. g++14 or higher.

#### Usage

Run to view all options. In most cases, you will only need to specify the path to the file.

#### Compiling

Initialize submodules with command
>git submodule update --init --recursive

Tt's known that this tool can be built in Windows, using MSVC or MinGW-w64,
as well as on Linux.
