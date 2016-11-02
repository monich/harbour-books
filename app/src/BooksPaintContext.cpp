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

#include "BooksPaintContext.h"

#include "HarbourDebug.h"

#include "ZLImage.h"
#include "ZLTextStyle.h"
#include "ZLStringUtil.h"
#include "image/ZLQtImageManager.h"

#include <QPainter>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QImage>

static const std::string HELVETICA = "Helvetica";

BooksPaintContext::BooksPaintContext() :
    iPainter(NULL), iWidth(0), iHeight(0),
    iSpaceWidth(0), iDescent(0), iInvertColors(false)
{
}

BooksPaintContext::BooksPaintContext(int aWidth, int aHeight) :
    iPainter(NULL), iWidth(aWidth), iHeight(aHeight),
    iSpaceWidth(0), iDescent(0), iInvertColors(false)
{
}

BooksPaintContext::~BooksPaintContext()
{
}

void BooksPaintContext::beginPaint(QPainter *aPainter)
{
    iPainter = aPainter;
    iPainter->setFont(iFont);
}

void BooksPaintContext::endPaint()
{
    iPainter = NULL;
}

void BooksPaintContext::fillFamiliesList(std::vector<std::string> &families) const
{
    QFontDatabase db;
    const QStringList qFamilies = db.families();
    bool helveticaFlag = false;
    for (QStringList::ConstIterator it = qFamilies.begin(); it != qFamilies.end(); ++it) {
        std::string family = it->toUtf8().constData();
        helveticaFlag |= (family == HELVETICA);
        families.push_back(family);
    }
    if (!helveticaFlag) {
        families.push_back(HELVETICA);
    }
}

const std::string BooksPaintContext::realFontFamilyName(std::string &fontFamily) const
{
    QString fullName = QFontInfo(QFont(QString::fromUtf8(fontFamily.c_str()))).family();
    if (fullName.isNull() || fullName.isEmpty()) {
        return HELVETICA;
    }
    return fullName.toStdString();
}

void BooksPaintContext::setFont(const std::string& family, int size, bool bold, bool italic)
{
    bool fontChanged = false;

    if (!family.empty()) {
        QString qtFamily(QString::fromStdString(family));
        if (iFont.family() != qtFamily) {
            iFont.setFamily(qtFamily);
            fontChanged = true;
        }
    }

    if (iFont.pointSize() != size) {
        iFont.setPointSize(size);
        fontChanged = true;
    }

    if ((iFont.weight() != (bold ? QFont::Bold : QFont::Normal))) {
        iFont.setWeight(bold ? QFont::Bold : QFont::Normal);
        fontChanged = true;
    }

    if (iFont.italic() != italic) {
        iFont.setItalic(italic);
        fontChanged = true;
    }

    if (fontChanged) {
        QFontMetrics fontMetrics(iFont);
        iSpaceWidth = fontMetrics.width(QString(" "));
        iDescent = fontMetrics.descent();
        if (iPainter) {
            iPainter->setFont(iFont);
        }
    }
}

void BooksPaintContext::setColor(ZLColor color, LineStyle style)
{
    if (iPainter) {
        iPainter->setPen(QPen(qtColor(color), 1, (style == SOLID_LINE) ?
            Qt::SolidLine : Qt::DashLine));
    }
}

void BooksPaintContext::setFillColor(ZLColor color, FillStyle style)
{
    if (iPainter) {
        iPainter->setBrush(QBrush(qtColor(color), (style == SOLID_FILL) ?
            Qt::SolidPattern : Qt::Dense4Pattern));
    }
}

int BooksPaintContext::stringWidth(const char *str, int len, bool) const
{
    QFontMetrics fontMetrics(iFont);
    return fontMetrics.width(QString::fromUtf8(str, len));
}

int BooksPaintContext::spaceWidth() const
{
    return iSpaceWidth;
}

int BooksPaintContext::descent() const
{
    return iDescent;
}

int BooksPaintContext::stringHeight() const
{
    return iFont.pointSize() + 2;
}

