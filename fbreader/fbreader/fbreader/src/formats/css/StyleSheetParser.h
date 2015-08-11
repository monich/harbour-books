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

#ifndef __STYLESHEETPARSER_H__
#define __STYLESHEETPARSER_H__

#include "StyleSheetTable.h"

#include <stack>

class ZLInputStream;

class StyleSheetParser {

protected:
	StyleSheetParser();

public:
	virtual ~StyleSheetParser();
	void reset();
	void parse(ZLInputStream &stream);
	void parse(const char *text, int len, bool final = false);

protected:
	virtual void storeData(const std::string &selector, const StyleSheetTable::AttributeMap &map);

private:
	enum ReadState {
		COMMENT,
		SELECTOR,
		ATTRIBUTE_NAME,
		ATTRIBUTE_VALUE,
		ATTRIBUTE_VALUE_SPACE,
		ATTRIBUTE_VALUE_COMMA,
		ATTRIBUTE_IGNORE,
		STRING_LITERAL_SINGLE = '\'',
		STRING_LITERAL_DOUBLE = '"',
		SKIP_BLOCK_CURLY = '}',
		SKIP_BLOCK_SQUARE = ']'
	};

	void reset(ReadState state);
	void processChar1(char c);
	void processChar2(char c);
	void processChar3(char c);
	void processChar4(char c);
	void finishRule();
	void finishAttribute();
	void finishAttributeValue();

private:
	std::string myWord;
	std::string myAttributeName;
	std::stack<ReadState> myStateStack;
	std::vector<std::string> mySelectors;
	StyleSheetTable::AttributeMap myMap;
	char myBuffer1;
	char myBuffer2;
	char myBuffer3;

friend class StyleSheetSingleStyleParser;
};

class StyleSheetTableParser : public StyleSheetParser {

public:
	StyleSheetTableParser(StyleSheetTable &table);

private:
	void storeData(const std::string &selector, const StyleSheetTable::AttributeMap &map);

private:
	StyleSheetTable &myTable;
};

class StyleSheetSingleStyleParser : public StyleSheetParser {

public:
	StyleSheetTable::Style parseString(const char *text);
};

inline void StyleSheetParser::reset() { reset(SELECTOR); }

#endif /* __STYLESHEETPARSER_H__ */
