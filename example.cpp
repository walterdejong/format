//
//	format		WJ121
//
//	* some examples
//

#include "format.h"

#include <iostream>
#include <cstdio>

using std::printf;


int main(int argc, char* argv[]) {
	printf("hello, world!\n");

	std::string s = format("hello format({})", 42);
	printf("%s\n", s.c_str());

	s = format("comma: {:,}", 1234567890);
	printf("%s\n", s.c_str());

	s = format("underscore: {:_}", 1234567890);
	printf("%s\n", s.c_str());

	s = format("comma fp: {:,.3f}", 12345678.90123);
	printf("%s\n", s.c_str());

	s = format("percentage: {:.1%}", 0.15);
	printf("%s\n", s.c_str());

	s = format("hex: {:#x}", 0x1337c0de);
	printf("%s\n", s.c_str());

	s = format("hex, with grouping: {:#_x}", 0x1337c0de);
	printf("%s\n", s.c_str());

	s = format("bits, with grouping: {:#_b}", (unsigned char)7);
	printf("%s\n", s.c_str());

	bool b = true;
	s = format("bool b: {}", b);
	printf("%s\n", s.c_str());

	s = format("left:   |{:<10}|", "left");
	printf("%s\n", s.c_str());

	s = format("right:  |{:>10}|", "right");
	printf("%s\n", s.c_str());

	s = format("center: |{:^10}|", "center");
	printf("%s\n", s.c_str());

	// or simply without printf()

	std::cout << format("こんにちは{:c}{:c}", 0x4e16, 0x754c) << std::endl;

	std::cout << format("{{ curly braces }}") << std::endl;

	// this will throw std::invalid_argument
//	std::cout << format("{", 1) << std::endl;

	return 0;
}

// EOB
