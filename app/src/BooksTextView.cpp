/*
 * Copyright (C) 2015-2016 Jolla Ltd.
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

#include "BooksTextView.h"
#include "BooksTextStyle.h"

#define SUPER ZLTextView

const ZLColor BooksTextView::DEFAULT_BACKGROUND(255, 255, 255);
const ZLColor BooksTextView::INVERTED_BACKGROUND(0, 0, 0);

BooksTextView::BooksTextView(
    BooksPaintContext& aContext,
    shared_ptr<ZLTextStyle> aTextStyle,
    BooksMargins aMargins) :
    SUPER(aContext),
    iMargins(aMargins),
    iPaintContext(aContext),
    iTextStyle(aTextStyle)
{
}

void BooksTextView::paint()
{
    SUPER::paint();
}

const std::string& BooksTextView::caption() const
{
    return iCaption;
}

int BooksTextView::leftMargin() const
{
    return iMargins.iLeft + iTextStyle->fontSize();
}

int BooksTextView::rightMargin() const
{
    return iMargins.iRight + iTextStyle->fontSize();
}

int BooksTextView::topMargin() const
{
    return iMargins.iTop;
}

int BooksTextView::bottomMargin() const
{
    return iMargins.iBottom;
}

ZLColor BooksTextView::backgroundColor() const
{
    return iPaintContext.realColor(DEFAULT_BACKGROUND);
}

ZLColor BooksTextView::color(const std::string& aStyle) const
{
    return iPaintContext.realColor(aStyle);
}

shared_ptr<ZLTextStyle> BooksTextView::baseStyle() const
{
    return iTextStyle;
}

bool BooksTextView::isSelectionEnabled() const
{
    return false;
}

int BooksTextView::doubleClickDelay() const
{
    return isSelectionEnabled() ? 200 : 0;
}

shared_ptr<ZLTextPositionIndicatorInfo> BooksTextView::indicatorInfo() const
{
    return NULL;
}

const BooksPos BooksTextView::rewind()
{
    SUPER::gotoPosition(0, 0, 0);
    preparePaintInfo();
    if (!textArea().isVisible()) nextPage();
    return position();
}

bool BooksTextView::nextPage()
{
    BooksPos saved(position());
    BooksPos current(saved);
    do {
        scrollPage(true, ZLTextAreaController::NO_OVERLAPPING, 1);
        preparePaintInfo();
        const BooksPos pos = position();
        if (pos == current) {
            gotoPosition(saved);
            return false;
        }
        current = pos;
    } while (!textArea().isVisible());
    return true;
}

void BooksTextView::gotoPosition(const BooksPos& aPos)
{
    SUPER::gotoPosition(aPos.iParagraphIndex, aPos.iElementIndex,
        aPos.iCharIndex);
}
