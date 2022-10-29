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

#include "BooksColorScheme.h"

#include "HarbourDebug.h"
#include "HarbourUtil.h"

#include <QHash>
#include <QAtomicInt>
#include <QCryptographicHash>

#define MEMBER_VAR(Name) i##Name

// ==========================================================================
// BooksColorScheme::Private
// ==========================================================================

class BooksColorScheme::Private
{
public:
    struct Colors {
        #define MEMBER_DECL(colorName,ColorName,key,default) \
        QRgb MEMBER_VAR(ColorName);
        BOOKS_COLORS(MEMBER_DECL)
        #undef MEMBER_DECL
        bool isDefault() const;
        void invert();
    };

    static const Colors DEFAULT_COLORS;
    static const QString DEFAULT_SCHEME_ID;

    Private(const Colors*, bool);
    ~Private();

    void updateSchemeId();

    static QString generateSchemeId(const Colors*, bool);
    static QString defaultSchemeId() { return generateSchemeId(&DEFAULT_COLORS, false); }
    static void parseRgb(QRgb*, const QString&);
    static uint rgb(QRgb);

public:
    QAtomicInt iRef;
    QString iSchemeId;
    Colors iColors;
    bool iInverted;
};

const BooksColorScheme::Private::Colors BooksColorScheme::Private::DEFAULT_COLORS = {
    #define DEFAULT_INIT(colorName,ColorName,key,default) default,
    BOOKS_COLORS(DEFAULT_INIT)
    #undef DEFAULT_INIT
};
const QString BooksColorScheme::Private::DEFAULT_SCHEME_ID
    (BooksColorScheme::Private::defaultSchemeId());

BooksColorScheme::Private::Private(
    const Colors* aColors,
    bool aInverted) :
    iRef(1),
    iColors(*aColors),
    iInverted(aInverted)
{
    // Note: leaving iSchemeId empty. Caller must do updateSchemeId()
}

BooksColorScheme::Private::~Private()
{
}

QString
BooksColorScheme::Private::generateSchemeId(
    const Colors* aColors,
    bool aInverted)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    #define HASH_COLOR(colorName,ColorName,key,default) \
    md5.addData((const char*)&aColors->MEMBER_VAR(ColorName), sizeof(aColors->MEMBER_VAR(ColorName)));
    BOOKS_COLORS(HASH_COLOR)
    #undef HASH_COLOR
    md5.addData((const char*)&aInverted, sizeof(aInverted));
    return QString(QLatin1String(md5.result().toHex()));
}

inline bool
BooksColorScheme::Private::Colors::isDefault() const
{
    return !memcmp(this, &DEFAULT_COLORS, sizeof(Colors));
}

void
BooksColorScheme::Private::Colors::invert()
{
    #define INVERT_COLOR(colorName,ColorName,key,default) \
    MEMBER_VAR(ColorName) = HarbourUtil::invertedRgb(MEMBER_VAR(ColorName));
    BOOKS_COLORS(INVERT_COLOR)
    #undef INVERT_COLOR
}

inline
void
BooksColorScheme::Private::updateSchemeId()
{
    iSchemeId = generateSchemeId(&iColors, iInverted);
}

void
BooksColorScheme::Private::parseRgb(
    QRgb* aRgb,
    const QString& aValue)
{
    const int len = aValue.length();
    if (len > 0 && len <= 8) {
        uint rgb = 0;
        for (int i = 0; i < len; i++) {
            const int c = aValue.at(i).unicode();
            if (isxdigit(c)) {
                static const uchar hex[] = {
                    0, 1, 2, 3, 4, 5, 6, 7,     /* 0x30..0x37 */
                    8, 9, 0, 0, 0, 0, 0, 0,     /* 0x3a..0x3f */
                    0,10,11,12,13,14,15, 0,     /* 0x40..0x47 */
                    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x4a..0x4f */
                    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x40..0x47 */
                    0, 0, 0, 0, 0, 0, 0, 0,     /* 0x5a..0x5f */
                    0,10,11,12,13,14,15, 0,     /* 0x60..0x66 */
                    0, 0, 0, 0, 0, 0, 0, 0      /* Make it 64 bytes */
                };
                Q_STATIC_ASSERT(sizeof(hex) == 0x40);
                rgb = (rgb << 4) | hex[(c - 0x30) & 0x3f];
            } else {
                HDEBUG("Not a valid hex string" << aValue);
                return;
            }
        }
        *aRgb = rgb;
    }
}

uint
BooksColorScheme::Private::rgb(
    QRgb aRgb)
{
    // Strip off opaque alpha
    return (qAlpha(aRgb) == 0xff) ? (aRgb & RGB_MASK) : aRgb;
}

// ==========================================================================
// BooksColorScheme
// ==========================================================================

BooksColorScheme::BooksColorScheme() :
    iPrivate(Q_NULLPTR)
{
}

