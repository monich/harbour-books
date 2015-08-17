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

#include <string.h>

#include <algorithm>

#include <ZLUnicodeUtil.h>
#include <ZLStringUtil.h>
#include <ZLImage.h>

#include "ZLTextParagraph.h"

const shared_ptr<ZLTextParagraphEntry> ResetBidiEntry::Instance = new ResetBidiEntry();

size_t ZLTextEntry::dataLength() const {
	size_t len;
	memcpy(&len, myAddress, sizeof(size_t));
	return len;
}

int ZLTextStyleEntry::hlength(int size, SizeUnit unit, const Metrics &metrics)
{
	switch (unit) {
		case SIZE_UNIT_PIXEL:
			return size;
		case SIZE_UNIT_EM_100:
			return (size * metrics.FontSize + 50) / 100;
		case SIZE_UNIT_EX_100:
			return (size * metrics.FontXHeight + 50) / 100;
		case SIZE_UNIT_PERCENT:
			return (size * metrics.FullWidth + 50) / 100;
		case SIZE_UNIT_AUTO:
			return metrics.FullWidth / 2;
	}
	return 0;
}

int ZLTextStyleEntry::vlength(int size, SizeUnit unit, const Metrics &metrics)
{
	switch (unit) {
		case SIZE_UNIT_PIXEL:
			return size;
		case SIZE_UNIT_EM_100:
			return (size * metrics.FontSize + 50) / 100;
		case SIZE_UNIT_EX_100:
			return (size * metrics.FontXHeight + 50) / 100;
		case SIZE_UNIT_PERCENT:
			return (size * metrics.FullHeight + 50) / 100;
		case SIZE_UNIT_AUTO:
			return metrics.FullHeight / 2;
	}
	return 0;
}

short ZLTextStyleEntry::length(Length name, const Metrics &metrics) const {
	switch (name) {
		case LENGTH_LEFT_INDENT:
		case LENGTH_RIGHT_INDENT:
		case LENGTH_FIRST_LINE_INDENT_DELTA:
		case LENGTH_WIDTH:
			return hlength(myLengths[name].Size, myLengths[name].Unit, metrics);
		case LENGTH_SPACE_BEFORE:
		case LENGTH_SPACE_AFTER:
			return vlength(myLengths[name].Size, myLengths[name].Unit, metrics);
		case NUMBER_OF_LENGTHS:
			break;
	}
	return 0;
}

ZLTextStyleEntry::ZLTextStyleEntry(char *address) {
	mySupportedFontModifier = *address++;
	memcpy(&myMask, address, sizeof(int));
	address += sizeof(int);
	for (int i = 0; i < NUMBER_OF_LENGTHS; ++i) {
		if (myMask & (1 << i)) {
			myLengths[i].Unit = (SizeUnit)*address++;
			memcpy(&myLengths[i].Size, address, sizeof(short));
			address += sizeof(short);
		}
	}
	if (opacitySupported()) {
		myOpacity = *address++;
	}
	if (alignmentTypeSupported()) {
		myAlignmentType = (ZLTextAlignmentType)*address++;
	}
	if (supportedFontModifier()) {
		myFontModifier = *address++;
	}
	if (fontSizeSupported()) {
		myFontSizeMag = (signed char)*address++;
	}
	if (fontFamiliesSupported()) {
		unsigned char n = *address++;
		for (unsigned int i = 0; i < n; ++i) {
			std::string font(address);
			address += font.length() + 1;
			myFontFamilies.push_back(font);
		}
	}
	if (colorSupported()) {
		ZLColor color;
		color.Red = (unsigned char)*address++;
		color.Green = (unsigned char)*address++;
		color.Blue = (unsigned char)*address++;
		setColor(color);
	}
}

void ZLTextStyleEntry::reset() {
	mySupportedFontModifier = 0;
	myMask = 0;
	myFontFamilies.resize(0);
}

