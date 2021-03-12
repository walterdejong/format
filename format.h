//
//  format.h      WJ121
//
//  * format() string with "{}" syntax
//	* std::format() is in C++20, consider this a backport for C++11 and up
//
/*
Copyright (c) 2021 Walter de Jong <walter@heiho.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef FORMAT_H_WJ121
#define FORMAT_H_WJ121	1

#include <string>
#include <sstream>
#include <ios>
#include <iomanip>
#include <utility>
#include <locale>
#include <stdexcept>
#include <cstdint>
#include <cassert>

//namespace something {

struct format_numpunct__ : public std::numpunct<char> {
	char sep;
	int8_t groupsize;
	std::string group;

	format_numpunct__(char sepchar, int8_t gsize) : sep(sepchar), groupsize(gsize) {
		std::ostringstream ss;
		ss << (char)gsize;
		group = ss.str();
	}

	char do_decimal_point(void) const { return '.'; }
	char do_thousands_sep(void) const { return sep; }
	std::string do_grouping(void) const { return group; }	// group by # digits
};

struct format_spec__ {
	// format specifier

	bool did_change;		// std::ostringstream flags did change
	bool alternate;			// use "alternate" number formatting
	bool center;			// text must be centered
	int width;				// width specifier (only really needed for center)
	char fillchar;			// fill for alignment (only really needed for center)
	char grouping;			// separator for grouping
	char fmt_type;			// formatting type (needed for binary, utf-8, percentage)

	format_spec__() : did_change(false), alternate(false), center(false), width(0), fillchar(' '), grouping(' '), fmt_type('s') { }

	static const char* parse_int(const char* fmt, int* n) {
		// parse integer and return pointer to first non-digit character
		// upon return, *n will have been set
		// no exception is thrown for invalid integer value; not convenient here

		assert(fmt != nullptr);
		assert(n != nullptr);

		*n = 0;

		// this function returns as soon as we hit a different (non-digit) character
		while (*fmt >= '0' && *fmt <= '9') {
			*n *= 10;
			*n += (*fmt - '0');
			fmt++;
		}
		return fmt;
	}

	const char* parse(const char* fmt, std::ostringstream& ss) {
		assert(fmt != nullptr);
		assert(*fmt == '{');	// how else did we get here?
		fmt++;

		if (*fmt == '}') {
			fmt++;
			return fmt;
		}

		if (*fmt == '\0') {
			throw std::invalid_argument("invalid format string");
		}

		if (*fmt != ':') {
			throw std::invalid_argument("invalid format string");
		}
		fmt++;

		// fill
		if (fmt[1] == '<' || fmt[1] == '>' || fmt[1] == '=' || fmt[1] == '^') {
			fillchar = fmt[0];
			fmt++;
		}

		// alignment
		switch (*fmt) {
			case '}':
				fmt++;
				return fmt;

			case '<':		// left justify
				did_change = true;
				ss.setf(std::ios::left, std::ios::adjustfield);
				ss.fill(fillchar);
				fmt++;
				break;

			case '>':		// right justify
				did_change = true;
				ss.setf(std::ios::right, std::ios::adjustfield);
				ss.fill(fillchar);
				fmt++;
				break;

			case '=':		// internal justify
				did_change = true;
				ss.setf(std::ios::internal, std::ios::adjustfield);
				ss.fill(fillchar);
				fmt++;
				break;

			case '^':		// center
				center = true;
				fmt++;
				break;

			case '\0':
				throw std::invalid_argument("invalid format string");

			default:
				;
		}

		// sign
		switch (*fmt) {
			case '}':
				fmt++;
				return fmt;

			case '+':		// enable sign on both positive and negative numbers
				did_change = true;
				ss.setf(std::ios::showpos);
				fmt++;
				break;

			case '-':		// minus sign for negative numbers
				// this already always happens
				fmt++;
				break;

			case '\0':
				throw std::invalid_argument("invalid format string");

			default:
				;

		}

		// '#' flag : alternate form for numbers
		switch (*fmt) {
			case '}':
				fmt++;
				return fmt;

			case '#':			// alternate form
				// hex prints leading "0x"
				// octal print leading "0"
				// floating point numbers will always have a decimal point
				did_change = true;
				alternate = true;
				ss.setf(std::ios::boolalpha|std::ios::showbase|std::ios::showpoint);
				fmt++;
				break;

			case '\0':
				throw std::invalid_argument("invalid format string");

			default:
				;
		}

		// width
		if (*fmt == '0') {
			// zerofill
			did_change = true;
			ss.setf(std::ios::internal, std::ios::adjustfield);
			fillchar = '0';
			ss.fill('0');
		}
		if (*fmt >= '0' && *fmt <= '9') {
			// width specifier
			did_change = true;
			fmt = format_spec__::parse_int(fmt, &width);
			// note: if center, do not manipulate the output stream now
			// we will center the (sub)string later
			if (!center) {
				ss.width(width);
			}
		}

		if (*fmt == ',') {
			// comma: comma separator for large numbers
			did_change = true;
			grouping = ',';
			fmt++;
		}
		if (*fmt == '_') {
			// underscore: thousands separator for large numbers
			did_change = true;
			grouping = '_';
			fmt++;
		}
		if (*fmt == '.') {
			fmt++;
			// floating point precision
			did_change = true;
			int precision;
			fmt = format_spec__::parse_int(fmt, &precision);
			ss.precision(precision);
		}

		// set the grouping on the stream
		if (grouping != ' ') {
			// some formats we group by 3 digits
			// some formats we group by 4 digits
			// explicit string format we don't group at all
			switch (*fmt) {
				case 's':		// string
					// don't group at all
					break;

				case 'x':		// hex
				case 'X':		// hex uppercase
				case 'o':		// octal
					// group by 4 digits
					{
						std::locale custom_locale(std::locale(), new format_numpunct__(grouping, 4));
						ss.imbue(custom_locale);
					}
					break;

				case 'b':		// binary
					// skip; we do the grouping by ourselves in sformat_binary__()
					break;

				case 'c':		// UTF-8 character
					// don't group at all
					break;

				default:
					// group by 3 digits
					{
						std::locale custom_locale(std::locale(), new format_numpunct__(grouping, 3));
						ss.imbue(custom_locale);
					}
			}
		}

		// type
		fmt_type = *fmt;
		switch (*fmt) {
			case '}':
				fmt++;
				return fmt;

			case 's':		// string
				break;

			case 'n':		// number in current locale
				// not implemented as such
			case 'd':		// decimal
			case 'i':
			case 'u':
				ss.setf(std::ios::dec, std::ios::basefield);
				break;

			case 'x':		// hexadecimal
				did_change = true;
				ss.setf(std::ios::hex, std::ios::basefield);
				break;

			case 'X':		// uppercase hexadecimal
				did_change = true;
				ss.setf(std::ios::uppercase);
				ss.setf(std::ios::hex, std::ios::basefield);
				break;

			case 'o':		// octal
				did_change = true;
				ss.setf(std::ios::oct, std::ios::basefield);
				break;

			case 'f':		// floating point (fixed notation)
				ss.setf(std::ios::fixed, std::ios::floatfield);
				ss.setf(std::ios::dec, std::ios::basefield);
				break;

			case 'F':		// fixed with uppercase NAN and INF
				did_change = true;
				ss.setf(std::ios::uppercase);
				ss.setf(std::ios::fixed, std::ios::floatfield);
				ss.setf(std::ios::dec, std::ios::basefield);
				break;

			case 'p':		// pointer (print as hex)
				did_change = true;
				ss.setf(std::ios::showbase);
				ss.setf(std::ios::hex, std::ios::basefield);
				break;

			case 'e':		// floating point scientific notation
				did_change = true;
				ss.setf(std::ios::scientific, std::ios::floatfield);
				ss.setf(std::ios::dec, std::ios::basefield);
				break;

			case 'E':		// scientific notation with uppercase
				did_change = true;
				ss.setf(std::ios::uppercase);
				ss.setf(std::ios::scientific, std::ios::floatfield);
				ss.setf(std::ios::dec, std::ios::basefield);
				break;

			case 'g':		// general format scientific floating point
				did_change = true;
				ss.setf(std::ios::dec, std::ios::basefield);
				ss.unsetf(std::ios::floatfield);		// set default behavior
				break;

			case 'G':		// general format scientific floating point with uppercase
				did_change = true;
				ss.setf(std::ios::uppercase);
				ss.setf(std::ios::dec, std::ios::basefield);
				ss.unsetf(std::ios::floatfield);		// set default behavior
				break;

			case 'c':		// output utf-8 character
				break;

			case 'b':		// binary format
				break;

			case '%':		// percentage
				// displays in fixed floating point format plus '%' percent sign
				did_change = true;
				ss.setf(std::ios::fixed, std::ios::floatfield);
				ss.setf(std::ios::dec, std::ios::basefield);
				break;

			default:
				throw std::invalid_argument("invalid format string");
		}

		fmt++;
		if (*fmt != '}') {
			throw std::invalid_argument("invalid format string");
		}
		fmt++;
		return fmt;
	}

	void reset(std::ostringstream& ss) {
		// reset this format_spec__ to default

		// and reset stringstream to default
		if (did_change) {
			ss.width(0);
			ss.precision(6);		// default precision is 6
			ss.fill(' ');
			ss.unsetf(std::ios::showbase|std::ios::basefield|std::ios::floatfield|std::ios::adjustfield|
				std::ios::showpoint|std::ios::showpos|std::ios::uppercase);
		}

		did_change = alternate = center = false;
		width = 0;
		fillchar = grouping = ' ';
		fmt_type = 's';
	}
};

void sformat_binary__(std::ostringstream& ss, uint64_t value, int bits, bool alternate, bool grouping) {
	if (alternate) {
		ss << "0b";
	}
	int g = 0;
	for (int i = bits - 1; i >= 0; i--) {
		ss << ((value & (1 << i)) ? '1' : '0');

		if (grouping && i) {
			// output grouping underscore
			g++;
			if (g >= 4) {
				g = 0;
				ss << '_';
			}
		}
	}
}

void sformat_utf8__(std::ostringstream& ss, uint32_t r) {
	// decode uint32_t to utf-8 (sub)string

	// last valid unicode point
	if (r > 0x10ffff) {
		throw std::invalid_argument("invalid unicode code point");
	}

	const int buflen = 8;
	char buf[buflen];
	int n = 0;

	char b;

	if (r < 0x80) {
		// ascii byte
		buf[n++] = r;
	} else if (r < 0x800) {
		// two bytes
		b = 0xc0 | (r >> 6);
		buf[n++] = b;
		b = 0x80 | (r & 0x3f);
		buf[n++] = b;
	} else if (r < 0x10000) {
		// three bytes
		b = 0xe0 | (r >> 12);
		buf[n++] = b;
		b = 0x80 | ((r >> 6) & 0x3f);
		buf[n++] = b;
		b = 0x80 | (r & 0x3f);
		buf[n++] = b;
	} else {
		// four bytes
		b = 0xf0 | (r >> 18);
		buf[n++] = b;
		b = 0x80 | ((r >> 12) & 0x3f);
		buf[n++] = b;
		b = 0x80 | ((r >> 6) & 0x3f);
		buf[n++] = b;
		b = 0x80 | (r & 0x3f);
		buf[n++] = b;
	}
	buf[n] = 0;
	ss << buf;
}

void sformat_center__(std::ostringstream& ss, const std::string& s, int width, char fill) {
	int len = s.length();
	if (width < len) {
		ss << s;
		return;
	}
	int diff = width - len;
	int left = diff / 2;
	int right = diff - left;
	ss << std::string(left, fill) << s << std::string(right, fill);
}

template <typename T>
void sformat_str__(std::ostringstream& ss, const format_spec__& spec, const T& value) {
	(void)spec;		// unused
	// default: use stream operator<<()
	// stream flags were already set up according to spec
	ss << value;
}

// format specific types
void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const bool& value) {
	(void)spec;		// unused
	ss << (value ? "true" : "false");
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const int8_t& value) {
	if (spec.fmt_type == 'b') {
		sformat_binary__(ss, (uint64_t)value, 8, spec.alternate, spec.grouping == '_');
	} else {
		ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const uint8_t& value) {
	if (spec.fmt_type == 'b') {
		sformat_binary__(ss, (uint64_t)value, 8, spec.alternate, spec.grouping == '_');
	} else {
		ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const int16_t& value) {
	if (spec.fmt_type == 'b') {
		sformat_binary__(ss, (uint64_t)value, 16, spec.alternate, spec.grouping == '_');
	} else {
		ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const uint16_t& value) {
	if (spec.fmt_type == 'b') {
		sformat_binary__(ss, (uint64_t)value, 16, spec.alternate, spec.grouping == '_');
	} else {
		ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const int32_t& value) {
	switch (spec.fmt_type) {
		case 'b':		// binary
			sformat_binary__(ss, (uint64_t)value, 32, spec.alternate, spec.grouping == '_');
			break;

		case 'c':		// utf-8
			sformat_utf8__(ss, (uint32_t)value);
			break;

		default:
			ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const uint32_t& value) {
	switch (spec.fmt_type) {
		case 'b':		// binary
			sformat_binary__(ss, (uint64_t)value, 32, spec.alternate, spec.grouping == '_');
			break;

		case 'c':		// utf-8
			sformat_utf8__(ss, (uint32_t)value);
			break;

		default:
			ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const int64_t& value) {
	if (spec.fmt_type == 'b') {
		sformat_binary__(ss, (uint64_t)value, 64, spec.alternate, spec.grouping == '_');
	} else {
		ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const uint64_t& value) {
	if (spec.fmt_type == 'b') {
		sformat_binary__(ss, value, 64, spec.alternate, spec.grouping == '_');
	} else {
		ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const float& value) {
	if (spec.fmt_type == '%') {
		ss << value * 100.0 << '%';
	} else {
		ss << value;
	}
}

void sformat_str__(std::ostringstream& ss, const format_spec__ spec, const double& value) {
	if (spec.fmt_type == '%') {
		ss << value * 100.0 << '%';
	} else {
		ss << value;
	}
}

inline void sformat(std::ostringstream& ss, const char *fmt) {
	// sformat() without further varargs arguments
	ss << fmt;
}

template <typename T, typename ...Args>
void sformat(std::ostringstream& ss, const char* fmt, const T& value, Args&& ...args) {
	if (fmt == nullptr) {
		return;
	}

	while (*fmt) {
		if (*fmt == '{') {
			if (fmt[1] == '{') {
				// literal curly brace
				ss << *fmt;
			} else {
				format_spec__ spec;

				fmt = spec.parse(fmt, ss);

				if (spec.center) {
					// for centering first put into a temp buffer
					spec.center = false;
					std::ostringstream tmp;
					sformat_str__(tmp, spec, value);
					sformat_center__(ss, tmp.str(), spec.width, spec.fillchar);
				} else {
					sformat_str__(ss, spec, value);
				}
				spec.reset(ss);

				if (*fmt) {
					// recurse to get the next argument from the Tpack
					sformat(ss, fmt, std::forward<Args>(args)...);
				}
				return;
			}
		} else {
			if (*fmt == '}') {
				if (fmt[1] == '}') {
					// literal curly brace
					fmt++;
				} else {
					throw std::invalid_argument("invalid format string");
				}
			}

			ss << *fmt;
		}
		fmt++;
	}
}

inline std::string format(const char* fmt) {
	return std::string(fmt);
}

template <typename ...Args>
std::string format(const char* fmt, Args&& ...args) {
	std::ostringstream ss;
	sformat(ss, fmt, std::forward<Args>(args)...);
	std::string s = ss.str();
	return std::string(s.c_str());
}

inline std::string format(const std::string& fmt) {
	return fmt;
}

template <typename ...Args>
std::string format(const std::string& fmt, Args&& ...args) {
	return format(fmt.c_str(), std::forward<Args>(args)...);
}

//}   // namespace

#endif  // FORMAT_H_WJ121

// EOB
