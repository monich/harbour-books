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

#include "BooksCoverModel.h"
#include "BooksShelf.h"
#include "BooksBook.h"

#include "HarbourDebug.h"

BooksCoverModel::BooksCoverModel(QObject* aParent) :
    QSortFilterProxyModel(aParent)
{
    connect(this, SIGNAL(sourceModelChanged()), SIGNAL(sourceChanged()));
}

bool BooksCoverModel::filterAcceptsRow(int aRow, const QModelIndex& aParent) const
{
    BooksShelf* shelf = qobject_cast<BooksShelf*>(sourceModel());
    if (shelf) {
        BooksBook* book = qobject_cast<BooksBook*>(shelf->get(aRow));
        if (book) {
            return book->hasCoverImage();
        }
    }
    return false;
}

int BooksCoverModel::count() const
{
    return rowCount(QModelIndex());
}

void BooksCoverModel::setSource(QObject* aSrc)
{
    QAbstractItemModel* oldM = sourceModel();
    if (aSrc != oldM) {
        const int oldCount = count();
        if (oldM) {
            BooksShelf* oldShelf = qobject_cast<BooksShelf*>(oldM);
            if (oldShelf) {
                const int n = oldShelf->count();
                for (int i=0; i<n; i++) {
                    onBookRemoved(oldShelf->bookAt(i));
                }
            }
            oldM->disconnect(this);
        }
        if (aSrc) {
            QAbstractItemModel* newM = qobject_cast<QAbstractItemModel*>(aSrc);
            if (newM) {
                setSourceModel(newM);
                BooksShelf* newShelf = qobject_cast<BooksShelf*>(newM);
                if (newShelf) {
                    const int n = newShelf->count();
                    for (int i=0; i<n; i++) {
                        onBookAdded(newShelf->bookAt(i));
                    }
                    connect(newShelf,
                        SIGNAL(bookAdded(BooksBook*)),
                        SLOT(onBookAdded(BooksBook*)));
                    connect(newShelf,
                        SIGNAL(bookRemoved(BooksBook*)),
                        SLOT(onBookRemoved(BooksBook*)));
                }
                connect(newM,
                    SIGNAL(rowsInserted(QModelIndex,int,int)),
                    SIGNAL(countChanged()));
                connect(newM,
                    SIGNAL(rowsRemoved(QModelIndex,int,int)),
                    SIGNAL(countChanged()));
                connect(newM,
                    SIGNAL(modelReset()),
                    SIGNAL(countChanged()));
            } else {
                HDEBUG("unexpected source" << aSrc);
                setSourceModel(NULL);
            }
        } else {
            setSourceModel(NULL);
        }
        if (oldCount != count()) {
            Q_EMIT countChanged();
        }
    }
}

void BooksCoverModel::onCoverImageChanged()
{
    beginResetModel();
    endResetModel();
    Q_EMIT countChanged();
}

void BooksCoverModel::onBookAdded(BooksBook* aBook)
{
    if (aBook) {
        connect(aBook,
            SIGNAL(coverImageChanged()),
            SLOT(onCoverImageChanged()));
    }
}

void BooksCoverModel::onBookRemoved(BooksBook* aBook)
{
    if (aBook) {
        aBook->disconnect(this);
    }
}
