/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
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
		myReadState = ATTRIBUTE_NAME;
		parse(text, strlen(text), true);
		StyleSheetTable::updateStyle(style, myMap);
		reset();
	}
	return style;
}

StyleSheetParser::StyleSheetParser() : myReadState(TAG_NAME), myInsideComment(false), myAtBlockDepth(0) {
}

StyleSheetParser::~StyleSheetParser() {
}

void StyleSheetParser::reset() {
	myWord.erase();
	myAttributeName.erase();
	myReadState = TAG_NAME;
	myInsideComment = false;
	myAtBlockDepth = 0;
	mySelectors.clear();
	myMap.clear();
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
	for (const char *ptr = start; ptr != end; ++ptr) {
		if ((myReadState != TAG_NAME && isspace(*ptr)) ||
		    (myReadState == TAG_NAME && *ptr == ',')) {
			if (start != ptr) {
				myWord.append(start, ptr - start);
			}
			processWord(myWord);
			myWord.erase();
			start = ptr + 1;
		} else if (isControlSymbol(*ptr)) {
			if (start != ptr) {
				myWord.append(start, ptr - start);
			}
			processWord(myWord);
			myWord.erase();
			if (!myInsideComment) {
				processControl(*ptr);
			}
			start = ptr + 1;
		}
	}
	if (start < end) {
		myWord.append(start, end - start);
		if (final) {
			processWord(myWord);
			myWord.erase();
		}
	}
}

bool StyleSheetParser::isControlSymbol(const char symbol) {
	switch (symbol) {
		case '{':
		case '}':
		case ';':
		case ':':
			return true;
		default:
			return false;
	}
}

void StyleSheetParser::storeData(const std::string&, const StyleSheetTable::AttributeMap&) {
}

void StyleSheetParser::processControl(const char control) {
	switch (control) {
		case '{':
			switch (myReadState) {
				case AT_RULE:
					myReadState = AT_BLOCK;
					myAtBlockDepth = 1;
					break;
				case AT_BLOCK:
					myAtBlockDepth++;
					break;
				case TAG_NAME:
					myReadState = ATTRIBUTE_NAME;
					myMap.clear();
					break;
				default:
					myReadState = BROKEN;
					break;
			}
			break;
		case '}':
			switch (myReadState) {
				case AT_BLOCK:
					if (--myAtBlockDepth > 0) {
						return;
					}
					break;
				case AT_RULE:
				case BROKEN:
					break;
				default:
					for (unsigned int i=0; i<mySelectors.size(); i++) {
						storeData(mySelectors[i], myMap);
					}
					break;
			}
			myReadState = TAG_NAME;
			mySelectors.clear();
			myMap.clear();
			break;
		case ';':
			switch (myReadState) {
				case AT_RULE:
					myReadState = TAG_NAME;
					mySelectors.clear();
					myMap.clear();
					break;
				case AT_BLOCK:
					break;
				case ATTRIBUTE_VALUE:
				case ATTRIBUTE_NAME:
					myReadState = ATTRIBUTE_NAME;
					break;
				default:
					myReadState = BROKEN;
					break;
			}
			break;
		case ':':
			switch (myReadState) {
				case AT_BLOCK:
					break;
				case ATTRIBUTE_NAME:
					myReadState = ATTRIBUTE_VALUE;
					break;
				default:
					myReadState = BROKEN;
					break;
			}
			break;
	}
}

void StyleSheetParser::processWord(std::string &word) {
	while (!word.empty()) {
		int index = word.find(myInsideComment ? "*/" : "/*");
		if (!myInsideComment) {
			if (index == -1) {
				processWordWithoutComments(word);
			} else if (index > 0) {
				processWordWithoutComments(word.substr(0, index));
			}
		}
		if (index == -1) {
			break;
		}
		myInsideComment = !myInsideComment;
		word.erase(0, index + 2);
	}
}
	
void StyleSheetParser::processWordWithoutComments(std::string word) {
	switch (myReadState) {
		case AT_RULE:
		case AT_BLOCK:
			break;
		case TAG_NAME:
			ZLStringUtil::stripWhiteSpaces(word);
			if (!word.empty()) {
				if (word[0] == '@') {
					myReadState = AT_RULE;

				} else {
					mySelectors.push_back(word);
				}
			}
			break;
		case ATTRIBUTE_NAME:
			myAttributeName = word;
			myMap[myAttributeName].clear();
			break;
		case ATTRIBUTE_VALUE:
			myMap[myAttributeName].push_back(word);
			break;
		case BROKEN:
			break;
	}
}
