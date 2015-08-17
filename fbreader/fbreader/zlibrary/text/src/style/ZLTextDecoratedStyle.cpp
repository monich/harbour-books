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

#include "ZLTextStyle.h"
#include "ZLTextStyleCollection.h"
#include "ZLTextDecoratedStyle.h"

static const std::string STYLE = "Style";

ZLTextStyleDecoration::ZLTextStyleDecoration(const std::string &name, int fontSizeDelta, ZLBoolean3 bold, ZLBoolean3 italic, int verticalShift, ZLBoolean3 allowHyphenations) :
	FontFamilyOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":fontFamily", std::string()),
	FontSizeDeltaOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":fontSize", -16, 16, fontSizeDelta),
	BoldOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":bold", bold),
	ItalicOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":italic", italic),
	VerticalShiftOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":vShift", verticalShift),
	AllowHyphenationsOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":allowHyphenations", allowHyphenations),
	myName(name), myFirstLineIndentDelta(NULL), myFirstLineIndentDeltaUnit(ZLTextStyleEntry::SIZE_UNIT_PIXEL) {
}

ZLTextFullStyleDecoration::ZLTextFullStyleDecoration(const std::string &name, int fontSizeDelta, ZLBoolean3 bold, ZLBoolean3 italic, short spaceBefore, short spaceAfter, short lineStartIndent, short lineEndIndent, short firstLineIndentDelta, int verticalShift, ZLTextAlignmentType alignment, double lineSpace, ZLBoolean3 allowHyphenations) : ZLTextStyleDecoration(name, fontSizeDelta, bold, italic, verticalShift, allowHyphenations),
	SpaceBeforeOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":spaceBefore", -10, 100, spaceBefore),
	SpaceAfterOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":spaceAfter", -10, 100, spaceAfter),
	LineStartIndentOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":leftIndent", -300, 300, lineStartIndent),
	LineEndIndentOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":rightIndent", -300, 300, lineEndIndent),
	AlignmentOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":alignment", alignment),
	LineSpaceOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":lineSpace", lineSpace),
	LineSpacePercentOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":lineSpacePercent", (lineSpace == 0.0) ? -1 : (int)(lineSpace * 100)),
	FirstLineIndentDeltaOption(ZLCategoryKey::LOOK_AND_FEEL, STYLE, name + ":firstLineIndentDelta", -300, 300, firstLineIndentDelta),
	SpaceBeforeOptionUnit(ZLTextStyleEntry::SIZE_UNIT_PIXEL),
	SpaceAfterOptionUnit(ZLTextStyleEntry::SIZE_UNIT_PIXEL),
	LineStartIndentOptionUnit(ZLTextStyleEntry::SIZE_UNIT_PIXEL),
	LineEndIndentOptionUnit(ZLTextStyleEntry::SIZE_UNIT_PIXEL),
	FirstLineIndentDeltaOptionUnit(ZLTextStyleEntry::SIZE_UNIT_PIXEL) {
}

const ZLTextFullStyleDecoration *ZLTextStyleDecoration::fullDecoration() const {
	return NULL;
}

shared_ptr<ZLTextStyle> ZLTextStyleDecoration::createDecoratedStyle(const shared_ptr<ZLTextStyle> base) const {
	return new ZLTextPartialDecoratedStyle(base, *this);
}

const ZLTextFullStyleDecoration *ZLTextFullStyleDecoration::fullDecoration() const {
	return this;
}

shared_ptr<ZLTextStyle> ZLTextFullStyleDecoration::createDecoratedStyle(const shared_ptr<ZLTextStyle> base) const {
	return new ZLTextFullDecoratedStyle(base, *this);
}

short ZLTextFullDecoratedStyle::spaceBefore(const ZLTextStyleEntry::Metrics &metrics) const {
	return ZLTextStyleEntry::vlength(myDecoration.SpaceBeforeOption.value(), myDecoration.SpaceBeforeOptionUnit, metrics);
}

short ZLTextFullDecoratedStyle::spaceAfter(const ZLTextStyleEntry::Metrics &metrics) const {
	return ZLTextStyleEntry::vlength(myDecoration.SpaceAfterOption.value(), myDecoration.SpaceAfterOptionUnit, metrics);
}

