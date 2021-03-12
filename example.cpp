//
//	format		WJ121
//
//	* some examples
//

#include "format.h"

#include <iostream>
#include <ostream>
#include <cstdio>

using std::printf;

// custom class can be format()ed with streaming operator<<
class MyClass {
	int a;

public:
	MyClass(int n) : a(n) { }

	friend std::ostream& operator<<(std::ostream&, const MyClass&);
};

std::ostream& operator<<(std::ostream& os, const MyClass& c) {
	os << "MyClass(" << c.a << ")";
	return os;
}

int main(int argc, char* argv[]) {
	(void)argc;		// unused
	(void)argv;		// unused

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

	MyClass bar(42);
	std::cout << format("{}", bar) << std::endl;

#if 0
	// this will throw std::invalid_argument
	std::cout << format("{", 1) << std::endl;
#endif

#if 0
	// nullptr produces a std::logic_error (for std::string)
	char* p = nullptr;
	std::cout << "nullptr: [" << format(nullptr) << "]" << std::endl;
	std::cout << "nullptr, arg: [" << format(nullptr, 1) << std::endl;
#endif

	return 0;
}

// EOB
