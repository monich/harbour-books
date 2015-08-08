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

#ifndef BOOKS_TEXT_STYLE_H
#define BOOKS_TEXT_STYLE_H

#include "BooksTypes.h"

#include <ZLTextStyle.h>

class BooksTextStyle : public ZLTextStyle
{
    class Default;
public:
    static shared_ptr<ZLTextStyle> defaults();
    static bool equalLayout(shared_ptr<ZLTextStyle> aStyle1, shared_ptr<ZLTextStyle> aStyle2);

private:
    static weak_ptr<ZLTextStyle> gInstance;

private:
    BooksTextStyle();

public:
    bool isDecorated() const;

    const std::vector<std::string>& fontFamilies() const;

    int fontSize() const;
    bool bold() const;
    bool italic() const;

    const std::string& colorStyle() const;

    short spaceBefore(const ZLTextStyleEntry::Metrics& aMetrics) const;
    short spaceAfter(const ZLTextStyleEntry::Metrics& aMetrics) const;
    short lineStartIndent(const ZLTextStyleEntry::Metrics& aMetrics, bool aRtl) const;
    short lineEndIndent(const ZLTextStyleEntry::Metrics& aMetrics, bool aRtl) const;
    short firstLineIndentDelta(const ZLTextStyleEntry::Metrics& aMetrics) const;
    int verticalShift() const;

    ZLTextAlignmentType alignment() const;

    double lineSpace() const;
    bool allowHyphenations() const;

private:
    std::vector<std::string> iFontFamilies;
};

#endif // BOOKS_TEXT_STYLE_H
