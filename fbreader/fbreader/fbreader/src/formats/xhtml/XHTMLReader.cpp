/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
 * Copyright (C) 2016-2017 Slava Monich <slava.monich@jolla.com>
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

#include <cstring>

#include <ZLFile.h>
#include <ZLFileUtil.h>
#include <ZLFileImage.h>
#include <ZLUnicodeUtil.h>
#include <ZLStringUtil.h>
#include <ZLXMLNamespace.h>
#include <ZLTextStyleCollection.h>

#include "XHTMLReader.h"
#include "../util/EntityFilesCollector.h"
#include "../util/MiscUtil.h"
#include "../css/StyleSheetParser.h"

#include "../../bookmodel/BookReader.h"
#include "../../bookmodel/BookModel.h"

static const bool USE_CSS = false;

std::map<std::string,XHTMLTagAction*> XHTMLReader::ourTagActions;

XHTMLTagAction::~XHTMLTagAction() {
}

void XHTMLTagAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
}

void XHTMLTagAction::doAtEnd(XHTMLReader &reader) {
}

BookReader &XHTMLTagAction::bookReader(XHTMLReader &reader) {
	return reader.myModelReader;
}

const std::string &XHTMLTagAction::pathPrefix(XHTMLReader &reader) {
	return reader.myPathPrefix;
}

void XHTMLTagAction::beginParagraph(XHTMLReader &reader) {
	reader.beginParagraph();
}

void XHTMLTagAction::endParagraph(XHTMLReader &reader) {
	reader.endParagraph();
}

class XHTMLTagStyleAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagLinkAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
};

class XHTMLTagParagraphAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagBodyAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagLineBreakAction : public XHTMLTagAction {

public:
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagImageAction : public XHTMLTagAction {

public:
	XHTMLTagImageAction(shared_ptr<ZLXMLReader::AttributeNamePredicate> predicate);
	XHTMLTagImageAction(const std::string &attributeName);

	void doAtStart(XHTMLReader &reader, const char **xmlattributes);

private:
	shared_ptr<ZLXMLReader::AttributeNamePredicate> myPredicate;
};

class XHTMLSvgImageAttributeNamePredicate : public ZLXMLReader::NamespaceAttributeNamePredicate {

public:
	XHTMLSvgImageAttributeNamePredicate();
	bool accepts(const ZLXMLReader &reader, const char *name) const;

private:
	bool myIsEnabled;

friend class XHTMLTagSvgAction;
};

class XHTMLTagSvgAction : public XHTMLTagAction {

public:
	XHTMLTagSvgAction(XHTMLSvgImageAttributeNamePredicate &predicate);
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);

private:
	XHTMLSvgImageAttributeNamePredicate &myPredicate;
};

class XHTMLTagItemAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagHyperlinkAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);

private:
	std::stack<FBTextKind> myHyperlinkStack;
};

class XHTMLTagControlAction : public XHTMLTagAction {

public:
	XHTMLTagControlAction(FBTextKind control);

	void doAtStart(XHTMLReader &reader, const char **xmlattributes);

private:
	FBTextKind myControl;
};

class XHTMLTagParagraphWithControlAction : public XHTMLTagAction {

public:
	XHTMLTagParagraphWithControlAction(FBTextKind control);

	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);

private:
	FBTextKind myControl;
};

class XHTMLTagFootnoteAction : public XHTMLTagAction {

public:
	XHTMLTagFootnoteAction();
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

class XHTMLTagPreAction : public XHTMLTagAction {

public:
	void doAtStart(XHTMLReader &reader, const char **xmlattributes);
	void doAtEnd(XHTMLReader &reader);
};

void XHTMLTagStyleAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
	const char *type = reader.attributeValue(xmlattributes, "type");
	if (!type || strcmp(type, "text/css")) {
		return;
	}

	if (reader.myReadState == XHTMLReader::READ_NOTHING) {
		reader.myReadState = XHTMLReader::READ_STYLE;
		reader.myTableParser = new StyleSheetTableParser(reader.myStyleSheetTable);
	}
}

