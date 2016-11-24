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
 *   * Neither the name of Jolla Ltd nor the names of its contributors
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

#ifndef BOOKS_POSITION_H
#define BOOKS_POSITION_H

#include <QVariant>
#include <QDebug>
#include <QList>

#include "ZLTextParagraphCursor.h"

struct BooksPos {
    int iParagraphIndex, iElementIndex, iCharIndex;

    typedef QList<BooksPos> List;
    typedef QList<BooksPos>::iterator Iterator;
    typedef QList<BooksPos>::const_iterator ConstIterator;

    BooksPos() :
        iParagraphIndex(-1),
        iElementIndex(-1),
        iCharIndex(-1) {}

    BooksPos(const ZLTextWordCursor& aCursor) :
        iElementIndex(aCursor.elementIndex()),
        iCharIndex(aCursor.charIndex())
    {
        ZLTextParagraphCursorPtr ptr(aCursor.paragraphCursorPtr());
        iParagraphIndex = ptr.isNull() ? 0 : ptr->index();
    }

    BooksPos(int aParagraphIndex, int aElementIndex, int aCharIndex) :
        iParagraphIndex(aParagraphIndex),
        iElementIndex(aElementIndex),
        iCharIndex(aCharIndex) {}

    BooksPos(const BooksPos& aPos) :
        iParagraphIndex(aPos.iParagraphIndex),
        iElementIndex(aPos.iElementIndex),
        iCharIndex(aPos.iCharIndex) {}

    void invalidate()
    {
        iParagraphIndex = iElementIndex = iCharIndex = -1;
    }

    bool valid() const
    {
        return iParagraphIndex >= 0 && iElementIndex >= 0 && iCharIndex >= 0;
    }

    QVariant toVariant() const
    {
        QVariantList list;
        list.append(iParagraphIndex);
        list.append(iElementIndex);
        list.append(iCharIndex);
        return QVariant::fromValue(list);
    }

    static BooksPos fromVariant(QVariant aVariant)
    {
        if (aVariant.isValid()) {
            QVariantList list = aVariant.toList();
            if (list.count() == 3) {
                bool ok = false;
                int paraIndex = list.at(0).toInt(&ok);
                if (ok) {
                    int elemIndex = list.at(1).toInt(&ok);
                    if (ok) {
                        int charIndex = list.at(2).toInt(&ok);
                        if (ok) {
                            return BooksPos(paraIndex, elemIndex, charIndex);
                        }
                    }
                }
            }
        }
        return BooksPos();
    }

    const BooksPos& operator = (const BooksPos& aPos)
    {
        iParagraphIndex = aPos.iParagraphIndex;
        iElementIndex = aPos.iElementIndex;
        iCharIndex = aPos.iCharIndex;
        return *this;
    }

    bool operator < (const BooksPos& aPos) const
    {
        return (iParagraphIndex < aPos.iParagraphIndex) ? true :
               (iParagraphIndex > aPos.iParagraphIndex) ?  false :
               (iElementIndex < aPos.iElementIndex) ? true :
               (iElementIndex > aPos.iElementIndex) ? false :
               (iCharIndex < aPos.iCharIndex) ?  true :
               (iCharIndex > aPos.iCharIndex) ?  false : true;
    }

    bool operator > (const BooksPos& aPos) const
    {
        return (aPos < *this);
    }

    bool operator <= (const BooksPos& aPos) const
    {
        return !(*this > aPos);
    }

    bool operator >= (const BooksPos& aPos) const
    {
        return !(*this < aPos);
    }

    bool operator == (const BooksPos& aPos) const
    {
        return iParagraphIndex == aPos.iParagraphIndex &&
               iElementIndex == aPos.iElementIndex &&
               iCharIndex == aPos.iCharIndex;
    }

    bool operator != (const BooksPos& aPos) const
    {
        return iParagraphIndex != aPos.iParagraphIndex ||
               iElementIndex != aPos.iElementIndex ||
               iCharIndex != aPos.iCharIndex;
    }

    QString toString() const
    {
        return QString("BooksPos(%1,%2,%3)").arg(iParagraphIndex).
            arg(iElementIndex).arg(iCharIndex);
    }

    static BooksPos posAt(BooksPos::List aList, int aPage)
    {
        return (aPage >=0 && aPage < aList.count()) ?
            aList.at(aPage) : BooksPos();
    }
};

inline QDebug& operator<<(QDebug& aDebug, const BooksPos& aPos)
    { aDebug << qPrintable(aPos.toString()); return aDebug; }

#endif /* BOOKS_POSITION_H */
