# An easy way to analyse command line in C++.

[![Build Status](https://travis-ci.org/mutouyun/cpp-cmdline.svg?branch=master)](https://travis-ci.org/mutouyun/cpp-cmdline)
[![Build status](https://ci.appveyor.com/api/projects/status/e8ci4mtpyb1gy936/branch/master?svg=true)](https://ci.appveyor.com/project/mutouyun/cpp-cmdline)

This is a simple library for C++ to analyse command line.  
This library only depends on STL.
# Compiler Support
 - MSVC-2015  
 - g++-4.8.1(-std=c++14)  
 - clang-3.4(-std=c++14)

# License
Codes covered by the MIT License.
# Tutorial
For using it, you only need to include cmdline.hpp.  
 
<b>Simple usage:</b>
```cpp
#include "cmdline.hpp"

int main(int argc, char* argv[])
{
    cmdline::parser a;
    a.push(cmdline::options
    {
        {
            "-h",                                   // short name
            "--help",                               // long name
            "Print usage.",                         // description
            false,                                  // is necessary or not
            "",                                     // default value
            [](auto& a, auto&) { a.print_usage(); } // handle for doing something
        },
        {
            "-t", "--test", "You must use this option.", true, "", 
            [](auto&, auto&) { /*Do Nothing.*/ }
        },
        {
            "-o", "--output", "Print text.", true, "Hello World!",
            [](auto&, auto& str) {
                std::cout << str << std::endl;
            }
        }
    });
    a.exec(argc, argv);
    return 0;
}
```
And there are the execution results:
```
$ ./ut
Usage: ut --test --output=Hello World! [OPTIONS]...
Options: 
  -t, --test	You must use this option.
  -o, --output	Print text.[=Hello World!]
  -h, --help	Print usage.
```
```
$ ./ut -h
Usage: ut --test --output=Hello World! [OPTIONS]...
Options: 
  -t, --test	You must use this option.
  -o, --output	Print text.[=Hello World!]
  -h, --help	Print usage.
```
```
$ ./ut -o=fdagdag
Usage: ut --test --output=Hello World! [OPTIONS]...
Options: 
  -t, --test	You must use this option.
  -o, --output	Print text.[=Hello World!]
  -h, --help	Print usage.
```
```
$ ./ut -o=fdagdag -t
fdagdag
```
```
$ ./ut -o -t
Hello World!
```