short ZLTextFullDecoratedStyle::lineStartIndent(const ZLTextStyleEntry::Metrics &metrics, bool rtl) const {
	return base()->lineStartIndent(metrics, rtl) + (rtl ?
		ZLTextStyleEntry::hlength(myDecoration.LineEndIndentOption.value(), myDecoration.LineEndIndentOptionUnit, metrics) :
		ZLTextStyleEntry::hlength(myDecoration.LineStartIndentOption.value(), myDecoration.LineStartIndentOptionUnit, metrics));
}

short ZLTextFullDecoratedStyle::lineEndIndent(const ZLTextStyleEntry::Metrics &metrics, bool rtl) const {
	return base()->lineEndIndent(metrics, rtl) + (rtl ?
		ZLTextStyleEntry::hlength(myDecoration.LineStartIndentOption.value(), myDecoration.LineStartIndentOptionUnit, metrics) :
		ZLTextStyleEntry::hlength(myDecoration.LineEndIndentOption.value(), myDecoration.LineEndIndentOptionUnit, metrics));
}

int ZLTextFullStyleDecoration::firstLineIndentDelta(const ZLTextStyleEntry::Metrics &metrics) const {
	ZLTextStyleEntry::SizeUnit unit;
	int size;
	int* indent = ZLTextStyleDecoration::firstLineIndentDelta(unit);
	if (indent) {
		size = *indent;
	} else {
		size = FirstLineIndentDeltaOption.value();
		unit = FirstLineIndentDeltaOptionUnit;
	}
	return ZLTextStyleEntry::hlength(size, unit, metrics);
}

ZLTextPartialDecoratedStyle::ZLTextPartialDecoratedStyle(const shared_ptr<ZLTextStyle> base, const ZLTextStyleDecoration &decoration) :
	ZLTextDecoratedStyle(base), myDecoration(decoration) {
	const std::string &family = myDecoration.FontFamilyOption.value();
	if (!family.empty()) myFontFamilies.push_back(family);
}

const std::vector<std::string> &ZLTextPartialDecoratedStyle::fontFamilies() const {
	return (!myFontFamilies.empty()) ? myFontFamilies : base()->fontFamilies();
}

int ZLTextPartialDecoratedStyle::fontSize() const {
	return base()->fontSize() + myDecoration.FontSizeDeltaOption.value();
}

bool ZLTextPartialDecoratedStyle::bold() const {
	ZLBoolean3 b = myDecoration.BoldOption.value();
	return (b == B3_UNDEFINED) ? base()->bold() : (b == B3_TRUE);
}

bool ZLTextPartialDecoratedStyle::italic() const {
	ZLBoolean3 i = myDecoration.ItalicOption.value();
	return (i == B3_UNDEFINED) ? base()->italic() : (i == B3_TRUE);
}

bool ZLTextPartialDecoratedStyle::allowHyphenations() const {
	ZLBoolean3 a = myDecoration.AllowHyphenationsOption.value();
	return (a == B3_UNDEFINED) ? base()->allowHyphenations() : (a == B3_TRUE);
}

short ZLTextPartialDecoratedStyle::firstLineIndentDelta(const ZLTextStyleEntry::Metrics &metrics) const {
	ZLTextStyleEntry::SizeUnit unit;
	int* indent = myDecoration.firstLineIndentDelta(unit);
	return indent ? ZLTextStyleEntry::hlength(*indent, unit, metrics) :base()->firstLineIndentDelta(metrics);
}

ZLTextFullDecoratedStyle::ZLTextFullDecoratedStyle(const shared_ptr<ZLTextStyle> base, const ZLTextFullStyleDecoration &decoration) :
	ZLTextDecoratedStyle(base), myDecoration(decoration) {
	const std::string &family = myDecoration.FontFamilyOption.value();
	if (!family.empty()) myFontFamilies.push_back(family);
}

short ZLTextFullDecoratedStyle::firstLineIndentDelta(const ZLTextStyleEntry::Metrics &metrics) const {
	return (alignment() == ALIGN_CENTER) ? 0 : myDecoration.firstLineIndentDelta(metrics);
}

