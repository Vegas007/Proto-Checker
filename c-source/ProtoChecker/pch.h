#ifndef PCH_H
#define PCH_H

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <set>
#include <algorithm>
#include <regex>
#include <cstdint>

/**
 * \brief
 * A system function in windows to change the colour of text and the console background.
 */
static const int32_t console_color = system("color 00");

/**
 * \brief
 * Parses the C-string str, interpreting its content as an integral number of the specified base,
 * which is returned as an value of type unsigned long int.
 * \param format: string
 * \return: uint32_t
 */
inline auto strtoul(const std::string& format) -> uint32_t
{
	return strtoul(format.c_str(), nullptr, 10);
}

/**
 * \brief
 * Write formatted output using a pointer to a list of arguments.
 * \param format: string
 * \param ...: any
 * \return: char *
 */
inline const char * format(const std::string format, ...)
{
	static char cFormatLine[512];
	va_list args;
	va_start(args, format);
	_vsnprintf_s(cFormatLine, sizeof(cFormatLine), format.c_str(), args);
	va_end(args);
	return cFormatLine;
}

#endif //PCH_H