void BooksPaintContext::drawString(int x, int y, const char* str, int len, bool rtl)
{
    if (iPainter) {
        QString qStr = QString::fromUtf8(str, len);
        iPainter->setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
        iPainter->drawText(x, y, qStr);
    }
}

void BooksPaintContext::drawImage(int x, int y, const ZLImageData& image)
{
    if (iPainter) {
        const QImage* qImage = ((ZLQtImageData&)image).image();
        if (qImage) {
            iPainter->drawImage(x, y - image.height(), *qImage);
        }
    }
}

void BooksPaintContext::drawImage(int x, int y, const ZLImageData& image,
    int width, int height, ScalingType type)
{
    if (iPainter) {
        const QImage* qImage = ((ZLQtImageData&)image).image();
        if (qImage) {
            const QImage scaled = qImage->scaled(
                QSize(imageWidth(image, width, height, type),
                      imageHeight(image, width, height, type)),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
            iPainter->drawImage(x, y - scaled.height(), scaled);
        }
    }
}

void BooksPaintContext::drawLine(int x0, int y0, int x1, int y1)
{
    if (iPainter) {
        iPainter->drawPoint(x0, y0);
        iPainter->drawLine(x0, y0, x1, y1);
        iPainter->drawPoint(x1, y1);
    }
}

void BooksPaintContext::fillRectangle(int x0, int y0, int x1, int y1)
{
    if (iPainter) {
        if (x1 < x0) {
            int tmp = x1;
            x1 = x0;
            x0 = tmp;
        }
        if (y1 < y0) {
            int tmp = y1;
            y1 = y0;
            y0 = tmp;
        }
        iPainter->fillRect(x0, y0, x1-x0+1, y1-y0+1, iPainter->brush());
    }
}

void BooksPaintContext::drawFilledCircle(int x, int y, int r)
{
    if (iPainter) {
        iPainter->drawEllipse(x - r, y - r, 2 * r + 1, 2 * r + 1);
    }
}

void BooksPaintContext::clear(ZLColor aColor)
{
    if (iPainter) {
        iPainter->fillRect(0, 0, iWidth, iHeight, qtColor(aColor));
    }
}

int BooksPaintContext::width() const
{
    return iWidth;
}

int BooksPaintContext::height() const
{
    return iHeight;
}

ZLColor BooksPaintContext::realColor(quint8 aRed, quint8 aGreen, quint8 aBlue, bool aInvert)
{
    return aInvert ?
        ZLColor(255-aRed, 255-aGreen, 255-aBlue) :
        ZLColor(aRed, aGreen, aBlue);
}

ZLColor BooksPaintContext::realColor(const std::string& aStyle, bool aInvert)
{
    static const std::string INTERNAL_HYPERLINK("internal");
    static const std::string EXTERNAL_HYPERLINK("external");
    static const std::string BOOK_HYPERLINK("book");

    if (ZLStringUtil::startsWith(aStyle, '#')) {
        if (aStyle.length() == 7) {
            int i, value = 0;
            for (i=1; i<7; i++) {
                int nibble = ZLStringUtil::fromHex(aStyle[i]);
                if (nibble >= 0) {
                    value <<= 4;
                    value |= nibble;
                } else {
                    break;
                }
            }
            if (i == 7) {
                return realColor(ZLColor(value), aInvert);
            }
        }
    } else if (aStyle == INTERNAL_HYPERLINK) {
        return realColor(33, 96, 180, aInvert);
    } else if (aStyle == EXTERNAL_HYPERLINK) {
        return realColor(33, 96, 180, aInvert);
    } else if (aStyle == BOOK_HYPERLINK) {
        return realColor(23, 68, 128, aInvert);
    } else if (aStyle == ZLTextStyle::SELECTION_BACKGROUND) {
        return realColor(82, 131, 194, aInvert);
    } else if (aStyle == ZLTextStyle::HIGHLIGHTED_TEXT) {
        return realColor(60, 139, 255, aInvert);
    }
    return realColor(0, 0, 0, aInvert);
}
