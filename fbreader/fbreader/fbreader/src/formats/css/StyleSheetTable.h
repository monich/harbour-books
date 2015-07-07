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

#ifndef __STYLESHEETTABLE_H__
#define __STYLESHEETTABLE_H__

#include <string>
#include <map>
#include <vector>

#include <shared_ptr.h>

#include <ZLTextParagraph.h>
#include <ZLBoolean3.h>

class StyleSheetTable {

public:
	struct Element {
		Element(const std::string &tag, const char *klass, const char* id);
		std::string Name;
		std::vector<std::string> Classes;
		std::string Id;
	};

	class Selector {
	public:
		Selector(const std::string &type, const std::string &klass, const std::string &id);
		Selector(const std::string &type, const std::string &klass);
		Selector(const std::string &selector);
		Selector(const Selector &other);
		Selector();

		Selector &operator = (const Selector &other);
		bool operator == (const Selector &other) const;
		bool operator != (const Selector &other) const;
		bool operator < (const Selector &other) const;
		bool match(const Element &element) const;

		static bool match(const std::string &pattern, const std::string &str);
		static bool match(const std::string &pattern, const std::vector<std::string> &strings);

		const std::string &type() const;
		const std::string &klass() const;
		const std::string &id() const;
		int a() const;
		int b() const;
		int c() const;

	private:
		std::string myType;
		std::string myClass;
		std::string myId;
	};

	typedef std::vector<Element> ElementList;
	typedef std::vector<Selector> SelectorList;
	typedef std::map<std::string,std::vector<std::string> > AttributeMap;

	struct Style {
		Style();
		Style(const Style &other);
		Style(const AttributeMap &map);

		Style &operator = (const Style &other);
		void apply(const Style &other);

		ZLTextStyleEntry TextStyle;
		ZLBoolean3 PageBreakBefore;
		ZLBoolean3 PageBreakAfter;
	};

	typedef std::vector<Style> StyleList;
	static void updateStyle(Style &style, const AttributeMap &map);
	static void updateTextStyle(ZLTextStyleEntry &entry, const AttributeMap &map);
	static bool getPageBreakBefore(const AttributeMap &map, ZLBoolean3 &value);
	static bool getPageBreakAfter(const AttributeMap &map, ZLBoolean3 &value);

private:
	struct Entry {
		Entry();
		Entry(const SelectorList &selectors, int specificity, const AttributeMap &map);
		Entry &operator = (const Entry &other);
		bool match(const ElementList &stack) const;

		SelectorList Selectors;
		int Specificity;
		struct Style Style;
	};

	void addMap(const std::vector<std::string> &selectors, const AttributeMap &map);

	static bool parseLength(const std::string &toParse, short &size, ZLTextStyleEntry::SizeUnit &unit);
	static bool parseMargin(const std::string &toParse, short &size, ZLTextStyleEntry::SizeUnit &unit);
	static void setLength(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const AttributeMap &map, const std::string &attributeName);
	static void setLength(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const std::string &value);
	static void setMargin(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const AttributeMap &map, const std::string &attributeName);
	static void setMargin(ZLTextStyleEntry &entry, ZLTextStyleEntry::Length name, const std::string &value);
	static const std::vector<std::string> &values(const AttributeMap &map, const std::string &name);
	static bool sortBySpecificity(const Entry *e1, const Entry *e2);

public:
	bool isEmpty() const;
	void applyStyles(const ElementList &stack, Style &style) const;

private:
	std::vector<Entry> myEntries;

friend class StyleSheetTableParser;
};

inline StyleSheetTable::Selector::Selector(const std::string &type, const std::string &klass, const std::string &id) : myType(type), myClass(klass), myId(id) {}
inline StyleSheetTable::Selector::Selector(const std::string &type, const std::string &klass) : myType(type), myClass(klass) {}
inline StyleSheetTable::Selector::Selector(const StyleSheetTable::Selector &other) : myType(other.myType), myClass(other.myClass), myId(other.myId) {}
inline StyleSheetTable::Selector::Selector() {}
inline StyleSheetTable::Selector &StyleSheetTable::Selector::operator = (const StyleSheetTable::Selector &other) {
	myType = other.myType; myClass = other.myClass; myId = other.myId; return *this;
}
inline bool StyleSheetTable::Selector::operator == (const Selector &other) const {
	return (&other == this) || (myType == other.myType && myClass == other.myClass && myId == other.myId);
}
inline bool StyleSheetTable::Selector::operator != (const Selector &other) const {
	return (&other != this) && (myType != other.myType || myClass != other.myClass || myId != other.myId);
}
inline bool StyleSheetTable::Selector::operator < (const StyleSheetTable::Selector &other) const {
	return myType < other.myType || (myType == other.myType && (myClass < other.myClass || (myClass == other.myClass && myId < other.myId)));
}
inline bool StyleSheetTable::Selector::match(const Element &element) const {
	return match(myType, element.Name) && match(myId, element.Id) && match(myClass, element.Classes);
}
inline int StyleSheetTable::Selector::a() const { return myId.empty() ? 0 : 1; }
inline int StyleSheetTable::Selector::b() const { return myClass.empty() ? 0 : 1; }
inline int StyleSheetTable::Selector::c() const { return myType.empty() ? 0 : 1; }
inline const std::string &StyleSheetTable::Selector::type() const { return myType; }
inline const std::string &StyleSheetTable::Selector::klass() const { return myClass; }
inline const std::string &StyleSheetTable::Selector::id() const { return myId; }

inline StyleSheetTable::Style::Style() : PageBreakBefore(B3_UNDEFINED), PageBreakAfter(B3_UNDEFINED) {}
inline StyleSheetTable::Style::Style(const StyleSheetTable::Style &other) : TextStyle(other.TextStyle), PageBreakBefore(other.PageBreakBefore), PageBreakAfter(other.PageBreakAfter) {}
inline StyleSheetTable::Style::Style(const StyleSheetTable::AttributeMap &map) : PageBreakBefore(B3_UNDEFINED), PageBreakAfter(B3_UNDEFINED) {
	updateStyle(*this, map);
}
inline StyleSheetTable::Style &StyleSheetTable::Style::operator = (const StyleSheetTable::Style &other) {
	TextStyle = other.TextStyle;
	PageBreakBefore = other.PageBreakBefore;
	PageBreakAfter = other.PageBreakAfter;
	return *this;
}

inline StyleSheetTable::Entry::Entry() : Specificity(0) {}
inline StyleSheetTable::Entry::Entry(const SelectorList &selectors, int specificity, const AttributeMap &map) : Selectors(selectors), Specificity(specificity), Style(map) {}
inline StyleSheetTable::Entry &StyleSheetTable::Entry::operator = (const StyleSheetTable::Entry &other) {
	Selectors = other.Selectors;
	Style = other.Style;
	Specificity = other.Specificity;
	return *this;
}

inline bool StyleSheetTable::isEmpty() const { return myEntries.empty(); }

#endif /* __STYLESHEETTABLE_H__ */
