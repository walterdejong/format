format
======

`format` is a Python-style `format()` function for C++.

I know `std::format()` is in C++20, but this code works in C++11 and upwards.
I also know about `libfmt`, which is a good production-quality project,
but I just wanted to roll my own and see if I could do it.
The result is a perfectly fine and usable `format()` function.
It's only a single include file, and very easy to include in your own code.

This implementation of `format()` supports practically all the Python
`format()` features documented for Python 3.9


Example
-------
Start by including the header;

```cpp
#include "format.h"
```

Some simple examples of use;

```cpp
    std::string s = format("hello, format({}) !", 42);
    printf("%s\n", s.c_str());

    std::cout << format("こんにちは{:c}{:c}", 0x4e16, 0x754c) << std::endl;

    std::cout << format("hex: {:#x}", 0x1337c0de) << std::endl;

    std::cout << format("left-align: |{:<10}|", "text") << std::endl;

    std::cout << format("center: |{:<30}|", "text") << std::endl;
```

See `example.cpp` for a couple of more examples.


Building
--------
Use a C++ compiler, for example:

    clang++ -Wall -std=c++11 example.cpp -o example


Namespacing
-----------
For easiness of use, the `format()` function is not in a namespace.
This is entirely on purpose; you can edit `format.h` to put it into
a namespace if you want.


Copyright and licensing
-----------------------
`format` is free to use under terms described in the MIT license.
See the accompanied LICENSE file.


Copyright (c) 2021 Walter de Jong <walter@heiho.net>
