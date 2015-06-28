/*
  Copyright (C) 2015 Jolla Ltd.
  Contact: Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of the BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Nemo Mobile nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BOOKS_TEXT_VIEW_H
#define BOOKS_TEXT_VIEW_H

#include "BooksTypes.h"
#include "BooksPos.h"

#include "ZLColor.h"
#include "ZLTextView.h"
#include "ZLTextModel.h"
#include "ZLTextStyle.h"
#include "ZLPaintContext.h"

class BooksTextView: public ZLTextView
{
public:
    BooksTextView(ZLPaintContext& aContext,
        shared_ptr<ZLTextStyle> aTextStyle,
        BooksMargins aMargin);

public:
    BooksPos position() const;
    const BooksPos rewind();
    void gotoPosition(const BooksPos& aPos);
    bool nextPage();
    void paint();

    // ZLView
    virtual const std::string &caption() const;
    virtual ZLColor backgroundColor() const;

    // ZLTextView
    virtual shared_ptr<ZLTextPositionIndicatorInfo> indicatorInfo() const;
    virtual int doubleClickDelay() const;
    virtual int leftMargin() const;
    virtual int rightMargin() const;
    virtual int topMargin() const;
    virtual int bottomMargin() const;

    // ZLTextArea::Properties
    virtual shared_ptr<ZLTextStyle> baseStyle() const;
    virtual ZLColor color(const std::string &style = std::string()) const;
    virtual bool isSelectionEnabled() const;

public:
    BooksMargins iMargins;
    ZLColor iBackgroundColor;

private:
    std::string iCaption;
    shared_ptr<ZLTextStyle> iTextStyle;
};

inline BooksPos BooksTextView::position() const
    { return BooksPos(textArea().startCursor()); }

#endif // BOOKS_TEXT_VIEW_H