BooksColorScheme::BooksColorScheme(
    const QString& aSpec) :
    iPrivate(Q_NULLPTR)
{
    QHash<QString,QString> map;
    const QStringList parts(aSpec.split(QChar(':'), QString::SkipEmptyParts));
    const int n = parts.count();
    for (int i = 0; i < n; i++) {
        const QStringList pair(parts.at(i).split(QChar('='), QString::SkipEmptyParts));
        if (pair.count() == 2) {
            map.insert(pair.at(0), pair.at(1));
        }
    }

    Private::Colors colors = Private::DEFAULT_COLORS;
    #define PARSE_KEY(colorName,ColorName,key,default) \
    Private::parseRgb(&colors.MEMBER_VAR(ColorName), map.value(QStringLiteral(#key)));
    BOOKS_COLORS(PARSE_KEY)
    #undef PARSE_KEY

    if (!colors.isDefault()) {
        (iPrivate = new Private(&colors, false))->updateSchemeId();
    }
}

BooksColorScheme::BooksColorScheme(
    const BooksColorScheme& aScheme) :
    iPrivate(aScheme.iPrivate)
{
    if (iPrivate) {
        iPrivate->iRef.ref();
    }
}

BooksColorScheme::~BooksColorScheme()
{
    if (iPrivate && !iPrivate->iRef.deref()) {
        delete iPrivate;
    }
}

bool
BooksColorScheme::isInverted() const
{
    return iPrivate && iPrivate->iInverted;
}

const QString
BooksColorScheme::schemeId() const
{
    return iPrivate ? iPrivate->iSchemeId : Private::DEFAULT_SCHEME_ID;
}

BooksColorScheme&
BooksColorScheme::operator=(
    const BooksColorScheme& aScheme)
{
    if (iPrivate != aScheme.iPrivate) {
        if (iPrivate && !iPrivate->iRef.deref()) {
            delete iPrivate;
        }
        iPrivate = aScheme.iPrivate;
        if (iPrivate) {
            iPrivate->iRef.ref();
        }
    }
    return *this;
}

bool
BooksColorScheme::equals(
    const BooksColorScheme& aScheme) const
{
    if (iPrivate == aScheme.iPrivate) {
        return true;
    } else if (iPrivate && aScheme.iPrivate) {
        #define MEMBER_EQUAL(colorName,ColorName,key,default) \
        iPrivate->iColors.MEMBER_VAR(ColorName) == \
        aScheme.iPrivate->iColors.MEMBER_VAR(ColorName) &&
        return BOOKS_COLORS(MEMBER_EQUAL) true;
        #undef MEMBER_EQUAL
    } else {
        #define METHOD_EQUAL(colorName,ColorName,key,default) \
        colorName() == aScheme.colorName() &&
        return BOOKS_COLORS(METHOD_EQUAL) true;
        #undef METHOD_EQUAL
    }
}

const QString
BooksColorScheme::toString() const
{
    Private::Colors colors = iPrivate ? iPrivate->iColors : Private::DEFAULT_COLORS;
    // QString::asprintf requires Qt 5.5
    return QString().sprintf(
    #define COLOR_FORMAT(colorName,ColorName,key,def) #key "=%06x:"
    BOOKS_COLORS(COLOR_FORMAT)
    #undef COLOR_FORMAT
    "%s",
    #define COLOR_VALUE(colorName,ColorName,key,def) \
    Private::rgb(colors.MEMBER_VAR(ColorName)),
    BOOKS_COLORS(COLOR_VALUE)
    #undef COLOR_VALUE
    "");
}

#define GETTER_IMPL(colorName,ColorName,key,default) \
QRgb BooksColorScheme::colorName() const { \
    return iPrivate ? iPrivate->iColors.MEMBER_VAR(ColorName) : \
        Private::DEFAULT_COLORS.MEMBER_VAR(ColorName); \
}
BOOKS_COLORS(GETTER_IMPL)
#undef GETTER_IMPL

#define SETTER_IMPL(colorName,ColorName,key,default) \
BooksColorScheme BooksColorScheme::with##ColorName(QRgb aColor) const { \
    if (colorName() == aColor) { return *this; } else { \
        BooksColorScheme scheme; \
        if (iPrivate) { \
            Private::Colors colors(iPrivate->iColors); \
            colors.MEMBER_VAR(ColorName) = aColor; \
            if (isInverted() || !colors.isDefault()) { \
                (scheme.iPrivate = new Private(&colors, isInverted()))->updateSchemeId(); \
            } \
        } else { \
            scheme.iPrivate = new Private(&Private::DEFAULT_COLORS, isInverted()); \
            scheme.iPrivate->iColors.MEMBER_VAR(ColorName) = aColor; \
            scheme.iPrivate->updateSchemeId(); \
        } \
        return scheme; \
    } \
}
BOOKS_COLORS(SETTER_IMPL)
#undef SETTER_IMPL

BooksColorScheme
BooksColorScheme::inverted() const
{
    BooksColorScheme scheme;
    Private::Colors colors = iPrivate ? iPrivate->iColors : Private::DEFAULT_COLORS;
    colors.invert();
    if (!isInverted() || !colors.isDefault()) {
        (scheme.iPrivate = new Private(&colors, !isInverted()))->updateSchemeId();
    }
    return scheme;
}

BooksColorScheme
BooksColorScheme::invertedWithSelectionBackground(
    QRgb aColor) const
{
    BooksColorScheme scheme;
    Private::Colors colors = iPrivate ? iPrivate->iColors : Private::DEFAULT_COLORS;
    colors.invert();
    colors.iSelectionBackground = aColor;
    if (!isInverted() || !colors.isDefault()) {
        (scheme.iPrivate = new Private(&colors, !isInverted()))->updateSchemeId();
    }
    return scheme;
}

QString
BooksColorScheme::rgbToString(
    QRgb aRgb)
{
    // QString::asprintf requires Qt 5.5
    return QString().sprintf("#%02x%02x%02x",
        qRed(aRgb), qGreen(aRgb), qBlue(aRgb));
}
