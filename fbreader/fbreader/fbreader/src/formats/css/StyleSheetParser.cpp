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

void StyleSheetTableParser::storeData(const std::string &tagName, const std::string &className, const StyleSheetTable::AttributeMap &map) {
	myTable.addMap(tagName, className, map);
}

shared_ptr<ZLTextStyleEntry> StyleSheetSingleStyleParser::parseString(const char *text, ZLBoolean3 *pageBreakBefore, ZLBoolean3 *pageBreakAfter) {
	myReadState = ATTRIBUTE_NAME;
	parse(text, strlen(text), true);
	shared_ptr<ZLTextStyleEntry> control = StyleSheetTable::createControl(NULL, myMap);
	bool value;
	if (pageBreakBefore && StyleSheetTable::getPageBreakBefore(myMap, value)) {
		*pageBreakBefore = value ? B3_TRUE : B3_FALSE;
	}
	if (pageBreakAfter && StyleSheetTable::getPageBreakAfter(myMap, value)) {
		*pageBreakAfter = value ? B3_TRUE : B3_FALSE;
	}
	reset();
	return control;
}

StyleSheetParser::StyleSheetParser() : myReadState(TAG_NAME), myInsideComment(false) {
}

StyleSheetParser::~StyleSheetParser() {
}

void StyleSheetParser::reset() {
	myWord.erase();
	myAttributeName.erase();
	myReadState = TAG_NAME;
	myInsideComment = false;
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
		if (isspace(*ptr)) {
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

void StyleSheetParser::storeData(const std::string&, const std::string&, const StyleSheetTable::AttributeMap&) {
}

void StyleSheetParser::processControl(const char control) {
	switch (control) {
		case '{':
			myReadState = (myReadState == TAG_NAME) ? ATTRIBUTE_NAME : BROKEN;
			break;
		case '}':
			if (myReadState != BROKEN) {
				for (unsigned int i=0; i<mySelectors.size(); i++) {
					std::string selector(mySelectors[i]);
					std::string tag, klass;
					const int index = selector.find('.');
					if (index == -1) {
						tag = selector;
					} else {
						tag = selector.substr(0, index);
						klass = selector.substr(index + 1);
					}
					storeData(tag, klass, myMap);
				}
			}
			myReadState = TAG_NAME;
			mySelectors.clear();
			myMap.clear();
			break;
		case ';':
			myReadState =
				((myReadState == ATTRIBUTE_VALUE) ||
				 (myReadState == ATTRIBUTE_NAME)) ? ATTRIBUTE_NAME : BROKEN;
			break;
		case ':':
			myReadState = (myReadState == ATTRIBUTE_NAME) ? ATTRIBUTE_VALUE : BROKEN;
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
	
void StyleSheetParser::processWordWithoutComments(const std::string &word) {	
	switch (myReadState) {
		case TAG_NAME:
			if (!word.empty()) {
				const unsigned int len = word.length();
				if (word.at(len-1) == ',') {
					mySelectors.push_back(word.substr(0, len-1));
				} else {
					mySelectors.push_back(word);
				}
			}
			myMap.clear();
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