void XHTMLTagStyleAction::doAtEnd(XHTMLReader &reader) {
	if (reader.myReadState == XHTMLReader::READ_STYLE) {
		reader.myReadState = XHTMLReader::READ_NOTHING;
		reader.myTableParser.reset();
	}
}

void XHTMLTagLinkAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
	const char *rel = reader.attributeValue(xmlattributes, "rel");
	if (!rel || strcmp(rel, "stylesheet")) {
		return;
	}

	const char *type = reader.attributeValue(xmlattributes, "type");
	if (!type || strcmp(type, "text/css")) {
		return;
	}

	const char *href = reader.attributeValue(xmlattributes, "href");
	if (href == 0) {
		return;
	}

	shared_ptr<ZLInputStream> cssStream = ZLFile(reader.myPathPrefix + MiscUtil::decodeHtmlURL(href)).inputStream();
	if (cssStream.isNull()) {
		return;
	}
	StyleSheetTableParser parser(reader.myStyleSheetTable);
	parser.parse(*cssStream);
	//reader.myStyleSheetTable.dump();
}

void XHTMLTagParagraphAction::doAtStart(XHTMLReader &reader, const char**) {
	endParagraph(reader);
}

void XHTMLTagParagraphAction::doAtEnd(XHTMLReader &reader) {
	endParagraph(reader);
}

void XHTMLTagBodyAction::doAtStart(XHTMLReader &reader, const char**) {
	reader.myReadState = XHTMLReader::READ_BODY;
}

void XHTMLTagBodyAction::doAtEnd(XHTMLReader &reader) {
	endParagraph(reader);
	reader.myReadState = XHTMLReader::READ_NOTHING;
}

void XHTMLTagLineBreakAction::doAtEnd(XHTMLReader& reader) {
	BookReader &br = bookReader(reader);
	if (br.paragraphIsOpen()) {
		br.addLineBreak();
	} else {
		beginParagraph(reader);
		br.addLineBreak();
		endParagraph(reader);
	}
}

void XHTMLTagItemAction::doAtStart(XHTMLReader &reader, const char**) {
	endParagraph(reader);
	// TODO: increase left indent
	beginParagraph(reader);
	// TODO: replace bullet sign by number inside OL tag
	const std::string bullet = "\xE2\x80\xA2\xC0\xA0";
	bookReader(reader).addData(bullet);
}

void XHTMLTagItemAction::doAtEnd(XHTMLReader &reader) {
	endParagraph(reader);
}

XHTMLTagImageAction::XHTMLTagImageAction(shared_ptr<ZLXMLReader::AttributeNamePredicate> predicate) {
	myPredicate = predicate;
}

XHTMLTagImageAction::XHTMLTagImageAction(const std::string &attributeName) {
	myPredicate = new ZLXMLReader::FixedAttributeNamePredicate(attributeName);
}

void XHTMLTagImageAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
	// Ignore transparent and hidden images
	if (!reader.myParseStack.back().opacity ||
	   (!reader.myStyleStack.empty() && reader.myStyleStack.back().DisplayNone)) {
		return;
	}

	const char *fileName = reader.attributeValue(xmlattributes, *myPredicate);
	if (fileName == 0) {
		return;
	}

	const std::string fullfileName = pathPrefix(reader) + MiscUtil::decodeHtmlURL(fileName);
	if (!ZLFile(fullfileName).exists()) {
		return;
	}

	if ((strlen(fileName) > 2) && strncmp(fileName, "./", 2) == 0) {
		fileName +=2;
	}

	reader.myParseStack.back().kind = IMAGE;
	reader.haveContent();
	reader.myModelReader.addImageReference(fullfileName);
	reader.myModelReader.addImage(fullfileName, new ZLFileImage(ZLFile(fullfileName), 0));
}

XHTMLTagSvgAction::XHTMLTagSvgAction(XHTMLSvgImageAttributeNamePredicate &predicate) : myPredicate(predicate) {
}

void XHTMLTagSvgAction::doAtStart(XHTMLReader&, const char**) {
	myPredicate.myIsEnabled = true;
}

void XHTMLTagSvgAction::doAtEnd(XHTMLReader&) {
	myPredicate.myIsEnabled = false;
}

