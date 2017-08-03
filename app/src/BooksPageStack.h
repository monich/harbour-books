/*
 * Copyright (C) 2016-2017 Jolla Ltd.
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

#ifndef BOOKS_STACK_MODEL_H
#define BOOKS_STACK_MODEL_H

#include "BooksTypes.h"
#include "BooksPos.h"

#include <QAbstractListModel>
#include <QtQml>

class BooksPageStack: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

    enum Role {
        PageRole = Qt::UserRole // "page"
    };

public:
    explicit BooksPageStack(QObject* aParent = NULL);

    int count() const;

    int currentIndex() const;
    void setCurrentIndex(int aIndex);

    int currentPage() const;
    void setCurrentPage(int aPage);

    BooksPos::Stack getStack() const;
    void setStack(BooksPos::List aStack, int aCurrentPos);
    void setPageMarks(BooksPos::List aPageMarks);

    // QAbstractListModel
    QHash<int,QByteArray> roleNames() const;
    int rowCount(const QModelIndex& aParent) const;
    QVariant data(const QModelIndex& aIndex, int aRole) const;
    bool setData(const QModelIndex& aIndex, const QVariant& aValue, int aRole);

    Q_INVOKABLE int pageAt(int aIndex);
    Q_INVOKABLE void pushPage(int aPage);
    Q_INVOKABLE void pushPosition(BooksPos aPos);
    Q_INVOKABLE void pop();
    Q_INVOKABLE void clear();
    Q_INVOKABLE void back();
    Q_INVOKABLE void forward();

Q_SIGNALS:
    void changed();
    void countChanged();
    void currentIndexChanged();
    void currentPageChanged();

private:
    class Entry;
    class Private;
    Private* iPrivate;
};

QML_DECLARE_TYPE(BooksPageStack)

#endif // BOOKS_STACK_MODEL_H
