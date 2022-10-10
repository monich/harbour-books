/*
 * Copyright (C) 2015-2021 Jolla Ltd.
 * Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>
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
#include "BooksColorScheme.h"

#include "ZLColor.h"
#include "ZLPaintContext.h"

#include <QFont>
#include <QSize>
#include <QColor>

class QPainter;

class BooksPaintContext : public ZLPaintContext {
protected:
    BooksPaintContext();

public:
    BooksPaintContext(int aWidth, int aHeight, BooksColorScheme aColors = BooksColorScheme());
    ~BooksPaintContext();

    bool isEmpty() const;
    QSize size() const;

    void beginPaint(QPainter* painter);
    void endPaint();

    // ZLPaintContext
    int width() const Q_DECL_OVERRIDE;
    int height() const Q_DECL_OVERRIDE;

    void clear(ZLColor color) Q_DECL_OVERRIDE;

    void fillFamiliesList(std::vector<std::string>& families) const Q_DECL_OVERRIDE;
    const std::string realFontFamilyName(std::string& fontFamily) const Q_DECL_OVERRIDE;

    void setFont(const std::string& family, int size, bool bold, bool italic) Q_DECL_OVERRIDE;
    void setColor(ZLColor color, LineStyle style) Q_DECL_OVERRIDE;
    void setFillColor(ZLColor color, FillStyle style) Q_DECL_OVERRIDE;

    int stringWidth(const char* str, int len, bool rtl) const Q_DECL_OVERRIDE;
    int spaceWidth() const Q_DECL_OVERRIDE;
    int stringHeight() const Q_DECL_OVERRIDE;
    int descent() const Q_DECL_OVERRIDE;
    void drawString(int x, int y, const char* str, int len, bool rtl) Q_DECL_OVERRIDE;

    void drawImage(int x, int y, const ZLImageData& image) Q_DECL_OVERRIDE;
    void drawImage(int x, int y, const ZLImageData& image, int width, int height, ScalingType type) Q_DECL_OVERRIDE;

    void drawLine(int x0, int y0, int x1, int y1) Q_DECL_OVERRIDE;
    void fillRectangle(int x0, int y0, int x1, int y1) Q_DECL_OVERRIDE;
    void drawFilledCircle(int x, int y, int r) Q_DECL_OVERRIDE;

    ZLColor realColor(const ZLColor aColor) const;
    ZLColor realColor(const std::string& aStyle) const;
    static ZLColor realColor(const std::string& aStyle, BooksColorScheme aColors);

private:
    QPainter* iPainter;
    mutable int iSpaceWidth;
    int iDescent;
    QFont iFont;

protected:
    int iWidth;
    int iHeight;

public:
    BooksColorScheme iColors;
};

inline bool BooksPaintContext::isEmpty() const
    { return (iWidth <= 0 || iHeight <= 0); }
inline QSize BooksPaintContext::size() const
    { return QSize(iWidth, iHeight); }

inline QColor qtColor(const ZLColor& aColor)
    { return QColor(aColor.Red, aColor.Green, aColor.Blue, aColor.Alpha); }
inline ZLColor BooksPaintContext::realColor(const std::string& aStyle) const
    { return realColor(aStyle, iColors); }

#endif /* BOOKS_PAINT_CONTEXT_H */
