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

#ifndef BOOKS_STORAGE_MODEL_H
#define BOOKS_STORAGE_MODEL_H

#include "BooksStorage.h"

#include <QHash>
#include <QVariant>
#include <QByteArray>
#include <QAbstractListModel>
#include <QtQml>

class BooksStorageModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QObject* internalStorage READ internalStorage CONSTANT)

public:
    explicit BooksStorageModel(QObject* aParent = NULL);
    ~BooksStorageModel();

    Q_INVOKABLE int count() const;
    Q_INVOKABLE int deviceIndex(QString aDevice) const;
    Q_INVOKABLE QString deviceAt(int aIndex) const;
    Q_INVOKABLE void setDeleteAllRequest(int aIndex, bool aValue);
    Q_INVOKABLE void cancelDeleteAllRequests();
    Q_INVOKABLE QObject* get(int aIndex) const;
    QObject* internalStorage() { return &iInternalStorage; }

    // QAbstractListModel
    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& aParent) const;
    virtual QVariant data(const QModelIndex& aIndex, int aRole) const;

Q_SIGNALS:
    void countChanged();
    void newStorage(int index);

private Q_SLOTS:
    void onStorageAdded(BooksStorage aStorage);
    void onStorageRemoved(BooksStorage aStorage);

private:
    bool validIndex(int aIndex) const;
    int find(const BooksStorage& aStorage) const;

private:
    class Data;
    QList<Data*> iList;
    BooksStorage iInternalStorage;
};

QML_DECLARE_TYPE(BooksStorageModel)

inline bool BooksStorageModel::validIndex(int aIndex) const
    { return aIndex >= 0 && aIndex < iList.count(); }

#endif // BOOKS_STORAGE_MODEL_H
