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

#include <cstdlib>

#include <ZLStringUtil.h>

#include "StyleSheetTable.h"

bool StyleSheetTable::isEmpty() const {
	return myControlMap.empty() && myPageBreakBeforeMap.empty() && myPageBreakAfterMap.empty();
}

void StyleSheetTable::addMap(const std::string &tag, const std::string &aClass, const AttributeMap &map) {
	if ((!tag.empty() || !aClass.empty()) && !map.empty()) {
		Key key(tag, aClass);
		// This will update the existing element if it already exists
		// or create a new one if it wasn't there yet
		myControlMap[key] = createControl(myControlMap[key], map);
		bool value;
		if (getPageBreakBefore(map, value)) {
			myPageBreakBeforeMap[key] = value;
		}
		if (getPageBreakAfter(map, value)) {
			myPageBreakAfterMap[key] = value;
		}
	}
}

static bool parseLength(const std::string &toParse, short &size, ZLTextStyleEntry::SizeUnit &unit) {
	if (!toParse.empty()) {
		if (ZLStringUtil::stringEndsWith(toParse, "%")) {
			unit = ZLTextStyleEntry::SIZE_UNIT_PERCENT;
			size = atoi(toParse.c_str());
			return true;
		} else if (ZLStringUtil::stringEndsWith(toParse, "em")) {
			unit = ZLTextStyleEntry::SIZE_UNIT_EM_100;
			size = (short)(100 * ZLStringUtil::stringToDouble(toParse, 0));
			return true;
		} else if (ZLStringUtil::stringEndsWith(toParse, "ex")) {
			unit = ZLTextStyleEntry::SIZE_UNIT_EX_100;
			size = (short)(100 * ZLStringUtil::stringToDouble(toParse, 0));
			return true;
		} else if (ZLStringUtil::stringEndsWith(toParse, "px") ||
			   ZLStringUtil::stringEndsWith(toParse, "pt") ||
			   ZLStringUtil::stringEndsWith(toParse, "pc")) {
			unit = ZLTextStyleEntry::SIZE_UNIT_PIXEL;
			size = atoi(toParse.c_str());
			return true;
		} else if (toParse == "0") {
			unit = ZLTextStyleEntry::SIZE_UNIT_PIXEL;
			size = 0;
			return true;
		}
	}
	return false;
}

void StyleSheetTable::setLength(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const AttributeMap &map, const std::string &attributeName) {
	StyleSheetTable::AttributeMap::const_iterator it = map.find(attributeName);
	if (it == map.end()) {
		return;
	}
	const std::vector<std::string> &values = it->second;
	if (!values.empty())
		setLength(entry, name, values[0]);
}

void StyleSheetTable::setLength(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const std::string &value) {
	short size;
	ZLTextStyleEntry::SizeUnit unit;
	if (parseLength(value, size, unit)) {
		entry.setLength(name, size, unit);
	}
}

bool StyleSheetTable::doBreakBefore(const std::string &tag, const std::string &aClass) const {
	std::map<Key,bool>::const_iterator it = myPageBreakBeforeMap.find(Key(tag, aClass));
	if (it != myPageBreakBeforeMap.end()) {
		return it->second;
	}

	it = myPageBreakBeforeMap.find(Key("", aClass));
	if (it != myPageBreakBeforeMap.end()) {
		return it->second;
	}

	it = myPageBreakBeforeMap.find(Key(tag, ""));
	if (it != myPageBreakBeforeMap.end()) {
		return it->second;
	}

	return false;
}

bool StyleSheetTable::doBreakAfter(const std::string &tag, const std::string &aClass) const {
	std::map<Key,bool>::const_iterator it = myPageBreakAfterMap.find(Key(tag, aClass));
	if (it != myPageBreakAfterMap.end()) {
		return it->second;
	}

	it = myPageBreakAfterMap.find(Key("", aClass));
	if (it != myPageBreakAfterMap.end()) {
		return it->second;
	}

	it = myPageBreakAfterMap.find(Key(tag, ""));
	if (it != myPageBreakAfterMap.end()) {
		return it->second;
	}

	return false;
}

