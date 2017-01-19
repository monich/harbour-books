/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
 * Copyright (C) 2015-2017 Slava Monich <slava.monich@jolla.com>
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
#include <cstring>

#include <ZLStringUtil.h>
#include <ZLInputStream.h>

#include "StyleSheetParser.h"

StyleSheetTableParser::StyleSheetTableParser(StyleSheetTable &table) : myTable(table) {
}

void StyleSheetTableParser::storeData(const std::string &selector, const StyleSheetTable::AttributeMap &map) {
	myTable.addMap(ZLStringUtil::splitString(selector, " \t+"), map);
}

StyleSheetTable::Style StyleSheetSingleStyleParser::parseString(const char *text) {
	StyleSheetTable::Style style;
	if (text) {
		reset(ATTRIBUTE_NAME);
		parse(text, strlen(text), true);
		if (!myStateStack.empty()) {
			switch (myStateStack.top()) {
			case ATTRIBUTE_VALUE:
			case ATTRIBUTE_VALUE_SPACE:
			case ATTRIBUTE_VALUE_COMMA:
				finishAttributeValue();
				break;
			default:
				break;
			}
		}
		StyleSheetTable::updateStyle(style, myMap);
	}
	return style;
}

StyleSheetParser::StyleSheetParser() :
	myBuffer1(0), myBuffer2(0), myBuffer3(0) {
	myStateStack.push(SELECTOR);
}

StyleSheetParser::~StyleSheetParser() {
}

void StyleSheetParser::reset(ReadState state) {
	myWord.resize(0);
	myAttributeName.resize(0);
	myStateStack = std::stack<ReadState>();
	myStateStack.push(state);
	mySelectors.resize(0);
	myMap.clear();
	myBuffer1 = myBuffer2 = myBuffer3 = 0;
}

void StyleSheetParser::parse(ZLInputStream &stream) {
	if (stream.open()) {
		char *buffer = new char[1024];
		while (true) {
			int len = stream.read(buffer, 1024);
			if (len == 0) {
				break;
			}
			parse(buffer, len);
		}
		delete[] buffer;
		stream.close();
	}
}

void StyleSheetParser::parse(const char *text, int len, bool final) {
	const char *start = text;
	const char *end = text + len;
	for (const char *ptr = start; ptr != end && !myStateStack.empty(); ++ptr) {
		processChar1(*ptr);
	}
}

void StyleSheetParser::storeData(const std::string&, const StyleSheetTable::AttributeMap&) {
}

/* Converts \r\n into \n */
void StyleSheetParser::processChar1(char c) {
	if (myBuffer1 == '\r') {
		myBuffer1 = 0;
		if (c == '\n') {
			processChar2('\n');
		} else {
			processChar2('\r');
		}
	} else if (c == '\r') {
		myBuffer1 = '\r';
	} else {
		processChar2(c);
	}
}

/* Glues continued lines together */
void StyleSheetParser::processChar2(char c) {
	if (myBuffer2 == '\\') {
		myBuffer2 = 0;
		if (c != '\n') {
			processChar3('\\');
			processChar3(c);
		}
	} else if (c == '\\') {
		myBuffer2 = '\\';
	} else {
		processChar3(c);
	}
}

/* Handles comments */
void StyleSheetParser::processChar3(char c) {
	switch (myStateStack.top()) {
	case COMMENT:
		if (myBuffer3 == '*' && c == '/') {
			myBuffer3 = 0;
			myStateStack.pop();
		} else {
			myBuffer3 = (c == '*') ? '*' : 0;
		}
		break;
	case STRING_LITERAL_SINGLE:
	case STRING_LITERAL_DOUBLE:
		processChar4(c);
		break;
	default:
		if (myBuffer3 == '/' && c == '*') {
			myStateStack.push(COMMENT);
		} else {
			if (myBuffer3) processChar4(myBuffer3);
			if (c == '/') {
				myBuffer3 = c;
			} else {
				myBuffer3 = 0;
				processChar4(c);
			}
		}
		break;
	}
}