XHTMLSvgImageAttributeNamePredicate::XHTMLSvgImageAttributeNamePredicate() : ZLXMLReader::NamespaceAttributeNamePredicate(ZLXMLNamespace::XLink, "href"), myIsEnabled(false) {
}

bool XHTMLSvgImageAttributeNamePredicate::accepts(const ZLXMLReader &reader, const char *name) const {
	return myIsEnabled && NamespaceAttributeNamePredicate::accepts(reader, name);
}

XHTMLTagControlAction::XHTMLTagControlAction(FBTextKind control) : myControl(control) {
}

void XHTMLTagControlAction::doAtStart(XHTMLReader &reader, const char**) {
	reader.myParseStack.back().kind = myControl;
}

void XHTMLTagHyperlinkAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
	const char *href = reader.attributeValue(xmlattributes, "href");
	if (href != 0 && href[0] != '\0') {
		FBTextKind hyperlinkType;
		const char *type = reader.attributeValue(xmlattributes, "epub:type");
		std::string link;
		if (type && !strcmp(type, "noteref")) {
			hyperlinkType = FOOTNOTE;
			link = href;
		} else {
			hyperlinkType = MiscUtil::referenceType(href);
			link = MiscUtil::decodeHtmlURL(href);
			if (hyperlinkType == INTERNAL_HYPERLINK) {
				link = (link[0] == '#') ?
					reader.myReferenceName + link :
					reader.myReferenceDirName + link;
				link = ZLFileUtil::normalizeUnixPath(link);
			}
		}
		myHyperlinkStack.push(hyperlinkType);
		bookReader(reader).addHyperlinkControl(hyperlinkType, link);
	} else {
		myHyperlinkStack.push(REGULAR);
	}
	const char *name = reader.attributeValue(xmlattributes, "name");
	if (name != 0) {
		bookReader(reader).addHyperlinkLabel(
			reader.myReferenceName + "#" + MiscUtil::decodeHtmlURL(name)
		);
	}
}

void XHTMLTagHyperlinkAction::doAtEnd(XHTMLReader &reader) {
	FBTextKind kind = myHyperlinkStack.top();
	if (kind != REGULAR) {
		bookReader(reader).addControl(kind, false);
	}
	myHyperlinkStack.pop();
}

XHTMLTagParagraphWithControlAction::XHTMLTagParagraphWithControlAction(FBTextKind control) : myControl(control) {
}

void XHTMLTagParagraphWithControlAction::doAtStart(XHTMLReader &reader, const char**) {
	if ((myControl == TITLE) && (bookReader(reader).model().bookTextModel()->paragraphsNumber() > 1)) {
		bookReader(reader).insertEndOfSectionParagraph();
	}
	reader.myParseStack.back().kind = myControl;
}

void XHTMLTagParagraphWithControlAction::doAtEnd(XHTMLReader &reader) {
	endParagraph(reader);
}

XHTMLTagFootnoteAction::XHTMLTagFootnoteAction() {
}

void XHTMLTagFootnoteAction::doAtStart(XHTMLReader &reader, const char **xmlattributes) {
	const char *id = reader.attributeValue(xmlattributes, "id");
	if (id) {
		bookReader(reader).setFootnoteTextModel(id);
	}
}

void XHTMLTagFootnoteAction::doAtEnd(XHTMLReader &reader) {
	bookReader(reader).setMainTextModel();
}

void XHTMLTagPreAction::doAtStart(XHTMLReader &reader, const char**) {
	endParagraph(reader);
	reader.myParseStack.back().kind = PREFORMATTED;
	if (++reader.myPreformatted == 1) {
		beginParagraph(reader);
	}
}

void XHTMLTagPreAction::doAtEnd(XHTMLReader &reader) {
	if (!--reader.myPreformatted) {
		endParagraph(reader);
	}
}

XHTMLTagAction *XHTMLReader::addAction(const std::string &tag, XHTMLTagAction *action) {
	XHTMLTagAction *old = ourTagActions[tag];
	ourTagActions[tag] = action;
	return old;
}

