/*
 * Copyright (C) 2015-2020 Jolla Ltd.
 * Copyright (C) 2015-2020 Slava Monich <slava.monich@jolla.com>
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

#ifndef BOOKS_PAINT_CONTEXT_H
#define BOOKS_PAINT_CONTEXT_H

#include "BooksTypes.h"

#include "ZLColor.h"
#include "ZLPaintContext.h"

#include <QFont>
#include <QSize>
#include <QColor>

class QPainter;

class BooksPaintContext : public ZLPaintContext {
public:
    BooksPaintContext(int aWidth, int aHeight);
    BooksPaintContext();
    ~BooksPaintContext();

    void setWidth(int aWidth);
    void setHeight(int aHeight);
    void setSize(int aWidth, int aHeight);
    bool isEmpty() const;
    QSize size() const;

    // ZLPaintContext
    void beginPaint(QPainter* painter);
    void endPaint();

    int width() const;
    int height() const;

    void clear(ZLColor color);

    void fillFamiliesList(std::vector<std::string>& families) const;
    const std::string realFontFamilyName(std::string& fontFamily) const;

    void setFont(const std::string& family, int size, bool bold, bool italic);
    void setColor(ZLColor color, LineStyle style);
    void setFillColor(ZLColor color, FillStyle style);

    int stringWidth(const char* str, int len, bool rtl) const;
    int spaceWidth() const;
    int stringHeight() const;
    int descent() const;
    void drawString(int x, int y, const char* str, int len, bool rtl);

    void drawImage(int x, int y, const ZLImageData& image);
    void drawImage(int x, int y, const ZLImageData& image, int width, int height, ScalingType type);

    void drawLine(int x0, int y0, int x1, int y1);
    void fillRectangle(int x0, int y0, int x1, int y1);
    void drawFilledCircle(int x, int y, int r);

    void setInvertColors(bool aInvertColors);

    ZLColor realColor(const ZLColor aColor) const;
    ZLColor realColor(const std::string& aStyle) const;
    static ZLColor realColor(const std::string& aStyle, bool aInvert);

private:
    ZLColor realColor(uchar aRed, uchar aGreen, uchar aBlue, uchar aAlpha) const;
    static ZLColor realColor(uchar aRed, uchar aGreen, uchar aBlue, uchar aAlpha, bool aInvert);
    static ZLColor realColor(const ZLColor aColor, bool aInvert);

private:
    QPainter* iPainter;
    int iWidth;
    int iHeight;
    mutable int iSpaceWidth;
    int iDescent;
    bool iInvertColors;
    QFont iFont;
};

inline void BooksPaintContext::setWidth(int aWidth)
    { iWidth = aWidth; }
inline void BooksPaintContext::setHeight(int aHeight)
    { iHeight = aHeight; }
inline void BooksPaintContext::setSize(int aWidth, int aHeight)
    { iWidth = aWidth; iHeight = aHeight; }
inline bool BooksPaintContext::isEmpty() const
    { return (iWidth <= 0 || iHeight <= 0); }
inline QSize BooksPaintContext::size() const
    { return QSize(iWidth, iHeight); }

inline QColor qtColor(const ZLColor& aColor)
    { return QColor(aColor.Red, aColor.Green, aColor.Blue, aColor.Alpha); }
inline ZLColor BooksPaintContext::realColor(const ZLColor aColor) const
    { return realColor(aColor.Red, aColor.Green, aColor.Blue, aColor.Alpha); }
inline ZLColor BooksPaintContext::realColor(uchar aRed, uchar aGreen, uchar aBlue, uchar aAlpha) const
    { return realColor(aRed, aGreen, aBlue, aAlpha, iInvertColors); }
inline ZLColor BooksPaintContext::realColor(const ZLColor aColor, bool aInvert)
    { return realColor(aColor.Red, aColor.Green, aColor.Blue, aColor.Alpha, aInvert); }
inline ZLColor BooksPaintContext::realColor(const std::string& aStyle) const
    { return realColor(aStyle, iInvertColors); }
inline void BooksPaintContext::setInvertColors(bool aInvertColors)
    { iInvertColors = aInvertColors; }

#endif /* BOOKS_PAINT_CONTEXT_H */
