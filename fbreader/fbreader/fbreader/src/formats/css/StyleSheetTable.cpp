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

#include <cstdlib>
#include <algorithm>
#include <string.h>

#include "ZLStringUtil.h"
#include "ZLUnicodeUtil.h"

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
	if (other.WhiteSpace != WS_UNDEFINED) {
		WhiteSpace = other.WhiteSpace;
	}
	if (other.DisplayNone) {
		DisplayNone = other.DisplayNone;
	}
}

void StyleSheetTable::Style::inherit(const Style &other) {
	TextStyle.inherit(other.TextStyle);
	if (other.WhiteSpace != WS_UNDEFINED) {
		WhiteSpace = other.WhiteSpace;
	}
	if (other.DisplayNone) {
		DisplayNone = other.DisplayNone;
	}
}

bool StyleSheetTable::Style::equals(const Style &other) const {
	return PageBreakBefore == other.PageBreakBefore &&
		PageBreakAfter == other.PageBreakAfter &&
		WhiteSpace == other.WhiteSpace &&
		DisplayNone == other.DisplayNone &&
		TextStyle.equals(other.TextStyle);
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
		Style style(map);
		if (!style.empty()) {
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
			myEntries.push_back(Entry(stack, (a << 16) + (b << 8) + c, style));
		}
	}
}