void ZLTextStyleEntry::apply(const ZLTextStyleEntry &other) {
	if (other.myMask & ((1 << NUMBER_OF_LENGTHS) - 1)) {
		for (int i = 0; i < NUMBER_OF_LENGTHS; ++i) {
			if (other.myMask & (1 << i)) {
				myLengths[i] = other.myLengths[i];
				myMask |= (1 << i);
			}
		}
	}
	if (other.myMask & SUPPORT_OPACITY) {
		setOpacity(other.myOpacity);
	}
	if (other.myMask & SUPPORT_ALIGNMENT_TYPE) {
		setAlignmentType(other.myAlignmentType);
	}
	if (other.mySupportedFontModifier) {
		myFontModifier &= ~other.mySupportedFontModifier;
		myFontModifier |= (other.myFontModifier & other.mySupportedFontModifier);
		mySupportedFontModifier |= other.mySupportedFontModifier;
	}
	if (other.myMask & SUPPORT_FONT_SIZE) {
		setFontSizeMag(other.myFontSizeMag);
	}
	if (other.myMask & SUPPORT_FONT_FAMILIES) {
		setFontFamilies(other.myFontFamilies);
	}
	if (other.myMask & SUPPORT_COLOR) {
		setColor(other.myColor);
	}
}

void ZLTextStyleEntry::inherit(const ZLTextStyleEntry &other) {
	// text-indent
	if (other.myMask & (1 << LENGTH_FIRST_LINE_INDENT_DELTA)) {
		myLengths[LENGTH_FIRST_LINE_INDENT_DELTA] = other.myLengths[LENGTH_FIRST_LINE_INDENT_DELTA];
		myMask |= (1 << LENGTH_FIRST_LINE_INDENT_DELTA);
	}
	// text-align
	if (other.myMask & SUPPORT_ALIGNMENT_TYPE) {
		setAlignmentType(other.myAlignmentType);
	}
	// font-style, font-variant, font-weight
	if (other.mySupportedFontModifier) {
		myFontModifier &= ~other.mySupportedFontModifier;
		myFontModifier |= (other.myFontModifier & other.mySupportedFontModifier);
		mySupportedFontModifier |= other.mySupportedFontModifier;
	}
	// font-size
	if (other.myMask & SUPPORT_FONT_SIZE) {
		setFontSizeMag(other.myFontSizeMag);
	}
	// font-family
	if (other.myMask & SUPPORT_FONT_FAMILIES) {
		setFontFamilies(other.myFontFamilies);
	}
	// color
	if (other.myMask & SUPPORT_COLOR) {
		setColor(other.myColor);
	}
}

bool ZLTextStyleEntry::equals(const ZLTextStyleEntry &other) const {
	if (myMask == other.myMask && mySupportedFontModifier == other.mySupportedFontModifier) {
		if (myMask & ((1 << NUMBER_OF_LENGTHS) - 1)) {
			for (int i = 0; i < NUMBER_OF_LENGTHS; ++i) {
				if (myMask & (1 << i)) {
					if (myLengths[i].Size != other.myLengths[i].Size ||
					    myLengths[i].Unit != other.myLengths[i].Unit) {
						return false;
					}
				}
			}
		}
		if ((myMask & SUPPORT_OPACITY) && myOpacity != other.myOpacity) {
			return false;
		}
		if ((myMask & SUPPORT_ALIGNMENT_TYPE) && myAlignmentType != other.myAlignmentType) {
			return false;
		}
		if ((myFontModifier & mySupportedFontModifier) != (other.myFontModifier & mySupportedFontModifier)) {
			return false;
		}
		if ((myMask & SUPPORT_FONT_SIZE) && myFontSizeMag != other.myFontSizeMag) {
			return false;
		}
		if ((myMask & SUPPORT_FONT_FAMILIES) && myFontFamilies != other.myFontFamilies) {
			return false;
		}
		if ((myMask & SUPPORT_OPACITY) && myColor != other.myColor) {
			return false;
		}
		return true;
	} else {
		return false;
	}
}

