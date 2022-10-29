/*
 * Copyright (C) 2022 Jolla Ltd.
 * Copyright (C) 2022 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

#ifndef BOOKS_COLOR_SCHEME_H
#define BOOKS_COLOR_SCHEME_H

#include <QMetaType>
#include <QString>
#include <QRgb>

// Let preprocessor magic begin!
// color(colorName,ColorName,key,default)
#define BOOKS_COLORS(color) \
  color(background,Background,bg,0xffffff) \
  color(foreground,Foreground,fg,0x000000) \
  color(selectionBackground,SelectionBackground,sb,0x3c8bff) \
  color(highlightedText,HighlightedText,ht,0x3c8bff) \
  color(internalHyperlink,InternalHyperlink,ih,0x2160b4) \
  color(externalHyperlink,ExternalHyperlink,eh,0x2160b4)

// Container for colors (thread-safe)
class BooksColorScheme
{
public:
    BooksColorScheme();
    BooksColorScheme(const QString&);
    BooksColorScheme(const BooksColorScheme&);
    ~BooksColorScheme();

    BooksColorScheme& operator = (const BooksColorScheme&);
    bool operator == (const BooksColorScheme&) const;
    bool operator != (const BooksColorScheme&) const;
    bool equals(const BooksColorScheme&) const;

    bool isInverted() const;
    const QString schemeId() const;
    const QString toString() const;

    // This generates pairs of method declarations, e.g.
    // QRgb background() const;
    // BooksColorScheme withBackground(QRgb aColor) const;
    #define BOOKS_COLORS_DECL(colorName,ColorName,key,default) \
    QRgb colorName() const; \
    inline QString colorName##Color() const { return rgbToString(colorName()); } \
    BooksColorScheme with##ColorName(QRgb) const Q_REQUIRED_RESULT;
    BOOKS_COLORS(BOOKS_COLORS_DECL)
    #undef BOOKS_COLORS_DECL

    BooksColorScheme inverted() const Q_REQUIRED_RESULT;
    BooksColorScheme invertedWithSelectionBackground(QRgb) const Q_REQUIRED_RESULT;

private:
    static QString rgbToString(QRgb);

private:
    class Private;
    Private* iPrivate;
};

// Inline methods
inline bool BooksColorScheme::operator == (const BooksColorScheme& aScheme) const
    { return equals(aScheme); }
inline bool BooksColorScheme::operator != (const BooksColorScheme& aScheme) const
    { return !equals(aScheme); }

Q_DECLARE_METATYPE(BooksColorScheme)

#endif // BOOKS_COLOR_SCHEME_H