/* Handles clean input */
void StyleSheetParser::processChar4(char c) {
	switch (myStateStack.top()) {
	case SELECTOR:
		switch (c) {
		case ',':
			if (ZLStringUtil::stripWhiteSpaces(myWord)) {
				mySelectors.push_back(myWord);
				myWord.resize(0);
			}
			break;
		case '{':
			if (ZLStringUtil::stripWhiteSpaces(myWord)) mySelectors.push_back(myWord);
			if (!mySelectors.empty()) {
				if (mySelectors[0][0] == '@') {
					// Ignore AT-rules
					mySelectors.resize(0);
					myStateStack.push(SKIP_BLOCK_CURLY);
				} else {
					myMap.clear();
					myStateStack.push(ATTRIBUTE_NAME);
				}
			} else {
				myStateStack.push(SKIP_BLOCK_CURLY);
			}
			myWord.resize(0);
			break;
		default:
			if (!isspace(c) || !myWord.empty()) {
				myWord.append(1, c);
			}
			break;
		}
		break;

	case ATTRIBUTE_NAME:
		switch (c) {
		case ':':
			if (ZLStringUtil::stripWhiteSpaces(myWord)) {
				myAttributeName = myWord;
				myMap[myAttributeName].resize(0);
				myWord.resize(0);
				static const std::string FONT_FAMILY("font-family");
				static const std::string COLOR("color");
				if (myAttributeName == COLOR) {
					myStateStack.top() = ATTRIBUTE_VALUE;
				} else if (myAttributeName == FONT_FAMILY) {
					myStateStack.top() = ATTRIBUTE_VALUE_COMMA;
				} else {
					myStateStack.top() = ATTRIBUTE_VALUE_SPACE;
				}
			} else {
				finishAttribute();
				myStateStack.top() = ATTRIBUTE_IGNORE;
			}
			break;
		case '\n':
		case ';':
			finishAttribute();
			break;
		case '{':
			myStateStack.push(SKIP_BLOCK_CURLY);
			break;
		case '}':
			finishRule();
			myStateStack.pop();
			break;
		default:
			if (!isspace(c) || !myWord.empty()) {
				myWord.append(1, c);
			}
			break;
		}
		break;

	case ATTRIBUTE_VALUE:
	case ATTRIBUTE_VALUE_SPACE:
	case ATTRIBUTE_VALUE_COMMA:
		switch (c) {
		case '\'':
			myStateStack.push(STRING_LITERAL_SINGLE);
			break;
		case '"':
			myStateStack.push(STRING_LITERAL_DOUBLE);
			break;
		case '}':
			finishAttributeValue();
			finishRule();
			myStateStack.pop();
			break;
		case ';':
		case '\n':
			finishAttributeValue();
			finishAttribute();
			myStateStack.top() = ATTRIBUTE_NAME;
			break;
		case ',':
			if (myStateStack.top() == ATTRIBUTE_VALUE_COMMA) {
				finishAttributeValue();
			} else {
				myWord.append(1, c);
			}
			break;
		default:
			if (isspace(c)) {
				if (myStateStack.top() == ATTRIBUTE_VALUE_SPACE) {
					finishAttributeValue();
				} else if (!myWord.empty()) {
					myWord.append(1, c);
				}
			} else {
				myWord.append(1, c);
			}
			break;
		}
		break;

	case ATTRIBUTE_IGNORE:
		switch (c) {
		case '\n':
		case ';':
			finishAttribute();
			myStateStack.top() = ATTRIBUTE_NAME;
			break;
		case '}':
			finishRule();
			myStateStack.pop();
			break;
		default:
			break;
		}
		break;

	case STRING_LITERAL_SINGLE:
	case STRING_LITERAL_DOUBLE:
		if (c == myStateStack.top()) {
			myStateStack.pop();
		} else if (c == '\n') {
			// User agents must close strings upon reaching
			// the end of a line (i.e., before an unescaped
			// line feed, carriage return or form feed character),
			// but then drop the construct (declaration or rule)
			// in which the string was found.
			myStateStack.pop();
			switch (myStateStack.top()) {
			case ATTRIBUTE_VALUE:
			case ATTRIBUTE_VALUE_SPACE:
			case ATTRIBUTE_VALUE_COMMA:
				myStateStack.top() = ATTRIBUTE_IGNORE;
				myMap[myAttributeName].resize(0);
				myAttributeName.resize(0);
				myWord.resize(0);
				break;
			default:
				break;
			}
		} else {
			myWord.append(1, c);
		}
		break;

	case SKIP_BLOCK_CURLY:
	case SKIP_BLOCK_SQUARE:
		switch (c) {
		case '{':
			myStateStack.push(SKIP_BLOCK_CURLY);
			break;
		case '[':
			myStateStack.push(SKIP_BLOCK_SQUARE);
			break;
		case '\'':
			myStateStack.push(STRING_LITERAL_SINGLE);
			break;
		case '"':
			myStateStack.push(STRING_LITERAL_DOUBLE);
			break;
		default:
			if (c == myStateStack.top()) {
				myStateStack.pop();
				if (myStateStack.top() == SELECTOR) {
					myWord.resize(0);
					mySelectors.resize(0);
					myMap.clear();
				}
			}
			break;
		}
		break;

	case COMMENT: // Comments are handled elsewhere
		break;
	}
}

void StyleSheetParser::finishAttributeValue() {
	if (ZLStringUtil::stripWhiteSpaces(myWord)) {
		myMap[myAttributeName].push_back(myWord);
		myWord.resize(0);
	}
}

void StyleSheetParser::finishAttribute() {
	myAttributeName.resize(0);
	myWord.resize(0);
}

void StyleSheetParser::finishRule() {
	for (unsigned int i=0; i<mySelectors.size(); i++) {
		storeData(mySelectors[i], myMap);
	}
	myAttributeName.resize(0);
	myWord.resize(0);
	mySelectors.resize(0);
	myMap.clear();
}