bool ZLTextStyleEntry::parseLength(const std::string &toParse, short &size, SizeUnit &unit) {
	if (!toParse.empty()) {
		static const std::string PERCENT("%");
		static const std::string ZERO("0");
		static const std::string EM("em");
		static const std::string EX("ex");
		static const std::string PX("px");
		static const std::string PT("pt");
		static const std::string PC("pc");
		if (ZLStringUtil::stringEndsWith(toParse, PERCENT)) {
			unit = ZLTextStyleEntry::SIZE_UNIT_PERCENT;
			size = atoi(toParse.c_str());
			return true;
		} else if (ZLStringUtil::stringEndsWith(toParse, EM)) {
			unit = ZLTextStyleEntry::SIZE_UNIT_EM_100;
			size = (short)(100 * ZLStringUtil::stringToDouble(toParse, 0));
			return true;
		} else if (ZLStringUtil::stringEndsWith(toParse, EX)) {
			unit = ZLTextStyleEntry::SIZE_UNIT_EX_100;
			size = (short)(100 * ZLStringUtil::stringToDouble(toParse, 0));
			return true;
		} else if (ZLStringUtil::stringEndsWith(toParse, PX) ||
			   ZLStringUtil::stringEndsWith(toParse, PT) ||
			   ZLStringUtil::stringEndsWith(toParse, PC)) {
			unit = ZLTextStyleEntry::SIZE_UNIT_PIXEL;
			size = atoi(toParse.c_str());
			return true;
		} else if (toParse == ZERO) {
			unit = ZLTextStyleEntry::SIZE_UNIT_PIXEL;
			size = 0;
			return true;
		}
	}
	return false;
}

const shared_ptr<ZLTextParagraphEntry> ZLTextParagraph::Iterator::entry() const {
	if (myEntry.isNull()) {
		switch (*myPointer) {
			case ZLTextParagraphEntry::TEXT_ENTRY:
				myEntry = new ZLTextEntry(myPointer + 1);
				break;
			case ZLTextParagraphEntry::CONTROL_ENTRY:
			{
				unsigned char token = *(myPointer + 1);
				myEntry = ZLTextControlEntryPool::Pool.controlEntry((ZLTextKind)(token >> 1), (token & 1) == 1);
				break;
			}
			case ZLTextParagraphEntry::HYPERLINK_CONTROL_ENTRY:
				myEntry = new ZLTextHyperlinkControlEntry(myPointer + 1);
				break;
			case ZLTextParagraphEntry::IMAGE_ENTRY:
			{
				ZLImageMap *imageMap = 0;
				short vOffset = 0;
				memcpy(&imageMap, myPointer + 1, sizeof(const ZLImageMap*));
				memcpy(&vOffset, myPointer + 1 + sizeof(const ZLImageMap*), sizeof(short));
				myEntry = new ImageEntry(myPointer + sizeof(const ZLImageMap*) + sizeof(short) + 1, imageMap, vOffset);
				break;
			}
			case ZLTextParagraphEntry::STYLE_ENTRY:
				myEntry = new ZLTextStyleEntry(myPointer + 1);
				break;
			case ZLTextParagraphEntry::FIXED_HSPACE_ENTRY:
				myEntry = new ZLTextFixedHSpaceEntry((unsigned char)*(myPointer + 1));
				break;
			case ZLTextParagraphEntry::RESET_BIDI_ENTRY:
				myEntry = ResetBidiEntry::Instance;
				break;
		}
	}
	return myEntry;
}

void ZLTextParagraph::Iterator::next() {
	++myIndex;
	myEntry = 0;
	if (myIndex != myEndIndex) {
		switch (*myPointer) {
			case ZLTextParagraphEntry::TEXT_ENTRY:
			{
				size_t len;
				memcpy(&len, myPointer + 1, sizeof(size_t));
				myPointer += len + sizeof(size_t) + 1;
				break;
			}
			case ZLTextParagraphEntry::CONTROL_ENTRY:
				myPointer += 2;
				break;
			case ZLTextParagraphEntry::HYPERLINK_CONTROL_ENTRY:
				myPointer += 2;
				while (*myPointer != '\0') {
					++myPointer;
				}
				++myPointer;
				while (*myPointer != '\0') {
					++myPointer;
				}
				++myPointer;
				break;
			case ZLTextParagraphEntry::IMAGE_ENTRY:
				myPointer += sizeof(const ZLImageMap*) + sizeof(short) + 1;
				while (*myPointer != '\0') {
					++myPointer;
				}
				++myPointer;
				break;
			case ZLTextParagraphEntry::STYLE_ENTRY:
			{
				int mask;
				const unsigned char supportedFontModifier = myPointer[1];
				memcpy(&mask, myPointer + 2, sizeof(int));
				myPointer += sizeof(int) + 2;
				if (mask & ((1 << ZLTextStyleEntry::NUMBER_OF_LENGTHS) - 1)) {
					for (int i = 0; i < ZLTextStyleEntry::NUMBER_OF_LENGTHS; ++i) {
						if (mask & (1 << i)) {
							myPointer += (sizeof(short) + 1);
						}
					}
				}
				if (mask & ZLTextStyleEntry::SUPPORT_OPACITY) ++myPointer;
				if (mask & ZLTextStyleEntry::SUPPORT_ALIGNMENT_TYPE) ++myPointer;
				if (supportedFontModifier) ++myPointer;
				if (mask & ZLTextStyleEntry::SUPPORT_FONT_SIZE) ++myPointer;
				if (mask & ZLTextStyleEntry::SUPPORT_FONT_FAMILIES) {
					unsigned char n = *myPointer++;
					for (unsigned int i = 0; i < n; ++i) {
						while (*myPointer++);
					}
				}
				if (mask & ZLTextStyleEntry::SUPPORT_COLOR) myPointer += 3;
				break;
			}
			case ZLTextParagraphEntry::FIXED_HSPACE_ENTRY:
				myPointer += 2;
				break;
			case ZLTextParagraphEntry::RESET_BIDI_ENTRY:
			case ZLTextParagraphEntry::LINE_BREAK_ENTRY:
			case ZLTextParagraphEntry::EMPTY_ENTRY:
				++myPointer;
				break;
		}
		if (*myPointer == 0) {
			memcpy(&myPointer, myPointer + 1, sizeof(char*));
		}
	}
}

