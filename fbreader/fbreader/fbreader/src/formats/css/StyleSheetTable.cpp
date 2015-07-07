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
#include <algorithm>

#include <ZLStringUtil.h>

#include "StyleSheetTable.h"

static const std::string WILDCARD("*");

StyleSheetTable::Element::Element(const std::string &tag, const char *klass, const char* id) :
	Name(tag), Classes(ZLStringUtil::splitString(klass, ", \n")) {
	if (id) Id = id;
}

StyleSheetTable::Selector::Selector(const std::string &selector) {
	std::string buf(selector);
	const int dot = buf.find('.');
	if (dot >= 0) {
		myClass = buf.substr(dot + 1);
		buf = buf.substr(0, dot);
	}
	const int hash = buf.find('#');
	if (hash < 0) {
		myType = buf;
	} else {
		myType = buf.substr(0, hash);
		myId = buf.substr(hash + 1);
	}
	if (myType == "*") myType.clear();
}

bool StyleSheetTable::Selector::match(const std::string &pattern, const std::string &str) {
	return pattern.empty() || pattern == WILDCARD || pattern == str;
}

bool StyleSheetTable::Selector::match(const std::string &pattern, const std::vector<std::string> &strings) {
	return pattern.empty() || pattern == WILDCARD || std::find(strings.begin(), strings.end(), pattern) != strings.end();
}

void StyleSheetTable::Style::apply(const Style &other) {
	TextStyle.apply(other.TextStyle);
	if (other.PageBreakBefore != B3_UNDEFINED) {
		PageBreakBefore = other.PageBreakBefore;
	}
	if (other.PageBreakAfter != B3_UNDEFINED) {
		PageBreakAfter = other.PageBreakAfter;
	}
}

bool StyleSheetTable::Entry::match(const ElementList &stack) const {
	if (!stack.empty() && !Selectors.empty()) {
		SelectorList::const_reverse_iterator it(Selectors.rbegin());
		ElementList::const_reverse_iterator match_it(stack.rbegin());
		if ((*it).match(*match_it)) {
			++it;
			++match_it;
			while (it != Selectors.rend()) {
				const Selector &selector = (*it);
				while (match_it != stack.rend() && !selector.match(*match_it)) {
					++match_it;
				}
				if (match_it == stack.rend()) {
					break;
				}
				++it;
			}
			if (it == Selectors.rend()) {
				return true;
			}
		}
	}
	return false;
}

void StyleSheetTable::addMap(const std::vector<std::string> &selectors, const AttributeMap &map) {
	if ((!selectors.empty()) && !map.empty()) {
		// http://www.w3.org/TR/selectors/#specificity
		int a = 0, b = 0, c = 0;
		SelectorList stack;
		for (unsigned int i=0; i<selectors.size(); i++) {
			const Selector &selector = selectors[i];
			a += selector.a();
			b += selector.b();
			c += selector.c();
			stack.push_back(selector);
		}
		if (a > 255) a = 255;
		if (b > 255) b = 255;
		if (c > 255) c = 255;
		myEntries.push_back(Entry(stack, (a << 16) + (b << 8) + c, map));
	}
}

