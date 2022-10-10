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

#include "BooksColorSchemeModel.h"

#include "HarbourDebug.h"

#include <QColor>

// Model roles
#define MODEL_ROLES_(first,role,last) \
    first(Key,key) \
    role(Label,label) \
    last(Color,color)

#define MODEL_ROLES(role) \
    MODEL_ROLES_(role,role,role)

// ==========================================================================
// BooksColorSchemeModel::Private
// ==========================================================================

class BooksColorSchemeModel::Private
{
public:
    struct ColorDescriptor {
        const QString iName;
        QString (*iLabel)();
        QRgb (BooksColorScheme::*iGetter)() const;
        BooksColorScheme (BooksColorScheme::*iSetter)(QRgb) const;
    };

    enum Role {
        #define FIRST(X,x) X##Role = Qt::UserRole,
        #define ROLE(X,x) X##Role,
        #define LAST(X,x) X##Role
        MODEL_ROLES_(FIRST,ROLE,LAST)
        #undef FIRST
        #undef ROLE
        #undef LAST
    };

    static const ColorDescriptor COLORS[];

    #define COLOR_LABEL(colorName,ColorName,key,default) \
    static QString ColorName##Label();
    BOOKS_COLORS(COLOR_LABEL)
    #undef COLOR_LABEL

public:
    BooksColorScheme iColorScheme;
};

const BooksColorSchemeModel::Private::ColorDescriptor
BooksColorSchemeModel::Private::COLORS[] = {
    #define COLOR_DESCRIPTOR(colorName,ColorName,key,default) \
    { QString(#colorName), &BooksColorSchemeModel::Private::ColorName##Label, \
      &BooksColorScheme::colorName, &BooksColorScheme::with##ColorName },
    BOOKS_COLORS(COLOR_DESCRIPTOR)
    #undef COLOR_DESCRIPTOR
};

#define COLOR_COUNT ((int)\
    (sizeof(BooksColorSchemeModel::Private::COLORS)/ \
     sizeof(BooksColorSchemeModel::Private::COLORS[0])))

// Localized labels

QString
BooksColorSchemeModel::Private::BackgroundLabel()
{
    //: List item label (description of a color scheme element)
    //% "Page background"
    return qtTrId("harbour-books-color-page_background");
}

QString
BooksColorSchemeModel::Private::ForegroundLabel()
{
    //: List item label (description of a color scheme element)
    //% "Regular text"
    return qtTrId("harbour-books-color-text");
}

QString
BooksColorSchemeModel::Private::SelectionBackgroundLabel()
{
    //: List item label (description of a color scheme element)
    //% "Selection background"
    return qtTrId("harbour-books-color-selection_background");
}

QString
BooksColorSchemeModel::Private::HighlightedTextLabel()
{
    //: List item label (description of a color scheme element)
    //% "Highlighted text"
    return qtTrId("harbour-books-color-highlighted_text");
}

QString
BooksColorSchemeModel::Private::InternalHyperlinkLabel()
{
    //: List item label (description of a color scheme element)
    //% "Internal hyperlink"
    return qtTrId("harbour-books-color-internal_hyperlink");
}

QString
BooksColorSchemeModel::Private::ExternalHyperlinkLabel()
{
    //: List item label (description of a color scheme element)
    //% "External hyperlink"
    return qtTrId("harbour-books-color-external_hyperlink");
}

// ==========================================================================
// BooksColorSchemeModel
// ==========================================================================

BooksColorSchemeModel::BooksColorSchemeModel(
    QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private)
{
}

BooksColorSchemeModel::~BooksColorSchemeModel()
{
    delete iPrivate;
}

BooksColorScheme
BooksColorSchemeModel::colorScheme() const
{
    return iPrivate->iColorScheme;
}

void
BooksColorSchemeModel::setColorScheme(
    BooksColorScheme aScheme)
{
    int i, nchanged = 0;
    bool changed[COLOR_COUNT];
    const BooksColorScheme* s1 = &iPrivate->iColorScheme;
    const BooksColorScheme* s2 = &aScheme;

    for (i = 0; i < COLOR_COUNT; i++) {
        const Private::ColorDescriptor* colorDesc = Private::COLORS + i;

        if ((s1->*(colorDesc->iGetter))() != (s2->*(colorDesc->iGetter))()) {
            changed[i] = true;
            nchanged++;
        } else {
            changed[i] = false;
        }
    }

    if (nchanged) {
        HDEBUG(aScheme.toString());
        iPrivate->iColorScheme = aScheme;
        const QVector<int> roles(1, Private::ColorRole);
        for (i = 0; i < COLOR_COUNT && nchanged > 0; i++) {
            if (changed[i]) {
                const QModelIndex idx(index(i));
                Q_EMIT dataChanged(idx, idx, roles);
                nchanged--;
            }
        }
        Q_EMIT colorSchemeChanged();
    }
}

Qt::ItemFlags
BooksColorSchemeModel::flags(
    const QModelIndex& aIndex) const
{
    return QAbstractListModel::flags(aIndex) | Qt::ItemIsEditable;
}

QHash<int,QByteArray>
BooksColorSchemeModel::roleNames() const
{
    QHash<int,QByteArray> roles;
#define ROLE(X,x) roles.insert(Private::X##Role, #x);
MODEL_ROLES(ROLE)
#undef ROLE
    return roles;
}

int
BooksColorSchemeModel::rowCount(
    const QModelIndex& aParent) const
{
    return COLOR_COUNT;
}

QVariant
BooksColorSchemeModel::data(
    const QModelIndex& aIndex,
    int aRole) const
{
    const int row = aIndex.row();

    if (row >= 0 && row < COLOR_COUNT) {
        const Private::ColorDescriptor* colorDesc = Private::COLORS + row;

        switch ((Private::Role)aRole) {
        case Private::KeyRole:
            return colorDesc->iName;
        case Private::LabelRole:
            return colorDesc->iLabel();
        case Private::ColorRole:
            return QColor(((&iPrivate->iColorScheme)->*(colorDesc->iGetter))());
        }
    }
    return QVariant();
}

bool
BooksColorSchemeModel::setData(
    const QModelIndex& aIndex,
    const QVariant& aValue,
    int aRole)
{
    const int row = aIndex.row();

    if (row >= 0 && row < COLOR_COUNT) {
        QColor newColor;

        switch ((Private::Role)aRole) {
        case Private::KeyRole:
        case Private::LabelRole:
            break;
        case Private::ColorRole:
            newColor = aValue.value<QColor>();
            if (newColor.isValid()) {
                const QRgb rgb = newColor.rgb();
                const Private::ColorDescriptor* colorDesc = Private::COLORS + row;
                const BooksColorScheme* scheme = &iPrivate->iColorScheme;

                if ((scheme->*(colorDesc->iGetter))() != rgb) {
                    const QVector<int> roles(1, Private::ColorRole);

                    HDEBUG(colorDesc->iName << newColor);
                    iPrivate->iColorScheme = (scheme->*(colorDesc->iSetter))(rgb);
                    Q_EMIT dataChanged(aIndex, aIndex, roles);
                    Q_EMIT colorSchemeChanged();
                }
                return true;
            }
            break;
        }
    }
    return false;
}