void XHTMLReader::fillTagTable() {
	if (ourTagActions.empty()) {
		//addAction("html",	new XHTMLTagAction());
		addAction("body",	new XHTMLTagBodyAction());
		//addAction("title",	new XHTMLTagAction());
		//addAction("meta",	new XHTMLTagAction());
		//addAction("script",	new XHTMLTagAction());

		//addAction("font",	new XHTMLTagAction());
		addAction("style",	new XHTMLTagStyleAction());

		addAction("p",	new XHTMLTagParagraphAction());
		addAction("h1",	new XHTMLTagParagraphWithControlAction(H1));
		addAction("h2",	new XHTMLTagParagraphWithControlAction(H2));
		addAction("h3",	new XHTMLTagParagraphWithControlAction(H3));
		addAction("h4",	new XHTMLTagParagraphWithControlAction(H4));
		addAction("h5",	new XHTMLTagParagraphWithControlAction(H5));
		addAction("h6",	new XHTMLTagParagraphWithControlAction(H6));

		//addAction("ol",	new XHTMLTagAction());
		//addAction("ul",	new XHTMLTagAction());
		//addAction("dl",	new XHTMLTagAction());
		addAction("li",	new XHTMLTagItemAction());

		addAction("strong",	new XHTMLTagControlAction(STRONG));
		addAction("b",	new XHTMLTagControlAction(BOLD));
		addAction("em",	new XHTMLTagControlAction(EMPHASIS));
		addAction("i",	new XHTMLTagControlAction(ITALIC));
		addAction("code",	new XHTMLTagControlAction(CODE));
		addAction("tt",	new XHTMLTagControlAction(CODE));
		addAction("kbd",	new XHTMLTagControlAction(CODE));
		addAction("var",	new XHTMLTagControlAction(CODE));
		addAction("samp",	new XHTMLTagControlAction(CODE));
		addAction("cite",	new XHTMLTagControlAction(CITE));
		addAction("sub",	new XHTMLTagControlAction(SUB));
		addAction("sup",	new XHTMLTagControlAction(SUP));
		addAction("dd",	new XHTMLTagParagraphWithControlAction(DEFINITION_DESCRIPTION));
		addAction("dt",	new XHTMLTagParagraphWithControlAction(DEFINITION));
		addAction("dfn",	new XHTMLTagParagraphWithControlAction(DEFINITION));
		addAction("strike",	new XHTMLTagControlAction(STRIKETHROUGH));
		addAction("blockquote",	new XHTMLTagParagraphWithControlAction(BLOCKQUOTE));
		addAction("aside", new XHTMLTagFootnoteAction());

		addAction("a",	new XHTMLTagHyperlinkAction());

		addAction("img",	new XHTMLTagImageAction("src"));
		addAction("object",	new XHTMLTagImageAction("data"));
		XHTMLSvgImageAttributeNamePredicate *predicate = new XHTMLSvgImageAttributeNamePredicate();
		addAction("image",	new XHTMLTagImageAction(predicate));
		addAction("svg",	new XHTMLTagSvgAction(*predicate));

		//addAction("area",	new XHTMLTagAction());
		//addAction("map",	new XHTMLTagAction());

		//addAction("base",	new XHTMLTagAction());
		addAction("br",	new XHTMLTagLineBreakAction());
		//addAction("center",	new XHTMLTagAction());
		addAction("div", new XHTMLTagParagraphAction());
		addAction("dt", new XHTMLTagParagraphAction());
		//addAction("head",	new XHTMLTagAction());
		//addAction("hr",	new XHTMLTagAction());
		addAction("link",	new XHTMLTagLinkAction());
		//addAction("param",	new XHTMLTagAction());
		//addAction("q",	new XHTMLTagAction());
		//addAction("s",	new XHTMLTagAction());

		addAction("pre",	new XHTMLTagPreAction());
		//addAction("big",	new XHTMLTagAction());
		//addAction("small",	new XHTMLTagAction());
		//addAction("u",	new XHTMLTagAction());

		//addAction("table",	new XHTMLTagAction());
		addAction("td",	new XHTMLTagParagraphAction());
		addAction("th",	new XHTMLTagParagraphAction());
		//addAction("tr",	new XHTMLTagAction());
		//addAction("caption",	new XHTMLTagAction());
		//addAction("span",	new XHTMLTagAction());
	}
}

