/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
 * Copyright (C) 2015 Slava Monich <slava.monich@jolla.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef __ZLSTRINGUTIL_H__
#define __ZLSTRINGUTIL_H__

#include <vector>
#include <string>

class ZLStringUtil {

private:
	ZLStringUtil();

public:
	static bool stringStartsWith(const std::string &str, const std::string &start);
	static bool stringEndsWith(const std::string &str, const std::string &end);
	static bool startsWith(const std::string &str, char c);
	static bool endsWith(const std::string &str, char c);
	static bool caseInsensitiveEqual(const std::string &str1, const std::string &str2);
	static bool caseInsensitiveSort(const std::string &str1, const std::string &str2);
	static void appendNumber(std::string &str, unsigned int n);
	static void append(std::string &str, const std::vector<std::string> &buffer);
	static bool stripWhiteSpaces(std::string &str);
	static std::vector<std::string> splitString(const char *str, const char* delim);
	static std::vector<std::string> splitString(const std::string &str, const char* delim);
	static void replaceAll(std::string &str, const std::string &find, const std::string &replaceWith);

	static std::string printf(const std::string &format, const std::string &arg0);

	static std::string doubleToString(double value);
	static double stringToDouble(const std::string &value, double defaultValue);
	static bool stringToLong(const std::string &str, long &result);
	static bool stringToLong(const char *str, long &result);
	static int fromHex(char hex);
};

inline std::vector<std::string> ZLStringUtil::splitString(const std::string &str, const char* delim) {
	return ZLStringUtil::splitString(str.c_str(), delim);
}
inline bool ZLStringUtil::startsWith(const std::string &str, char c) {
    return !str.empty() && str[0] == c;
}
inline bool ZLStringUtil::endsWith(const std::string &str, char c) {
	return !str.empty() && str[str.length()-1] == c;
}
inline bool ZLStringUtil::stringToLong(const std::string &str, long &result) {
	return stringToLong(str.c_str(), result);
}

#endif /* __ZLSTRINGUTIL_H__ */