bool StyleSheetTable::parseMargin(const std::string &toParse, short &size, ZLTextStyleEntry::SizeUnit &unit) {
	if (ZLTextStyleEntry::parseLength(toParse, size, unit)) {
		// Negative margins do make sense but we don't really support them
		if (size < 0) size = 0;
		return true;
	} else if (toParse == "auto") {
		size = 0;
		unit = ZLTextStyleEntry::SIZE_UNIT_AUTO;
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
	if (ZLTextStyleEntry::parseLength(value, size, unit)) {
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
	if (!entries.empty()) {
		std::sort(entries.begin(), entries.end(), sortBySpecificity);
		for (std::vector<const Entry*>::const_iterator e = entries.begin(); e != entries.end(); ++e) {
			style.apply((*e)->Style);
		}
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
	static const std::string TEXT_ALIGN("text-align");
	const std::vector<std::string> &alignment = values(styles, TEXT_ALIGN);
	if (!alignment.empty()) {
		const std::string &value = alignment[0];
		if (value == "justify") {
			entry.setAlignmentType(ALIGN_JUSTIFY);
		} else if (value == "left") {
			entry.setAlignmentType(ALIGN_LEFT);
		} else if (value == "right") {
			entry.setAlignmentType(ALIGN_RIGHT);
		} else if (value == "center") {
			entry.setAlignmentType(ALIGN_CENTER);
		}
	} else {
		static const std::string FLOAT("float");
		const std::vector<std::string> &floatVal = values(styles, FLOAT);
		if (!floatVal.empty()) {
			const std::string &value = floatVal[0];
			if (value == "left") {
				entry.setAlignmentType(ALIGN_LEFT);
			} else if (value == "right") {
				entry.setAlignmentType(ALIGN_RIGHT);
			}
		}
	}

	static const std::string FONT_WEIGHT("font-weight");
	const std::vector<std::string> &weight = values(styles, FONT_WEIGHT);
	if (!weight.empty()) {
		int num = -1;
		const std::string &value = weight[0];
		if (value == "bold") {
			num = 700;
		} else if (value == "normal") {
			num = 400;
		} else if ((value.length() == 3) &&
		           (value[1] == '0') &&
		           (value[2] == '0') &&
		           (value[0] >= '1') &&
		           (value[0] <= '9')) {
			num = 100 * (value[0] - '0');
		} else if (value == "bolder") {
		} else if (value == "lighter") {
		}
		if (num != -1) {
			entry.setFontModifier(FONT_MODIFIER_BOLD, num >= 600);
		}
	}

	static const std::string FONT_STYLE("font-style");
	const std::vector<std::string> &italic = values(styles, FONT_STYLE);
	if (!italic.empty()) {
		entry.setFontModifier(FONT_MODIFIER_ITALIC, italic[0] == "italic");
	}

	static const std::string FONT_VARIANT("font-variant");
	const std::vector<std::string> &variant = values(styles, FONT_VARIANT);
	if (!variant.empty()) {
		entry.setFontModifier(FONT_MODIFIER_SMALLCAPS, variant[0] == "small-caps");
	}

	static const std::string FONT_FAMILY("font-family");
	const std::vector<std::string> &fontFamilies = values(styles, FONT_FAMILY);
	if (!fontFamilies.empty()) entry.setFontFamilies(fontFamilies);

	short size;
	ZLTextStyleEntry::SizeUnit unit;
	static const std::string FONT_SIZE("font-size");
	const std::vector<std::string> &fontSize = values(styles, FONT_SIZE);
	if (!fontSize.empty()) {
		const std::string &value = fontSize[0];
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
			if (ZLTextStyleEntry::parseLength(value, size, unit)) {
				switch (unit) {
				case ZLTextStyleEntry::SIZE_UNIT_AUTO:
				case ZLTextStyleEntry::SIZE_UNIT_PIXEL:
					// What to do with pixels?
					break;
				case ZLTextStyleEntry::SIZE_UNIT_EM_100:
				case ZLTextStyleEntry::SIZE_UNIT_EX_100:
				case ZLTextStyleEntry::SIZE_UNIT_PERCENT:
					// Percent to magnification mapping algorithm
					// matches ZLTextForcedStyle::fontSize() logic
					if (size < 100) {
						if (size >= 80) {
							entry.setFontSizeMag(-1);
						} else {
							int mag;
							unsigned int x1 = 5*100, x2 = 6*size;
							// Too many iterations would cause 32-bit
							// overflow and generally don't make sense.
							for (mag=1; mag<=6 && x1<x2; ++mag) {
								x1 *= 5; x2 *= 6;
							}
							entry.setFontSizeMag(-mag);
						}
					} else if (size > 100) {
						if (size < 120) {
							entry.setFontSizeMag(1);
						} else {
							int mag;
							unsigned int x1 = 6*100, x2 = 5*size;
							for (mag=1; mag<=6 && x1<x2; ++mag) {
								x1 *= 6; x2 *= 5;
							}
							entry.setFontSizeMag(mag);
						}
					} else {
						entry.setFontSizeMag(0);
					}
					break;
				}
			}
		}
	}

	static const std::string OPACITY("opacity");
	const std::vector<std::string> &opacity = values(styles, OPACITY);
	if (!opacity.empty()) {
		const int value = (int)(255 * ZLStringUtil::stringToDouble(opacity[0], 1));
		entry.setOpacity((unsigned char)((value < 0) ? 0 : (value > 255) ? 255 : value));
	}

	static const std::string WIDTH("width");
	setLength(entry, ZLTextStyleEntry::LENGTH_WIDTH, styles, WIDTH);

	// Margins will overwrite padding, sorry
	static const std::string PADDING_TOP("padding-top");
	static const std::string PADDING_BOTTOM("padding-bottom");
	setLength(entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, styles, PADDING_TOP);
	setLength(entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER, styles, PADDING_BOTTOM);

	static const std::string MARGIN("margin");
	std::vector<std::string> margins = values(styles, MARGIN);
	if (!margins.empty()) {
		// Ignore the "!important" modifier for now
		if (margins.back() == "!important") {
			margins.pop_back();
		} else if (margins.back() == "important") {
			margins.pop_back();
			if (!margins.empty() && margins.back() == "!") {
				margins.pop_back();
			}
		}
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

	static const std::string MARGIN_LEFT("margin-left");
	static const std::string MARGIN_RIGHT("margin-right");
	static const std::string MARGIN_TOP("margin-top");
	static const std::string MARGIN_BOTTOM("margin-bottom");
	static const std::string TEXT_INDENT("text-indent");

	setMargin(entry, ZLTextStyleEntry::LENGTH_LEFT_INDENT, styles, MARGIN_LEFT);
	setMargin(entry, ZLTextStyleEntry::LENGTH_RIGHT_INDENT, styles, MARGIN_RIGHT);
	setLength(entry, ZLTextStyleEntry::LENGTH_FIRST_LINE_INDENT_DELTA, styles, TEXT_INDENT);
	setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_BEFORE, styles, MARGIN_TOP);
	setMargin(entry, ZLTextStyleEntry::LENGTH_SPACE_AFTER, styles, MARGIN_BOTTOM);

	static const std::string COLOR("color");
	std::vector<std::string> colors = values(styles, COLOR);
	if (colors.size() == 1) {
		ZLColor color;
		if (stringToColor(colors[0], color)) {
			entry.setColor(color);
		}
	}
}

void StyleSheetTable::getPageBreakValue(const std::vector<std::string> &values, ZLBoolean3 &value) {
	if (!values.empty()) {
		const std::string &first = values[0];
		if ((first == "always") ||
		    (first == "left") ||
		    (first == "right")) {
			value = B3_TRUE;
		} else if (first == "avoid") {
			value = B3_FALSE;
		}
	}
}

void StyleSheetTable::updateStyle(Style &style, const AttributeMap &map) {
	updateTextStyle(style.TextStyle, map);

	static const std::string PAGE_BREAK_BEFORE("page-break-before");
	getPageBreakValue(values(map, PAGE_BREAK_BEFORE), style.PageBreakBefore);

	static const std::string PAGE_BREAK_AFTER("page-break-after");
	getPageBreakValue(values(map, PAGE_BREAK_AFTER), style.PageBreakAfter);

	static const std::string WHITE_SPACE("white-space");
	const std::vector<std::string> &whiteSpace = values(map, WHITE_SPACE);
	if (!whiteSpace.empty()) {
		const std::string &value = whiteSpace[0];
		if (value == "normal") {
			style.WhiteSpace = WS_NORMAL;
		} else if (value == "nowrap") {
			style.WhiteSpace = WS_NOWRAP;
		} else if (value == "pre") {
			style.WhiteSpace = WS_PRE;
		} else if (value == "pre-wrap") {
			style.WhiteSpace = WS_PRE_WRAP;
		} else if (value == "pre-line") {
			style.WhiteSpace = WS_PRE_LINE;
		}
	}

	static const std::string DISPLAY("display");
	const std::vector<std::string> &display = values(map, DISPLAY);
	if (!display.empty() && display[0] == "none") {
		style.DisplayNone = true;
	}
}

bool StyleSheetTable::stringToColor(const std::string &text, ZLColor &color) {
	std::string str(text);
	ZLStringUtil::stripWhiteSpaces(str);
	if (!str.empty()) {
		// Color spec must be an ASCII string
		if (ZLUnicodeUtil::utf8Length(str) == (int)str.length()) {
			static const std::string RGB("rgb");
			if (str[0] == '#') {
				if (str.length() == 4) {
					// Hexadecimal notation #RGB
					int rgb[3];
					for (int i=0; i<3; i++) {
						if ((rgb[i] = ZLStringUtil::fromHex(str[i+1])) < 0) {
							return false;
						}
					}
					color.Red = ((rgb[0] << 4) | rgb[0]);
					color.Green = ((rgb[1] << 4) | rgb[1]);
					color.Blue = ((rgb[2] << 4) | rgb[2]);
					return true;
				} else if (str.length() == 7) {
					// Hexadecimal notation #RRGGBB
					int rrggbb[6];
					for (int i=0; i<6; i++) {
						if ((rrggbb[i] = ZLStringUtil::fromHex(str[i+1])) < 0) {
							return false;
						}
					}
					color.Red = ((rrggbb[0] << 4) | rrggbb[1]);
					color.Green = ((rrggbb[2] << 4) | rrggbb[3]);
					color.Blue = ((rrggbb[4] << 4) | rrggbb[5]);
					return true;
				}
			} else if (ZLStringUtil::stringStartsWith(str, RGB)) {
				// Functional Notation rgb(R,G,B)
				str.erase(0,3);
				ZLStringUtil::stripWhiteSpaces(str);
				if (ZLStringUtil::startsWith(str, '(') && ZLStringUtil::endsWith(str, ')')) {
					str = str.substr(1,str.length()-2);
					std::vector<std::string> rgb = ZLStringUtil::splitString(str, ",");
					if (rgb.size() == 3) {
						int i;
						long c[3];
						for (i=0; i<3; i++) {
							std::string tmp(rgb[i]);
							ZLStringUtil::stripWhiteSpaces(tmp);
							if (ZLStringUtil::endsWith(tmp, '%')) {
								tmp.resize(tmp.length()-1);
								if (ZLStringUtil::stringToLong(tmp, c[i]) && c[i] >= 0 && c[i] <= 100) {
									c[i] = (c[i] * 255 + 50) / 100;
									continue;
								}
							} else if (ZLStringUtil::stringToLong(tmp, c[i]) && c[i] >= 0 && c[i] <= 255) {
								continue;
							}
						}
						if (i == 3) {
							color.Red = c[0];
							color.Green = c[1];
							color.Blue = c[2];
							return true;
						}
					}
				}
			} else {
				// Color keywords
				static const struct _CSSColorEntry {
					const char* stringValue;
					long intValue;
				} knownColors [] = {
					{ "aliceblue", 0xf0f8ff },
					{ "antiquewhite", 0xfaebd7 },
					{ "aqua", 0x00ffff },
					{ "aquamarine", 0x7fffd4 },
					{ "azure", 0xf0ffff },
					{ "beige", 0xf5f5dc },
					{ "bisque", 0xffe4c4 },
					{ "black", 0x000000 },
					{ "blanchedalmond", 0xffe4c4 },
					{ "blue", 0x0000ff },
					{ "blueviolet", 0x8a2be2 },
					{ "brown", 0xa52a2a },
					{ "burlywood", 0xdeb887 },
					{ "cadetblue", 0x5f9ea0 },
					{ "chartreuse", 0x7fff00 },
					{ "chocolate", 0xd2691e },
					{ "coral", 0xff7f50 },
					{ "cornflowerblue", 0x6495ed },
					{ "cornsilk", 0xfff8dc },
					{ "crimson", 0xdc143c },
					{ "darkblue", 0x00008b },
					{ "darkcyan", 0x008b8b },
					{ "darkgoldenrod", 0xb8860b },
					{ "darkgray", 0xa9a9a9 },
					{ "darkgreen", 0x006400 },
					{ "darkkhaki", 0xbdb76b },
					{ "darkmagenta", 0x8b008b },
					{ "darkolivegreen", 0x556b2f },
					{ "darkorange", 0xff8c00 },
					{ "darkorchid", 0x9932cc },
					{ "darkred", 0x8b0000 },
					{ "darksalmon", 0xe9967a },
					{ "darkseagreen", 0x8fbc8f },
					{ "darkslateblue", 0x483d8b },
					{ "darkslategray", 0x2f4f4f },
					{ "darkturquoise", 0x00ced1 },
					{ "darkviolet", 0x9400d3 },
					{ "deeppink", 0xff1493 },
					{ "deepskyblue", 0x00bfff },
					{ "dimgray", 0x696969 },
					{ "dodgerblue", 0x1e90ff },
					{ "firebrick", 0xb22222 },
					{ "floralwhite", 0xfffaf0 },
					{ "forestgreen", 0x228b22 },
					{ "fuchsia", 0xff00ff },
					{ "gainsboro", 0xdcdcdc },
					{ "ghostwhite", 0xf8f8ff },
					{ "goldenrod", 0xdaa520 },
					{ "gold", 0xffd700 },
					{ "gray", 0x808080 },
					{ "green", 0x008000 },
					{ "greenyellow", 0xadff2f },
					{ "grey", 0x808080 },
					{ "honeydew", 0xf0fff0 },
					{ "hotpink", 0xff69b4 },
					{ "indianred", 0xcd5c5c },
					{ "indigo", 0x4b0082 },
					{ "ivory", 0xfffff0 },
					{ "khaki", 0xf0e68c },
					{ "lavenderblush", 0xfff0f5 },
					{ "lavender", 0xe6e6fa },
					{ "lawngreen", 0x7cfc00 },
					{ "lemonchiffon", 0xfffacd },
					{ "lightblue", 0xadd8e6 },
					{ "lightcoral", 0xf08080 },
					{ "lightcyan", 0xe0ffff },
					{ "lightgoldenrodyellow", 0xfafad2 },
					{ "lightgreen", 0x90ee90 },
					{ "lightgrey", 0xd3d3d3 },
					{ "lightpink", 0xffb6c1 },
					{ "lightsalmon", 0xffa07a },
					{ "lightseagreen", 0x20b2aa },
					{ "lightskyblue", 0x87cefa },
					{ "lightslategray", 0x778899 },
					{ "lightsteelblue", 0xb0c4de },
					{ "lightyellow", 0xffffe0 },
					{ "lime", 0x00ff00 },
					{ "limegreen", 0x32cd32 },
					{ "linen", 0xfaf0e6 },
					{ "maroon", 0x800000 },
					{ "mediumaquamarine", 0x66cdaa },
					{ "mediumblue", 0x0000cd },
					{ "mediumorchid", 0xba55d3 },
					{ "mediumpurple", 0x9370db },
					{ "mediumseagreen", 0x3cb371 },
					{ "mediumslateblue", 0x7b68ee },
					{ "mediumspringgreen", 0x00fa9a },
					{ "mediumturquoise", 0x48d1cc },
					{ "mediumvioletred", 0xc71585 },
					{ "midnightblue", 0x191970 },
					{ "mintcream", 0xf5fffa },
					{ "mistyrose", 0xffe4e1 },
					{ "moccasin", 0xffe4b5 },
					{ "navajowhite", 0xffdead },
					{ "navy", 0x000080 },
					{ "oldlace", 0xfdf5e6 },
					{ "olive", 0x808000 },
					{ "olivedrab", 0x6b8e23 },
					{ "orange", 0xffa500 },
					{ "orangered", 0xff4500 },
					{ "orchid", 0xda70d6 },
					{ "palegoldenrod", 0xeee8aa },
					{ "palegreen", 0x98fb98 },
					{ "paleturquoise", 0xafeeee },
					{ "palevioletred", 0xdb7093 },
					{ "papayawhip", 0xffefd5 },
					{ "peachpuff", 0xffdab9 },
					{ "peru", 0xcd853f },
					{ "pink", 0xffc0cb },
					{ "plum", 0xdda0dd },
					{ "powderblue", 0xb0e0e6 },
					{ "purple", 0x800080 },
					{ "rebeccapurple", 0x663399 },
					{ "red", 0xff0000 },
					{ "rosybrown", 0xbc8f8f },
					{ "royalblue", 0x4169e1 },
					{ "saddlebrown", 0x8b4513 },
					{ "salmon", 0xfa8072 },
					{ "sandybrown", 0xf4a460 },
					{ "seagreen", 0x2e8b57 },
					{ "seashell", 0xfff5ee },
					{ "sienna", 0xa0522d },
					{ "silver", 0xc0c0c0 },
					{ "skyblue", 0x87ceeb },
					{ "slateblue", 0x6a5acd },
					{ "slategrey", 0x708090 },
					{ "snow", 0xfffafa },
					{ "springgreen", 0x00ff7f },
					{ "steelblue", 0x4682b4 },
					{ "tan", 0xd2b48c },
					{ "teal", 0x008080 },
					{ "thistle", 0xd8bfd8 },
					{ "tomato", 0xff6347 },
					{ "turquoise", 0x40e0d0 },
					{ "violet", 0xee82ee },
					{ "wheat", 0xf5deb3 },
					{ "white", 0xffffff },
					{ "whitesmoke", 0xf5f5f5 },
					{ "yellow", 0xffff00 },
					{ "yellowgreen", 0x9acd32 }
				};

				int low = 0;
				int high = (sizeof(knownColors)/sizeof(knownColors[0]))-1;
				const char* key = str.c_str();

				while (low <= high) {
					const int mid = (low + high)/2;
					int cmp = strcasecmp(knownColors[mid].stringValue, key);
					if (cmp < 0) {
						low = mid + 1;
					} else if (cmp > 0) {
						high = mid - 1;
					} else {
						color.setIntValue(knownColors[mid].intValue);
						return true;
					}
				}
			}
		}
	}
	return false;
}