XHTMLReader::XHTMLReader(BookReader &modelReader) : myModelReader(modelReader) {
}

bool XHTMLReader::readFile(const ZLFile &file, const std::string &referenceName) {
	myModelReader.addHyperlinkLabel(referenceName);

	fillTagTable();

	myPathPrefix = MiscUtil::htmlDirectoryPrefix(file.path());
	myReferenceName = referenceName;
	const int index = referenceName.rfind('/', referenceName.length() - 1);
	myReferenceDirName = referenceName.substr(0, index + 1);

	myPreformatted = 0;
	myReadState = READ_NOTHING;

	myElementStack.clear();
	myStyleStack.clear();
	myParseStack.resize(1);

	return readDocument(file);
}

void XHTMLReader::startElementHandler(const char *tag, const char **attributes) {
	static const std::string HASH = "#";
	const char *id = attributeValue(attributes, "id");
	const char *inlineStyle = attributeValue(attributes, "style");
	const char *klass = attributeValue(attributes, "class");
	if (id != 0) {
		myModelReader.addHyperlinkLabel(myReferenceName + HASH + id);
	}

	const std::string sTag = ZLUnicodeUtil::toLower(tag);
	myElementStack.push_back(StyleSheetTable::Element(sTag, klass, id));

	myStyleStack.resize(myStyleStack.size() + 1);
	StyleSheetTable::Style *style = &myStyleStack.back();
	if (myStyleStack.size() > 1) {
		style->inherit(myStyleStack.at(myStyleStack.size()-2));
	}

	myStyleSheetTable.applyStyles(myElementStack, *style);
	if (inlineStyle) {
		style->apply(myStyleParser.parseString(inlineStyle));
	}

	myParseStack.resize(myParseStack.size() + 1);
	ParseContext &prev(myParseStack.at(myParseStack.size()-2));
	ParseContext &context(myParseStack.back());
	if (style->TextStyle.opacitySupported()) {
		int opacity = prev.opacity;
		opacity *= style->TextStyle.opacity();
		opacity /= 255;
		context.opacity = opacity;
	} else {
		context.opacity = prev.opacity;
	}

	// Don't collect empty styles
	if (style->empty()) {
		myStyleStack.resize(myStyleStack.size()-1);
		style = NULL;
	} else {
		context.styleIndex = myStyleStack.size() - 1;
		if (style->PageBreakBefore == B3_TRUE) {
			addPageBreak();
		}
	}

	XHTMLTagAction *action = ourTagActions[sTag];
	if (action != 0) {
		action->doAtStart(*this, attributes);
	}

	if (context.kind >= 0) {
		context.decoration = ZLTextStyleCollection::Instance().decoration(context.kind);
	}

	if (myModelReader.paragraphIsOpen()) {
		applyStyles(myParseStack.back());
	}
}

void XHTMLReader::endElementHandler(const char *tag) {
	bool pageBreak = false;
	ParseContext &context(myParseStack.back());
	if (context.styleIndex >= 0) {
		if (myStyleStack[context.styleIndex].PageBreakAfter == B3_TRUE) {
			// If we are about to have a page break, we don't want
			// endParagraph() to apply pending bottom margins.
			myBottomMargins.resize(0);
			pageBreak = true;
		}
	}

	XHTMLTagAction *action = ourTagActions[ZLUnicodeUtil::toLower(tag)];
	if (action != 0) {
		action->doAtEnd(*this);
	}

	if (pageBreak) {
		addPageBreak();
	}

	if (myModelReader.paragraphIsOpen()) {
		if (context.styleIndex >= 0) {
			myModelReader.addControl(REGULAR, false);
		}
		if (context.kind >= 0) {
			myModelReader.addControl((FBTextKind)context.kind, false);
		}
	}

	if (!context.bottomMarginApplied && elementHasBottomMargin(context)) {
		ZLTextStyleEntry::SizeUnit unit = ZLTextStyleEntry::SIZE_UNIT_PIXEL;
		short size = 0;
		if (context.styleIndex >= 0 && myStyleStack[context.styleIndex].TextStyle.lengthSupported(ZLTextStyleEntry::LENGTH_SPACE_AFTER)) {
			size = myStyleStack[context.styleIndex].TextStyle.length(ZLTextStyleEntry::LENGTH_SPACE_AFTER, unit);
		} else if (context.decoration) {
			const ZLTextFullStyleDecoration *decoration = context.decoration->fullDecoration();
			if (decoration) {
				size = decoration->SpaceAfterOption.value();
				unit = decoration->SpaceAfterOptionUnit;
			}
		}
		if (size > 0) {
			addBottomMargin(size, unit);
		}
	}

	if (!myModelReader.paragraphIsOpen()) {
		applyBottomMargins();
	}

	if (context.styleIndex >= 0) {
		myStyleStack.pop_back();
	}
	myElementStack.pop_back();
	myParseStack.pop_back();
}