bool StyleSheetTable::parseLength(const std::string &toParse, short &size, ZLTextStyleEntry::SizeUnit &unit) {
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

bool StyleSheetTable::parseMargin(const std::string &toParse, short &size, ZLTextStyleEntry::SizeUnit &unit) {
	if (parseLength(toParse, size, unit)) {
		// Negative margins do make sense but we don't really support them
		if (size < 0) size = 0;
		return true;
	} else if (toParse == "auto") {
		size = 0;
		unit = ZLTextStyleEntry::SIZE_UNIT_PIXEL;
		return true;
	} else {
		return false;
	}
}

void StyleSheetTable::setLength(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const AttributeMap &map, const std::string &attributeName) {
	AttributeMap::const_iterator it = map.find(attributeName);
	if (it != map.end()) {
		const std::vector<std::string> &values = it->second;
		if (!values.empty()) {
			setLength(entry, name, values[0]);
		}
	}
}

void StyleSheetTable::setLength(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const std::string &value) {
	short size;
	ZLTextStyleEntry::SizeUnit unit;
	if (parseLength(value, size, unit)) {
		entry.setLength(name, size, unit);
	}
}

void StyleSheetTable::setMargin(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const AttributeMap &map, const std::string &attributeName) {
	AttributeMap::const_iterator it = map.find(attributeName);
	if (it != map.end()) {
		const std::vector<std::string> &values = it->second;
		if (!values.empty()) {
			setMargin(entry, name, values[0]);
		}
	}
}

void StyleSheetTable::setMargin(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const std::string &value) {
	short size;
	ZLTextStyleEntry::SizeUnit unit;
	if (parseMargin(value, size, unit)) {
		entry.setLength(name, size, unit);
	}
}

bool StyleSheetTable::sortBySpecificity(const Entry *e1, const Entry *e2) {
	return e1->Specificity < e2->Specificity;
}

void StyleSheetTable::applyStyles(const ElementList &stack, Style &style) const {
	std::vector<const Entry*> entries;
	for (std::vector<Entry>::const_iterator i = myEntries.begin(); i != myEntries.end(); ++i) {
		if ((*i).match(stack)) {
			entries.push_back(&(*i));
		}
	}
	std::sort(entries.begin(), entries.end(), sortBySpecificity);
	for (std::vector<const Entry*>::const_iterator e = entries.begin(); e != entries.end(); ++e) {
		style.apply((*e)->Style);
	}
}

const std::vector<std::string> &StyleSheetTable::values(const AttributeMap &map, const std::string &name) {
	const AttributeMap::const_iterator it = map.find(name);
	if (it != map.end()) {
		return it->second;
	}
	static const std::vector<std::string> emptyVector;
	return emptyVector;
}

void StyleSheetTable::updateTextStyle(ZLTextStyleEntry &entry, const AttributeMap &styles) {
	const std::vector<std::string> &alignment = values(styles, "text-align");
	if (!alignment.empty()) {
		if (alignment[0] == "justify") {
			entry.setAlignmentType(ALIGN_JUSTIFY);
		} else if (alignment[0] == "left") {
			entry.setAlignmentType(ALIGN_LEFT);
		} else if (alignment[0] == "right") {
			entry.setAlignmentType(ALIGN_RIGHT);
		} else if (alignment[0] == "center") {
			entry.setAlignmentType(ALIGN_CENTER);
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
			entry.setFontModifier(FONT_MODIFIER_BOLD, num >= 600);
		}
	}

	const std::vector<std::string> &italic = values(styles, "font-style");
	if (!italic.empty()) {
		entry.setFontModifier(FONT_MODIFIER_ITALIC, italic[0] == "italic");
	}

	const std::vector<std::string> &variant = values(styles, "font-variant");
	if (!variant.empty()) {
		entry.setFontModifier(FONT_MODIFIER_SMALLCAPS, variant[0] == "small-caps");
	}

	const std::vector<std::string> &fontFamily = values(styles, "font-family");
	if (!fontFamily.empty() && !fontFamily[0].empty()) {
		entry.setFontFamily(fontFamily[0]);
	}

	short size;
	ZLTextStyleEntry::SizeUnit unit;
	const std::vector<std::string> &fontSize = values(styles, "font-size");
	if (!fontSize.empty()) {
		std::string value = fontSize[0];
		if (value == "xx-small") {
			entry.setFontSizeMag(-3);
		} else if (value == "x-small") {
			entry.setFontSizeMag(-2);
		} else if (value == "small" || value == "smaller") {
			entry.setFontSizeMag(-1);
		} else if (value == "medium") {
			entry.setFontSizeMag(0);
		} else if (value == "large" || value == "larger") {
			entry.setFontSizeMag(1);
		} else if (value == "x-large") {
			entry.setFontSizeMag(2);
		} else if (value == "xx-large") {
			entry.setFontSizeMag(3);
		} else {
			if (parseLength(value, size, unit)) {
				switch (unit) {
				case ZLTextStyleEntry::SIZE_UNIT_PIXEL:
					// What to do with pixels?
					break;
				case ZLTextStyleEntry::SIZE_UNIT_EM_100:
				case ZLTextStyleEntry::SIZE_UNIT_EX_100:
				case ZLTextStyleEntry::SIZE_UNIT_PERCENT:
					entry.setFontSizeMag((size < 100 && size > 80) ? -1 :
					                     (size > 100 && size < 120) ? 1 :
					                     (size - 100)/20);
					break;
				}
			}
		}
	}

	const std::vector<std::string> &opacity = values(styles, "opacity");
	if (!opacity.empty()) {
		const int value = (int)(255 * ZLStringUtil::stringToDouble(opacity[0], 1));
		entry.setOpacity((unsigned char)((value < 0) ? 0 : (value > 255) ? 255 : value));
	}

	std::vector<std::string> margins = values(styles, "margin");
	if (!margins.empty() && margins.back() == "!important") {
		// Ignore the "!important" modifier for now
		margins.pop_back();
	}
	switch (margins.size()) {
	case 1:
		if (parseMargin(margins[0], size, unit)) {
			entry.setLength(ZLTextStyleEntry::LENGTH_SPACE_BEFORE, size, unit);
			entry.setLength(ZLTextStyleEntry::LENGTH_RIGHT_INDENT, size, unit);
			entry.setLength(ZLTextStyleEntry::LENGTH_SPACE_AFTER, size, unit);
			entry.setLength(ZLTextStyleEntry::LENGTH_LEFT_INDENT, size, unit);
		}
		break;
	case 2:
		if (parseMargin(margins[0], size, unit)) {
			entry.setLength(ZLTextStyleEntry::LENGTH_SPACE_BEFORE, size, unit);
			entry.setLength(ZLTextStyleEntry::LENGTH_SPACE_AFTER, size, unit);
		}
		if (parseMargin(margins[1], size, unit)) {
			entry.setLength(ZLTextStyleEntry::LENGTH_RIGHT_INDENT, size, unit);
			entry.setLength(ZLTextStyleEntry::LENGTH_LEFT_INDENT, size, unit);
		}
		break;
	case 3:
		setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, margins[0]);
		if (parseMargin(margins[1], size, unit)) {
			entry.setLength(ZLTextStyleEntry::LENGTH_RIGHT_INDENT, size, unit);
			entry.setLength(ZLTextStyleEntry::LENGTH_LEFT_INDENT,  size, unit);
		}
		setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER,  margins[2]);
		break;
	case 4:
		setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, margins[0]);
		setMargin(entry, ZLTextStyleEntry::LENGTH_RIGHT_INDENT, margins[1]);
		setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER,  margins[2]);
		setMargin(entry, ZLTextStyleEntry::LENGTH_LEFT_INDENT,  margins[3]);
		break;
	}

	setMargin(entry, ZLTextStyleEntry::LENGTH_LEFT_INDENT, styles, "margin-left");
	setMargin(entry, ZLTextStyleEntry::LENGTH_RIGHT_INDENT, styles, "margin-right");
	setLength(entry, ZLTextStyleEntry::LENGTH_FIRST_LINE_INDENT_DELTA, styles, "text-indent");
	setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, styles, "margin-top");
	setLength(entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, styles, "padding-top");
	setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER, styles, "margin-bottom");
	setLength(entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER, styles, "padding-bottom");
}

bool StyleSheetTable::getPageBreakBefore(const AttributeMap &map, ZLBoolean3 &value) {
	const std::vector<std::string> &pbb = values(map, "page-break-before");
	if (!pbb.empty()) {
		if ((pbb[0] == "always") ||
		    (pbb[0] == "left") ||
		    (pbb[0] == "right")) {
			value = B3_TRUE;
			return true;
		} else if (pbb[0] == "avoid") {
			value = B3_FALSE;
			return true;
		}
	}
	return false;
}

bool StyleSheetTable::getPageBreakAfter(const AttributeMap &map, ZLBoolean3 &value) {
	const std::vector<std::string> &pba = values(map, "page-break-after");
	if (!pba.empty()) {
		if ((pba[0] == "always") ||
		    (pba[0] == "left") ||
		    (pba[0] == "right")) {
			value = B3_TRUE;
			return true;
		} else if (pba[0] == "avoid") {
			value = B3_FALSE;
			return true;
		}
	}
	return false;
}

void StyleSheetTable::updateStyle(Style &style, const AttributeMap &map) {
	updateTextStyle(style.TextStyle, map);
	getPageBreakBefore(map, style.PageBreakBefore);
	getPageBreakAfter(map, style.PageBreakAfter);
}