const std::vector<std::string> &ZLTextFullDecoratedStyle::fontFamilies() const {
	return (!myFontFamilies.empty()) ? myFontFamilies : base()->fontFamilies();
}

int ZLTextFullDecoratedStyle::fontSize() const {
	return base()->fontSize() + myDecoration.FontSizeDeltaOption.value();
}

bool ZLTextFullDecoratedStyle::bold() const {
	ZLBoolean3 b = myDecoration.BoldOption.value();
	return (b == B3_UNDEFINED) ? base()->bold() : (b == B3_TRUE);
}

bool ZLTextFullDecoratedStyle::italic() const {
	ZLBoolean3 i = myDecoration.ItalicOption.value();
	return (i == B3_UNDEFINED) ? base()->italic() : (i == B3_TRUE);
}

ZLTextAlignmentType ZLTextFullDecoratedStyle::alignment() const {
	ZLTextAlignmentType a = (ZLTextAlignmentType)myDecoration.AlignmentOption.value();
	return (a == ALIGN_UNDEFINED) ? base()->alignment() : a;
}

bool ZLTextFullDecoratedStyle::allowHyphenations() const {
	ZLBoolean3 a = myDecoration.AllowHyphenationsOption.value();
	return (a == B3_UNDEFINED) ? base()->allowHyphenations() : (a == B3_TRUE);
}

const std::string &ZLTextPartialDecoratedStyle::colorStyle() const {
	const std::string &style = myDecoration.colorStyle();
	return style.empty() ? base()->colorStyle() : style;
}

const std::string &ZLTextFullDecoratedStyle::colorStyle() const {
	const std::string &style = myDecoration.colorStyle();
	return style.empty() ? base()->colorStyle() : style;
}

ZLTextForcedStyle::ZLTextForcedStyle(shared_ptr<ZLTextStyle> base, const ZLTextStyleEntry &entry) :
	ZLTextDecoratedStyle(base), myEntry(entry) {
	if (myEntry.colorSupported()) {
		myColorStyle = ZLTextStyle::colorStyle(myEntry.color());
	}
}

short ZLTextForcedStyle::lineStartIndent(const ZLTextStyleEntry::Metrics &metrics, bool rtl) const {
	ZLTextStyleEntry::Length lengthType = rtl ?
		ZLTextStyleEntry::LENGTH_RIGHT_INDENT :
		ZLTextStyleEntry::LENGTH_LEFT_INDENT;
	if (!myEntry.lengthSupported(lengthType)) {
		return base()->lineStartIndent(metrics, rtl);
	}

	const short baseLen = base()->lineStartIndent(metrics, rtl);
	ZLTextStyleEntry::Metrics adjusted(metrics);
	adjusted.FullWidth -= baseLen + base()->lineEndIndent(metrics, rtl);
	if (adjusted.FullWidth < 0) adjusted.FullWidth = 0;
	if (myEntry.lengthUnit(lengthType) != ZLTextStyleEntry::SIZE_UNIT_AUTO) {
		return baseLen + myEntry.length(lengthType, adjusted);
	}

	if (myEntry.autoLeftRightMargins() && myEntry.lengthSupported(ZLTextStyleEntry::LENGTH_WIDTH)) {
		int x = adjusted.FullWidth - myEntry.length(ZLTextStyleEntry::LENGTH_WIDTH, adjusted);
		if (x < 0) x = 0;
		return x/2;
	}

	return baseLen;
}