void XHTMLReader::addPageBreak() {
	myBottomMargins.resize(0);
	endParagraph();
	myModelReader.insertEndOfSectionParagraph();
}

void XHTMLReader::addBottomMargin(short size, ZLTextStyleEntry::SizeUnit unit) {
	for (std::vector<ZLTextStyleEntry>::iterator it = myBottomMargins.begin(); it != myBottomMargins.end(); ++it) {
		ZLTextStyleEntry::SizeUnit entryUnit;
		short entrySize = it->length(ZLTextStyleEntry::LENGTH_SPACE_AFTER, entryUnit);
		if (entryUnit == unit) {
			it->setLength(ZLTextStyleEntry::LENGTH_SPACE_AFTER, entrySize + size, unit);
			return;
		}
	}
	// No such unit yet
	myBottomMargins.resize(myBottomMargins.size()+1);
	myBottomMargins.back().setLength(ZLTextStyleEntry::LENGTH_SPACE_AFTER, size, unit);
}

void XHTMLReader::applyBottomMargins() {
	if (!myBottomMargins.empty()) {
		myModelReader.endParagraph();
		for (std::vector<ZLTextStyleEntry>::const_iterator it = myBottomMargins.begin(); it != myBottomMargins.end(); ++it) {
			addStyleParagraph(*it);
		}
		myBottomMargins.resize(0);
	}
}

bool XHTMLReader::elementHasTopMargin(const ParseContext &context) const {
	return
		(context.styleIndex >= 0 && myStyleStack[context.styleIndex].TextStyle.lengthSupported(ZLTextStyleEntry::LENGTH_SPACE_BEFORE)) ||
		(context.decoration && context.decoration->fullDecoration());
}

bool XHTMLReader::elementHasBottomMargin(const ParseContext &context) const {
	return
		(context.styleIndex >= 0 && myStyleStack[context.styleIndex].TextStyle.lengthSupported(ZLTextStyleEntry::LENGTH_SPACE_AFTER)) ||
		(context.decoration && context.decoration->fullDecoration());
}

void XHTMLReader::addStyleParagraph(const ZLTextStyleEntry &style) {
	myModelReader.beginParagraph();
	myModelReader.addControl(style);
	myModelReader.addEmpty();
	myModelReader.endParagraph();
}

void XHTMLReader::applyStyles(ParseContext &context) {
	if (!context.stylesApplied) {
		context.stylesApplied = true;
		if (context.kind >= 0) {
			myModelReader.addControl((FBTextKind)context.kind, true);
		}
		if (context.styleIndex >= 0) {
			myModelReader.addControl(myStyleStack[context.styleIndex].TextStyle);
		}
	}
}

void XHTMLReader::beginParagraph() {
	if (!myModelReader.paragraphIsOpen()) {
		myModelReader.beginParagraph();
		for (std::vector<ParseContext>::iterator it = myParseStack.begin(); it != myParseStack.end(); ++it) {
			applyStyles(*it);
		}
	}
}

void XHTMLReader::endParagraph() {
	if (myModelReader.paragraphIsOpen()) {
		myModelReader.endParagraph();

		// Find which bottom margins have been applied
		// and at the same time reset stylesApplied flag.
		std::vector<ParseContext>::reverse_iterator it = myParseStack.rbegin();
		for (; it != myParseStack.rend(); ++it) {
			it->stylesApplied = false;
			if (elementHasBottomMargin(*it)) {
				it->bottomMarginApplied = true;
				break;
			}
		}

		// Reset stylesApplied for the remaining entries
		for (; it != myParseStack.rend(); ++it) {
			it->stylesApplied = false;
		}

		// Apply pending bottom margins
		applyBottomMargins();
	}
}