ZLTextControlEntryPool ZLTextControlEntryPool::Pool;

shared_ptr<ZLTextParagraphEntry> ZLTextControlEntryPool::controlEntry(ZLTextKind kind, bool isStart) {
	std::map<ZLTextKind, shared_ptr<ZLTextParagraphEntry> > &entries = isStart ? myStartEntries : myEndEntries;
	std::map<ZLTextKind, shared_ptr<ZLTextParagraphEntry> >::iterator it = entries.find(kind);
	if (it != entries.end()) {
		return it->second;
	}
	shared_ptr<ZLTextParagraphEntry> entry = new ZLTextControlEntry(kind, isStart);
	entries[kind] = entry;
	return entry;
}
	
size_t ZLTextParagraph::textDataLength() const {
	size_t len = 0;
	for (Iterator it = *this; !it.isEnd(); it.next()) {
		if (it.entryKind() == ZLTextParagraphEntry::TEXT_ENTRY) {
			len += ((ZLTextEntry&)*it.entry()).dataLength();
		}
	}
	return len;
}

size_t ZLTextParagraph::characterNumber() const {
	size_t len = 0;
	for (Iterator it = *this; !it.isEnd(); it.next()) {
		switch (it.entryKind()) {
			case ZLTextParagraphEntry::TEXT_ENTRY:
			{
				const ZLTextEntry &entry = (ZLTextEntry&)*it.entry();
				len += ZLUnicodeUtil::utf8Length(entry.data(), entry.dataLength());
				break;
			}
			case ZLTextParagraphEntry::IMAGE_ENTRY:
				len += 100;
				break;
			default:
				break;
		}
	}
	return len;
}

shared_ptr<const ZLImage> ImageEntry::image() const {
	ZLImageMap::const_iterator it = myMap->find(myId);
	return (it != myMap->end()) ? (*it).second : 0;
}

ZLTextTreeParagraph::ZLTextTreeParagraph(ZLTextTreeParagraph *parent) : myIsOpen(false), myParent(parent) {
	if (parent != 0) {
		parent->addChild(this);
		myDepth = parent->myDepth + 1;
	} else {
		myDepth = 0;
	}
}

void ZLTextTreeParagraph::openTree() {
	for (ZLTextTreeParagraph *p = parent(); p != 0; p = p->parent()) {
		p->open(true);
	}
}

void ZLTextTreeParagraph::removeFromParent() {
	if (myParent != 0) {
		myParent->myChildren.erase(std::find(myParent->myChildren.begin(), myParent->myChildren.end(), this));
	}
}

int ZLTextTreeParagraph::fullSize() const {
	int size = 1;
	for (std::vector<ZLTextTreeParagraph*>::const_iterator it = myChildren.begin(); it != myChildren.end(); ++it) {
		size += (*it)->fullSize();
	}
	return size;
}

ResetBidiEntry::ResetBidiEntry() {
}
