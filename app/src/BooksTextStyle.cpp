/*
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "BooksTextStyle.h"
#include "BooksDefs.h"

class BooksTextStyle::Default
{
public:
    static weak_ptr<ZLTextStyle> instance;
    static const std::string FONT_FAMILY;
};

weak_ptr<ZLTextStyle> BooksTextStyle::Default::instance;
const std::string BooksTextStyle::Default::FONT_FAMILY("Times");

shared_ptr<ZLTextStyle> BooksTextStyle::defaults()
{
    shared_ptr<ZLTextStyle> style = Default::instance;
    if (style.isNull()) {
        style = new BooksTextStyle;
        Default::instance = style;
    }
    return style;
}

BooksTextStyle::BooksTextStyle()
{
    iFontFamilies.push_back(Default::FONT_FAMILY);
}

bool
BooksTextStyle::equalLayout(
    shared_ptr<ZLTextStyle> aStyle1,
    shared_ptr<ZLTextStyle> aStyle2)
{
    if (aStyle1.isNull()) aStyle1 = defaults();
    if (aStyle2.isNull()) aStyle2 = defaults();
    return aStyle1->fontSize() == aStyle2->fontSize();
}

const std::string& BooksTextStyle::colorStyle() const
{
    return REGULAR_TEXT;
}

bool BooksTextStyle::isDecorated() const
{
    return false;
}

const std::vector<std::string>& BooksTextStyle::fontFamilies() const
{
    return iFontFamilies;
}

int BooksTextStyle::fontSize() const
{
    return (26 * BOOKS_PPI / 330) & (~1); // Make sure it's divisible by 2
}

bool BooksTextStyle::bold() const
{
    return false;
}

bool BooksTextStyle::italic() const
{
    return false;
}

short BooksTextStyle::spaceBefore(const ZLTextStyleEntry::Metrics&) const
{
    return 0;
}

short BooksTextStyle::spaceAfter(const ZLTextStyleEntry::Metrics&) const
{
    return 0;
}

short BooksTextStyle::lineStartIndent(const ZLTextStyleEntry::Metrics&, bool) const
{
    return 0;
}

short BooksTextStyle::lineEndIndent(const ZLTextStyleEntry::Metrics&, bool) const
{
    return 0;
}

short BooksTextStyle::firstLineIndentDelta(const ZLTextStyleEntry::Metrics&) const
{
    return 0;
}

int BooksTextStyle::verticalShift() const
{
    return 0;
}

ZLTextAlignmentType BooksTextStyle::alignment() const
{
    return ALIGN_LEFT;
}

double BooksTextStyle::lineSpace() const
{
    return 1.4;
}

bool BooksTextStyle::allowHyphenations() const
{
    return true;
}
