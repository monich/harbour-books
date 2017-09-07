/*
 * Copyright (C) 2015-2017 Jolla Ltd.
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

#ifndef BOOKS_IMPORT_MODEL_H
#define BOOKS_IMPORT_MODEL_H

#include "BooksBook.h"
#include "BooksTaskQueue.h"

#include <QHash>
#include <QVariant>
#include <QByteArray>
#include <QAbstractListModel>
#include <QtQml>

class BooksImportModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)
    Q_PROPERTY(QString destination READ destination WRITE setDestination NOTIFY destinationChanged)

public:
    explicit BooksImportModel(QObject* aParent = NULL);
    ~BooksImportModel();

    bool busy() const;
    int count() const;
    int progress() const;
    int selectedCount() const;
    QString destination() const;
    void setDestination(QString aDestination);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void setSelected(int aIndex, bool aSelected);
    Q_INVOKABLE QObject* selectedBook(int aIndex);

    // QAbstractListModel
    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& aParent) const;
    virtual QVariant data(const QModelIndex& aIndex, int aRole) const;

Q_SIGNALS:
    void countChanged();
    void busyChanged();
    void progressChanged();
    void selectedCountChanged();
    void destinationChanged();

private Q_SLOTS:
    void onScanProgress(int aProgress);
    void onBookFound(BooksBook* aBook);
    void onTaskDone();

private:
    bool validIndex(int aIndex) const;

private:
    class Task;
    class Data;
    QString iDestination;
    QList<Data*> iList;
    QVector<int> iSelectedRole;
    int iProgress;
    int iSelectedCount;
    bool iAutoRefresh;
    shared_ptr<BooksTaskQueue> iTaskQueue;
    Task* iTask;
};

QML_DECLARE_TYPE(BooksImportModel)

inline bool BooksImportModel::busy() const
    { return iTask != NULL; }
inline int BooksImportModel::count() const
    { return iList.count(); }
inline int BooksImportModel::progress() const
    { return iProgress; }
inline int BooksImportModel::selectedCount() const
    { return iSelectedCount; }
inline QString BooksImportModel::destination() const
    { return iDestination; }
inline bool BooksImportModel::validIndex(int aIndex) const
    { return aIndex >= 0 && aIndex < iList.count(); }

#endif // BOOKS_IMPORT_MODEL_H