shared_ptr<ZLTextStyleEntry> StyleSheetTable::control(const std::string &tag, const std::string &aClass) const {
	std::map<Key,shared_ptr<ZLTextStyleEntry> >::const_iterator it =
		myControlMap.find(Key(tag, aClass));
	return (it != myControlMap.end()) ? it->second : 0;
}

const std::vector<std::string> &StyleSheetTable::values(const AttributeMap &map, const std::string &name) {
	const AttributeMap::const_iterator it = map.find(name);
	if (it != map.end()) {
		return it->second;
	}
	static const std::vector<std::string> emptyVector;
	return emptyVector;
}

shared_ptr<ZLTextStyleEntry> StyleSheetTable::createControl(shared_ptr<ZLTextStyleEntry> entry, const AttributeMap &styles) {
	if (entry.isNull()) {
		entry = new ZLTextStyleEntry();
	}

	const std::vector<std::string> &alignment = values(styles, "text-align");
	if (!alignment.empty()) {
		if (alignment[0] == "justify") {
			entry->setAlignmentType(ALIGN_JUSTIFY);
		} else if (alignment[0] == "left") {
			entry->setAlignmentType(ALIGN_LEFT);
		} else if (alignment[0] == "right") {
			entry->setAlignmentType(ALIGN_RIGHT);
		} else if (alignment[0] == "center") {
			entry->setAlignmentType(ALIGN_CENTER);
		}
	}

	const std::vector<std::string> &bold = values(styles, "font-weight");
	if (!bold.empty()) {
		int num = -1;
		if (bold[0] == "bold") {
			num = 700;
		} else if (bold[0] == "normal") {
			num = 400;
		} else if ((bold[0].length() == 3) &&
							 (bold[0][1] == '0') &&
							 (bold[0][2] == '0') &&
							 (bold[0][0] >= '1') &&
							 (bold[0][0] <= '9')) {
			num = 100 * (bold[0][0] - '0');
		} else if (bold[0] == "bolder") {
		} else if (bold[0] == "lighter") {
		}
		if (num != -1) {
			entry->setFontModifier(FONT_MODIFIER_BOLD, num >= 600);
		}
	}

	const std::vector<std::string> &italic = values(styles, "font-style");
	if (!italic.empty()) {
		entry->setFontModifier(FONT_MODIFIER_ITALIC, italic[0] == "italic");
	}

	const std::vector<std::string> &variant = values(styles, "font-variant");
	if (!variant.empty()) {
		entry->setFontModifier(FONT_MODIFIER_SMALLCAPS, variant[0] == "small-caps");
	}

	const std::vector<std::string> &fontFamily = values(styles, "font-family");
	if (!fontFamily.empty() && !fontFamily[0].empty()) {
		entry->setFontFamily(fontFamily[0]);
	}

	short size;
	ZLTextStyleEntry::SizeUnit unit;
	const std::vector<std::string> &fontSize = values(styles, "font-size");
	if (!fontSize.empty()) {
		std::string value = fontSize[0];
		if (value == "xx-small") {
			entry->setFontSizeMag(-3);
		} else if (value == "x-small") {
			entry->setFontSizeMag(-2);
		} else if (value == "small" || value == "smaller") {
			entry->setFontSizeMag(-1);
		} else if (value == "medium") {
			entry->setFontSizeMag(0);
		} else if (value == "large" || value == "larger") {
			entry->setFontSizeMag(1);
		} else if (value == "x-large") {
			entry->setFontSizeMag(2);
		} else if (value == "xx-large") {
			entry->setFontSizeMag(3);
		} else {
			if (parseLength(value, size, unit)) {
				switch (unit) {
				case ZLTextStyleEntry::SIZE_UNIT_PIXEL:
					// What to do with pixels?
					break;
				case ZLTextStyleEntry::SIZE_UNIT_EM_100:
				case ZLTextStyleEntry::SIZE_UNIT_EX_100:
				case ZLTextStyleEntry::SIZE_UNIT_PERCENT:
					entry->setFontSizeMag(
							      (size < 100 && size > 80) ? -1 :
							      (size > 100 && size < 120) ? 1 :
							      (size - 100)/20);
					break;
				}
			}
		}
	}

	std::vector<std::string> margins = values(styles, "margin");
	if (!margins.empty() && margins.back() == "!important") {
		// Ignore the "!important" modifier for now
		margins.pop_back();
	}
	switch (margins.size()) {
	case 1:
		if (parseLength(margins[0], size, unit)) {
			entry->setLength(ZLTextStyleEntry::LENGTH_SPACE_BEFORE, size, unit);
			entry->setLength(ZLTextStyleEntry::LENGTH_RIGHT_INDENT, size, unit);
			entry->setLength(ZLTextStyleEntry::LENGTH_SPACE_AFTER, size, unit);
			entry->setLength(ZLTextStyleEntry::LENGTH_LEFT_INDENT, size, unit);
		}
		break;
	case 2:
		if (parseLength(margins[0], size, unit)) {
			entry->setLength(ZLTextStyleEntry::LENGTH_SPACE_BEFORE, size, unit);
			entry->setLength(ZLTextStyleEntry::LENGTH_SPACE_AFTER, size, unit);
		}
		if (parseLength(margins[1], size, unit)) {
			entry->setLength(ZLTextStyleEntry::LENGTH_RIGHT_INDENT, size, unit);
			entry->setLength(ZLTextStyleEntry::LENGTH_LEFT_INDENT, size, unit);
		}
		break;
	case 3:
		setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, margins[0]);
		if (parseLength(margins[1], size, unit)) {
			entry->setLength(ZLTextStyleEntry::LENGTH_RIGHT_INDENT, size, unit);
			entry->setLength(ZLTextStyleEntry::LENGTH_LEFT_INDENT,  size, unit);
		}
		setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER,  margins[2]);
		break;
	case 4:
		setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, margins[0]);
		setLength(*entry, ZLTextStyleEntry::LENGTH_RIGHT_INDENT, margins[1]);
		setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER,  margins[2]);
		setLength(*entry, ZLTextStyleEntry::LENGTH_LEFT_INDENT,  margins[3]);
		break;
	}

	setLength(*entry, ZLTextStyleEntry::LENGTH_LEFT_INDENT, styles, "margin-left");
	setLength(*entry, ZLTextStyleEntry::LENGTH_RIGHT_INDENT, styles, "margin-right");
	setLength(*entry, ZLTextStyleEntry::LENGTH_FIRST_LINE_INDENT_DELTA, styles, "text-indent");
	setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, styles, "margin-top");
	setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, styles, "padding-top");
	setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER, styles, "margin-bottom");
	setLength(*entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER, styles, "padding-bottom");

	return entry;
}

bool StyleSheetTable::getPageBreakBefore(const AttributeMap &map, bool &value) {
	const std::vector<std::string> &pbb = values(map, "page-break-before");
	if (!pbb.empty()) {
		if ((pbb[0] == "always") ||
		    (pbb[0] == "left") ||
		    (pbb[0] == "right")) {
			value = true;
			return true;
		} else if (pbb[0] == "avoid") {
			value = false;
			return true;
		}
	}
	return false;
}

bool StyleSheetTable::getPageBreakAfter(const AttributeMap &map, bool &value) {
	const std::vector<std::string> &pba = values(map, "page-break-after");
	if (!pba.empty()) {
		if ((pba[0] == "always") ||
		    (pba[0] == "left") ||
		    (pba[0] == "right")) {
			value = true;
			return true;
		} else if (pba[0] == "avoid") {
			value = false;
			return true;
		}
	}
	return false;
}