void XHTMLReader::haveContent() {
	if (!myParseStack.back().haveContent) {
		// Create empty paragraphs for other parent entries that haven't
		// had any content yet, in order to apply their top margins. Skip
		// the last entry as it will be applied for the paragraph we are
		// about to start.
		bool skippedLastEntry = false;
		for (std::vector<ParseContext>::reverse_iterator it = myParseStack.rbegin(); it != myParseStack.rend() && !it->haveContent; ++it) {
			it->haveContent = true;
			if (elementHasTopMargin(*it)) {
				if (!skippedLastEntry) {
					skippedLastEntry = true;
				} else {
					ZLTextStyleEntry::SizeUnit unit = ZLTextStyleEntry::SIZE_UNIT_PIXEL;
					short size = 0;
					if (it->styleIndex >= 0 && myStyleStack[it->styleIndex].TextStyle.lengthSupported(ZLTextStyleEntry::LENGTH_SPACE_BEFORE)) {
						size = myStyleStack[it->styleIndex].TextStyle.length(ZLTextStyleEntry::LENGTH_SPACE_BEFORE, unit);
					} else if (it->decoration) {
						const ZLTextFullStyleDecoration *decoration = it->decoration->fullDecoration();
						if (decoration) {
							size = decoration->SpaceBeforeOption.value();
							unit = decoration->SpaceBeforeOptionUnit;
						}
					}
					if (size > 0) {
						ZLTextStyleEntry style;
						style.setLength(ZLTextStyleEntry::LENGTH_SPACE_BEFORE, size, unit);
						addStyleParagraph(style);
					}
				}
			}
		}
	}
	myBottomMargins.resize(0);
	beginParagraph();
}

void XHTMLReader::characterDataHandler(const char *text, size_t len) {
	switch (myReadState) {
		case READ_NOTHING:
			break;
		case READ_STYLE:
			if (!myTableParser.isNull()) {
				myTableParser->parse(text, len);
			}
			break;
		case READ_BODY:
			if (myParseStack.back().opacity && (myStyleStack.empty() || !myStyleStack.back().DisplayNone)) {
				const StyleSheetTable::WhiteSpaceValue whiteSpace = myStyleStack.empty() ?
					StyleSheetTable::WS_UNDEFINED : myStyleStack.back().WhiteSpace;
				if (myPreformatted || whiteSpace == StyleSheetTable::WS_PRE || whiteSpace == StyleSheetTable::WS_PRE_WRAP) {
					size_t spaceCounter = 0;
					while (len > 0 && isspace(*text)) {
						if (*text == '\n') {
							haveContent();
							if (spaceCounter) {
								myModelReader.addFixedHSpace(spaceCounter);
								spaceCounter = 0;
							}
							myModelReader.addLineBreak();
						} else if (*text != '\r') {
							spaceCounter++;
						}
						++text;
						--len;
					}
					if (spaceCounter) {
						haveContent();
						myModelReader.addFixedHSpace(spaceCounter);
					}
				} else if (!myModelReader.paragraphIsOpen()) {
					while (len > 0 && isspace(*text)) {
						switch (whiteSpace) {
						case StyleSheetTable::WS_PRE:
						case StyleSheetTable::WS_PRE_WRAP:
						case StyleSheetTable::WS_PRE_LINE:
							if (*text == '\n') {
								haveContent();
								myModelReader.addLineBreak();
							}
							break;
						default:
							break;
						}
						++text;
						--len;
					}
				}
				if (len > 0) {
					haveContent();
					myModelReader.addData(std::string(text, len));
				}
			}
			break;
	}
}

const std::vector<std::string> &XHTMLReader::externalDTDs() const {
	return EntityFilesCollector::Instance().externalDTDs("xhtml");
}

bool XHTMLReader::processNamespaces() const {
	return true;
}
