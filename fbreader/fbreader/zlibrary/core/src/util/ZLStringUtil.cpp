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

#include <cctype>
#include <cstdio>
#include <cstdlib>

#include <locale.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "ZLStringUtil.h"

bool ZLStringUtil::stringEndsWith(const std::string &str, const std::string &end) {
	return
		(end.length() <= str.length()) &&
#if __GNUC__ == 2
		(str.compare(end, str.length() - end.length(), end.length()) == 0);
#else
		(str.compare(str.length() - end.length(), end.length(), end) == 0);
#endif
}

bool ZLStringUtil::stringStartsWith(const std::string &str, const std::string &start) {
	return
		(start.length() <= str.length()) &&
#if __GNUC__ == 2
		(str.compare(start, 0, start.length()) == 0);
#else
		(str.compare(0, start.length(), start) == 0);
#endif
}

bool ZLStringUtil::caseInsensitiveEqual(const std::string &str1, const std::string &str2) {
	return !strcasecmp(str1.c_str(), str2.c_str());
}

bool ZLStringUtil::caseInsensitiveSort(const std::string &str1, const std::string &str2) {
	return strcasecmp(str1.c_str(), str2.c_str()) < 0;
}

void ZLStringUtil::appendNumber(std::string &str, unsigned int n) {
	int len;
	if (n > 0) {
		len = 0;
		for (unsigned int copy = n; copy > 0; copy /= 10) {
			len++;
		}
	} else {
		len = 1;
	}
	
	str.append(len, '\0');
	char *ptr = (char*)str.data() + str.length() - 1;
	for (int i = 0; i < len; ++i) {
		*ptr-- = '0' + n % 10;
		n /= 10;
	}
}

void ZLStringUtil::append(std::string &str, const std::vector<std::string> &text) {
	size_t len = str.length();
	for (std::vector<std::string>::const_iterator it = text.begin(); it != text.end(); ++it) {
		len += it->length();
	}
	str.reserve(len);
	for (std::vector<std::string>::const_iterator it = text.begin(); it != text.end(); ++it) {
		str += *it;
	}
}

// Returns true if there's anything left
bool ZLStringUtil::stripWhiteSpaces(std::string &str) {
	const size_t old_length = str.length();
	if (old_length > 0) {
		size_t end = old_length;
		while ((end > 0) && isspace((unsigned char)str[end - 1])) {
			end--;
		}
		if (end < old_length) {
			str.erase(end, old_length - end);
		}

		size_t start = 0;
		while ((start < end) && isspace((unsigned char)str[start])) {
			start++;
		}
		if (start > 0) {
			str.erase(0, start);
		}
		return !str.empty();
	} else {
		return false;
	}
}

std::vector<std::string> ZLStringUtil::splitString(const char *str, const char* delim) {
	std::vector<std::string> tokens;
	if (str != 0) {
		char *buf = strdup(str);
		char *saveptr;
		char *token = strtok_r(buf, delim, &saveptr);
		while (token) {
			tokens.push_back(std::string(token));
			token = strtok_r(NULL, delim, &saveptr);
		}
		free(buf);
	}
	return tokens;
}

void ZLStringUtil::replaceAll(std::string &str, const std::string &find, const std::string &replaceWith) {
	size_t pos = 0;
	while ((pos == str.find(find, pos)) != std::string::npos) {
		str.replace(pos, find.length(), replaceWith);
		pos += replaceWith.length();
	}
}

std::string ZLStringUtil::printf(const std::string &format, const std::string &arg0) {
	int index = format.find("%s");
	if (index == -1) {
		return format;
	}
	return format.substr(0, index) + arg0 + format.substr(index + 2);
}

std::string ZLStringUtil::doubleToString(double value) {
	char buf[100];
	setlocale(LC_NUMERIC, "C");
	sprintf(buf, "%f", value);
	return buf;
}

double ZLStringUtil::stringToDouble(const std::string &value, double defaultValue) {
	if (!value.empty()) {
		setlocale(LC_NUMERIC, "C");
		return atof(value.c_str());
	} else {
		return defaultValue;
	}
}

bool ZLStringUtil::stringToLong(const char *s, long &result) {
	while (*s && isspace(*s)) s++;
	char* endptr = NULL;
	long number = strtol(s, &endptr, 10);
	if (endptr && endptr != s) {
		if ((number != LONG_MAX && number != LONG_MIN) || (errno != ERANGE)) {
			while (*endptr && isspace(*endptr)) endptr++;
			if (!*endptr) {
				result = number;
				return true;
			}
		}
	}
	return false;
}

int ZLStringUtil::fromHex(char hex) {
	return (hex >= '0' && hex <= '9') ? (hex - '0') :
	       (hex >= 'a' && hex <= 'z') ? (hex - 'a' + 10) :
	       (hex >= 'A' && hex <= 'Z') ? (hex - 'A' + 10) : -1;
}