short ZLTextForcedStyle::lineEndIndent(const ZLTextStyleEntry::Metrics &metrics, bool rtl) const {
	ZLTextStyleEntry::Length lengthType = rtl ?
		ZLTextStyleEntry::LENGTH_LEFT_INDENT :
		ZLTextStyleEntry::LENGTH_RIGHT_INDENT;

	if (!myEntry.lengthSupported(lengthType)) {
		return base()->lineStartIndent(metrics, rtl);
	}

	const short baseLen = base()->lineEndIndent(metrics, rtl);
	ZLTextStyleEntry::Metrics adjusted(metrics);
	adjusted.FullWidth -= baseLen + base()->lineStartIndent(metrics, rtl);
	if (adjusted.FullWidth < 0) adjusted.FullWidth = 0;
	if (myEntry.lengthUnit(lengthType) != ZLTextStyleEntry::SIZE_UNIT_AUTO) {
		return baseLen + myEntry.length(lengthType, adjusted);
	}

	if (myEntry.autoLeftRightMargins() && myEntry.lengthSupported(ZLTextStyleEntry::LENGTH_WIDTH)) {
		int x = adjusted.FullWidth - myEntry.length(ZLTextStyleEntry::LENGTH_WIDTH, adjusted);
		if (x < 0) x = 0;
		return x/2;
	}

	return baseLen;
}

short ZLTextForcedStyle::spaceBefore(const ZLTextStyleEntry::Metrics &metrics) const {
	return
		myEntry.lengthSupported(ZLTextStyleEntry::LENGTH_SPACE_BEFORE) ?
			myEntry.length(ZLTextStyleEntry::LENGTH_SPACE_BEFORE, metrics) :
			base()->spaceBefore(metrics);
}

short ZLTextForcedStyle::spaceAfter(const ZLTextStyleEntry::Metrics &metrics) const {
	return
		myEntry.lengthSupported(ZLTextStyleEntry::LENGTH_SPACE_AFTER) ?
			myEntry.length(ZLTextStyleEntry::LENGTH_SPACE_AFTER, metrics) :
			base()->spaceAfter(metrics);
}

short ZLTextForcedStyle::firstLineIndentDelta(const ZLTextStyleEntry::Metrics &metrics) const {
	return
		myEntry.lengthSupported(ZLTextStyleEntry::LENGTH_FIRST_LINE_INDENT_DELTA) ?
			myEntry.length(ZLTextStyleEntry::LENGTH_FIRST_LINE_INDENT_DELTA, metrics) :
			base()->firstLineIndentDelta(metrics);
}

ZLTextAlignmentType ZLTextForcedStyle::alignment() const {
	return myEntry.alignmentTypeSupported() ? myEntry.alignmentType() : base()->alignment();
}

bool ZLTextForcedStyle::bold() const {
	return (myEntry.supportedFontModifier() & FONT_MODIFIER_BOLD) ?
					 (myEntry.fontModifier() & FONT_MODIFIER_BOLD) : base()->bold();
}

bool ZLTextForcedStyle::italic() const {
	return (myEntry.supportedFontModifier() & FONT_MODIFIER_ITALIC) ?
					 (myEntry.fontModifier() & FONT_MODIFIER_ITALIC) : base()->italic();
}

int ZLTextForcedStyle::fontSize() const {
	if (myEntry.fontSizeSupported()) {
		shared_ptr<ZLTextStyle> base = this->base();
		while (base->isDecorated()) {
			base = ((ZLTextDecoratedStyle&)*base).base();
		}
		int size = base->fontSize();
		const int mag = myEntry.fontSizeMag();
		if (mag >= 0) {
			for (int i = 0; i < mag; ++i) {
				size *= 6;
			}
			for (int i = 0; i < mag; ++i) {
				size /= 5;
			}
		} else {
			for (int i = 0; i > mag; --i) {
				size *= 5;
			}
			for (int i = 0; i > mag; --i) {
				size /= 6;
			}
		}
		return size;
	} else {
		return base()->fontSize();
	}
}

const std::vector<std::string> &ZLTextForcedStyle::fontFamilies() const {
	return (!ZLTextStyleCollection::Instance().OverrideSpecifiedFontsOption.value() &&
					myEntry.fontFamiliesSupported()) ?
					myEntry.fontFamilies() : base()->fontFamilies();
}

const std::string &ZLTextForcedStyle::colorStyle() const {
	return myEntry.colorSupported() ? myColorStyle : base()->colorStyle();
}

const std::string &ZLTextStyleDecoration::colorStyle() const {
	return myColorStyle;
}

void ZLTextStyleDecoration::setColorStyle(const std::string &colorStyle) {
	myColorStyle = colorStyle;
}
