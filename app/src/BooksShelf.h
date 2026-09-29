/*
 * Copyright (C) 2015-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2015-2018 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#ifndef BOOKS_SHELF_MODEL_H
#define BOOKS_SHELF_MODEL_H

#include "BooksItem.h"
#include "BooksStorage.h"
#include "BooksSaveTimer.h"
#include "BooksTaskQueue.h"
#include "BooksLoadingProperty.h"

#include <QtCore/QAbstractListModel>
#include <QtQml>

class BooksShelf:
    public QAbstractListModel,
    public BooksItem,
    public BooksLoadingProperty
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int bookCount READ bookCount NOTIFY bookCountChanged)
    Q_PROPERTY(int shelfCount READ shelfCount NOTIFY shelfCountChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool accessible READ accessible CONSTANT)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(QString relativePath READ relativePath WRITE setRelativePath NOTIFY relativePathChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
    Q_PROPERTY(bool hasDummyItem READ hasDummyItem WRITE setHasDummyItem NOTIFY hasDummyItemChanged)
    Q_PROPERTY(int dummyItemIndex READ dummyItemIndex WRITE setDummyItemIndex NOTIFY dummyItemIndexChanged)
    Q_PROPERTY(BooksBook* book READ book CONSTANT)
    Q_PROPERTY(BooksShelf* shelf READ shelf CONSTANT)
    Q_PROPERTY(QObject* storage READ storageObject CONSTANT)

public:
    explicit BooksShelf(QObject* aParent = Q_NULLPTR);
    BooksShelf(BooksStorage, QString relPath);
    ~BooksShelf();

    Q_INVOKABLE QObject* get(int index) const;
    Q_INVOKABLE bool drop(QObject*);
    Q_INVOKABLE void move(int from, int to);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void removeAll();
    Q_INVOKABLE bool deleteRequested(int index) const;
    Q_INVOKABLE void setDeleteRequested(int index, bool);
    Q_INVOKABLE void cancelAllDeleteRequests();
    Q_INVOKABLE void importBook(QObject*);

    bool loading() const { return iLoadTask != NULL; }
    int count() const;
    int bookCount() const;
    int shelfCount() const;
    QString relativePath() const { return iRelativePath; }
    void setRelativePath(QString);
    BooksBook* bookAt(int) const;
    QObject* storageObject() { return &iStorage; }
    const BooksStorage& storage() const { return iStorage; }
    void setName(QString);

    bool editMode() const { return iEditMode; }
    void setEditMode(bool);

    bool hasDummyItem() const { return iDummyItemIndex >= 0; }
    void setHasDummyItem(bool);

    int dummyItemIndex() const { return iDummyItemIndex; }
    void setDummyItemIndex(int);

    QString device() { return iStorage.device(); }
    void setDevice(QString);

    // QAbstractListModel
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex&) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex&, int) const Q_DECL_OVERRIDE;

    // BooksItem
    BooksItem* retain() Q_DECL_OVERRIDE;
    void release() Q_DECL_OVERRIDE;
    QObject* object() Q_DECL_OVERRIDE;
    BooksShelf* shelf() Q_DECL_OVERRIDE;
    BooksBook* book() Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString fileName() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;
    bool accessible() const Q_DECL_OVERRIDE;
    void deleteFiles() Q_DECL_OVERRIDE;
    BooksItem* copyTo(const BooksStorage&, QString relPath,
        CopyOperation*) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void loadingChanged();
    void countChanged();
    void bookCountChanged();
    void shelfCountChanged();
    void pathChanged();
    void nameChanged();
    void deviceChanged();
    void relativePathChanged();
    void editModeChanged();
    void hasDummyItemChanged();
    void dummyItemIndexChanged();
    void bookAdded(BooksBook* book);
    void bookRemoved(BooksBook* book);

private Q_SLOTS:
    void onLoadTaskDone();
    void onBookAccessibleChanged();
    void onBookCopyingOutChanged();
    void onBookMovedAway();
    void onCopyTaskProgressChanged();
    void onCopyTaskDone();
    void onDeleteTaskDone();
    void onStorageReplaced(BooksStorage old, BooksStorage newStorage);
    void saveState();

private:
    void init();
    QString stateFileName() const;
    QString stateFileName(QString) const;
    int bookIndex(BooksBook*) const;
    int itemIndex(QString fileName, int startAt = 0) const;
    bool validIndex(int) const;
    void emitDataChangedSignal(int row, int role);
    void queueStateSave();
    void loadBookList();
    void updatePath();
    void updateFileName();
    void submitDeleteTask(int index);

private:
    class Data;
    class Counts;
    class CopyTask;
    class LoadTask;
    class ImportTask;
    class DeleteTask;
    friend class Counts;

    bool iLoadBookList;
    QList<Data*> iList;
    QList<DeleteTask*> iDeleteTasks;
    LoadTask* iLoadTask;
    QString iFileName;
    QString iPath;
    QString iRelativePath;
    BooksStorage iStorage;
    int iDummyItemIndex;
    bool iEditMode;
    QAtomicInt iRef;
    BooksSaveTimer* iSaveTimer;
    shared_ptr<BooksTaskQueue> iTaskQueue;
};

QML_DECLARE_TYPE(BooksShelf)

inline bool BooksShelf::validIndex(int aIndex) const
    { return aIndex >= 0 && aIndex < iList.count(); }
inline QString BooksShelf::stateFileName() const
    { return stateFileName(iRelativePath); }

#endif // BOOKS_SHELF_MODEL_H
