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

#include "BooksShelf.h"
#include "BooksDefs.h"
#include "BooksBook.h"

#include "HarbourJson.h"
#include "HarbourDebug.h"

#include <unistd.h>
#include <errno.h>

enum BooksItemRole {
    BooksItemName = Qt::UserRole,
    BooksItemBook,
    BooksItemShelf,
    BooksItemAccessible,
    BooksItemCopyingOut,
    BooksItemCopyingIn,
    BooksItemCopyPercent,
    BooksItemDummy,
    BooksItemDeleteRequested
};

#define MIN_SIGNAL_DELAY    (100) /* ms */
#define FILE_PERMISSIONS \
    (QFile::ReadOwner | QFile::WriteOwner | \
     QFile::ReadGroup | QFile::ReadOther)

#define SHELF_STATE_FILE    BOOKS_STATE_FILE_SUFFIX
#define SHELF_STATE_ORDER   "order"

class BooksShelf::CopyTask : public BooksTask
{
    Q_OBJECT

public:
    CopyTask(BooksShelf::Data* aData, BooksBook* aBook);
    ~CopyTask();

    void performTask();

    bool linkFiles();

Q_SIGNALS:
    void copyPercentChanged();

public:
    BooksShelf::Data* iData;
    int iCopyPercent;
    bool iSuccess;
    QString iSource;
    QString iDest;
};

// ==========================================================================
// BooksShelf::LoadTask
// ==========================================================================

class BooksShelf::LoadTask : public BooksTask
{
    Q_OBJECT

public:
    LoadTask(BooksStorage aStorage, QString aPath, QString aStateFilePath) :
        iStorage(aStorage), iPath(aPath), iStateFilePath(aStateFilePath) {}
    ~LoadTask();

    void performTask();

    int findBook(QString aFileName) const;
    static int find(QFileInfoList aList, QString aFileName, int aStart);

Q_SIGNALS:
    void bookFound(BooksBook* aBook);

public:
    BooksStorage iStorage;
    QString iPath;
    QString iStateFilePath;
    QList<BooksBook*> iBooks;
};

BooksShelf::LoadTask::~LoadTask()
{
    const int n = iBooks.count();
    for (int i=0; i<n; i++) iBooks.at(i)->release();
}

int BooksShelf::LoadTask::find(QFileInfoList aList, QString aName, int aStart)
{
    if (!aName.isEmpty()) {
        const int n = aList.count();
        for (int i=aStart; i<n; i++) {
            if (aList.at(i).fileName() == aName) {
                return i;
            }
        }
    }
    return -1;
}

int BooksShelf::LoadTask::findBook(QString aFileName) const
{
    if (!aFileName.isEmpty()) {
        const int n = iBooks.count();
        for (int i=0; i<n; i++) {
            if (iBooks.at(i)->fileName() == aFileName) {
                return i;
            }
        }
    }
    return -1;
}

void BooksShelf::LoadTask::performTask()
{
    if (!isCanceled()) {
        QDir dir(iPath);
        HDEBUG("checking" << iPath);
        QFileInfoList list = dir.entryInfoList(QDir::Files, QDir::Time);

        // Restore the order
        QVariantMap state;
        if (HarbourJson::load(iStateFilePath, state)) {
            QVariantList order = state.value(SHELF_STATE_ORDER).toList();
            const int n = order.count();
            for (int i=0, dest=0; i<n; i++) {
                int index = find(list, order.at(i).toString(), dest);
                if (index >= 0) {
                    if (index != dest) {
                        HDEBUG(order.at(i).toString() << index << "->" << dest);
                        list.move(index, dest);
                    } else {
                        HDEBUG(order.at(i).toString() << index);
                    }
                    dest++;
                } else {
                    HDEBUG(order.at(i).toString());
                }
            }
        }

        const int n = list.count();
        for (int i=0; i<n && !isCanceled(); i++) {
            std::string path(list.at(i).filePath().toStdString());
            ZLFile file(path);
            shared_ptr<Book> book = Book::loadFromFile(file);
            if (!book.isNull()) {
                BooksBook* newBook = new BooksBook(iStorage, book);
                newBook->moveToThread(thread());
                iBooks.append(newBook);
                HDEBUG("[" << iBooks.size() << "]" <<
                    qPrintable(newBook->fileName()) <<
                    newBook->title());
                Q_EMIT bookFound(newBook);
            } else {
                HDEBUG("not a book:" << path.c_str());
            }
        }
    }

    // Cleanup the state files
    if (!isCanceled()) {
        QStringList deleteMe;
        const QString suffix(BOOKS_STATE_FILE_SUFFIX);
        QDirIterator configIt(iStorage.configDir());
        while (configIt.hasNext() && !isCanceled()) {
            QString path(configIt.next());
            if (path.endsWith(suffix)) {
                QString fileName(configIt.fileName());
                QString name(fileName.left(fileName.length() - suffix.length()));
                if (!name.isEmpty() && findBook(name) < 0) {
                    deleteMe.append(path);
                }
            }
        }
        while (!deleteMe.isEmpty() && !isCanceled()) {
            QString path(deleteMe.takeLast());
            if (QFile::remove(path)) {
                HDEBUG("removed" << qPrintable(path));
            } else {
                HWARN("failed to remove" << qPrintable(path));
            }
        }
    }
}

// ==========================================================================
// BooksShelf::Data
// ==========================================================================

class BooksShelf::Data {
public:
    Data(BooksShelf* aShelf, BooksItem* aItem, bool aExternal);
    ~Data();

    QString name() { return iItem ? iItem->name() : QString(); }
    QString fileName() { return iItem ? iItem->fileName() : QString(); }
    QObject* object() { return iItem ? iItem->object() : NULL; }
    BooksBook* book() { return iItem ? iItem->book() : NULL; }
    BooksShelf* shelf() { return iItem ? iItem->shelf() : NULL; }
    bool accessible();
    bool copyingOut();
    bool copyingIn() { return iCopyTask != NULL; }
    int copyPercent() { return iCopyTask ? iCopyTask->iCopyPercent : 0; }

    void setBook(BooksBook* aBook, bool aExternal);
    void connectSignals(BooksBook* aBook);

public:
    BooksShelf* iShelf;
    BooksItem* iItem;
    BooksShelf::CopyTask* iCopyTask;
    bool iDeleteRequested;
};

BooksShelf::Data::Data(BooksShelf* aShelf, BooksItem* aItem, bool aExternal) :
    iShelf(aShelf),
    iItem(aItem),
    iCopyTask(NULL),
    iDeleteRequested(false)
{
    if (iItem && !aExternal) {
        connectSignals(iItem->book());
    }
}

BooksShelf::Data::~Data()
{
    if (iItem) {
        iItem->object()->disconnect(iShelf);
        iItem->release();
    }
    if (iCopyTask) {
        iCopyTask->release(iShelf);
    }
}

void BooksShelf::Data::connectSignals(BooksBook* aBook)
{
    if (aBook) {
        iShelf->connect(aBook,
            SIGNAL(accessibleChanged()),
            SLOT(onBookAccessibleChanged()));
        iShelf->connect(aBook,
            SIGNAL(copyingOutChanged()),
            SLOT(onBookCopyingOutChanged()));
        iShelf->connect(aBook,
            SIGNAL(movedAway()),
            SLOT(onBookMovedAway()));
    }
}

void BooksShelf::Data::setBook(BooksBook* aBook, bool aExternal)
{
    if (iItem != aBook) {
        if (iItem) {
            iItem->object()->disconnect(iShelf);
            iItem->release();
        }
        iItem = aBook;
        if (aBook) {
            aBook->retain();
            if (!aExternal) connectSignals(aBook);
        }
    }
}

inline bool BooksShelf::Data::accessible()
{
    if (iCopyTask) {
        return false;
    } else {
        BooksBook* bookItem = book();
        return bookItem && bookItem->accessible();
    }
}

inline bool BooksShelf::Data::copyingOut()
{
    BooksBook* bookItem = book();
    return bookItem && bookItem->copyingOut();
}

// ==========================================================================
// BooksShelf::CopyTask
// ==========================================================================

BooksShelf::CopyTask::CopyTask(BooksShelf::Data* aData, BooksBook* aBook) :
    iData(aData),
    iCopyPercent(0),
    iSuccess(false),
    iSource(aBook->path()),
    iDest(QFileInfo(aData->iShelf->path(),
          QFileInfo(iSource).fileName()).absoluteFilePath())
{
    HDEBUG(qPrintable(iSource) << "->" << qPrintable(iDest));
    if (iData->iCopyTask) {
        iData->iCopyTask->release(iData->iShelf);
    }
    iData->iCopyTask = this;
    iData->iShelf->connect(this, SIGNAL(done()), SLOT(onCopyTaskDone()));
    iData->iShelf->connect(this, SIGNAL(copyPercentChanged()),
        SLOT(onCopyTaskPercentChanged()), Qt::QueuedConnection);
}

BooksShelf::CopyTask::~CopyTask()
{
    HASSERT(!iData);
}

bool BooksShelf::CopyTask::linkFiles()
{
    QByteArray oldp(iSource.toLocal8Bit());
    QByteArray newp(iDest.toLocal8Bit());
    if (!oldp.isEmpty()) {
        if (!newp.isEmpty()) {
            int err = link(oldp.data(), newp.data());
            if (!err) {
                HDEBUG("linked" << newp << "->" << oldp);
                iSuccess = true;
            } else {
                HDEBUG(newp << "->" << oldp << "error:" << strerror(errno));
            }
        } else {
            HDEBUG("failed to convert" << newp << "to locale encoding");
        }
    } else {
        HDEBUG("failed to convert" << oldp << "to locale encoding");
    }
    return iSuccess;
}

void BooksShelf::CopyTask::performTask()
{
    if (!isCanceled() && !linkFiles()) {
        QFile src(iSource);
        const qint64 total = src.size();
        qint64 copied = 0;
        if (src.open(QIODevice::ReadOnly)) {
            QFile dest(iDest);
            QDir dir(QFileInfo(dest).dir());
            dir.mkpath(dir.path());
            if (dest.open(QIODevice::WriteOnly)) {
                QDateTime lastSignal;
                const qint64 bufsiz = 0x1000;
                char* buf = new char[bufsiz];
                qint64 len;
                while (!isCanceled() && (len = src.read(buf, bufsiz)) > 0 &&
                       !isCanceled() && dest.write(buf, len) == len) {
                    copied += len;
                    int percent = (int)(copied*100/total);
                    if (iCopyPercent != percent) {
                        // Don't fire signals too often
                        QDateTime now(QDateTime::currentDateTimeUtc());
                        if (!lastSignal.isValid() ||
                            lastSignal.msecsTo(now) >= MIN_SIGNAL_DELAY) {
                            lastSignal = now;
                            iCopyPercent = percent;
                            Q_EMIT copyPercentChanged();
                        }
                    }
                }
                delete [] buf;
                dest.close();
                if (copied == total) {
                    dest.setPermissions(FILE_PERMISSIONS);
                    iSuccess = true;
                    HDEBUG(total << "bytes copied from"<< qPrintable(iSource) <<
                        "to" << qPrintable(iDest));
                } else {
                    if (isCanceled()) {
                        HDEBUG("copy" << qPrintable(iSource) <<  "to" <<
                            qPrintable(iDest) << "cancelled");
                    } else {
                        HWARN(copied << "out of" << total <<
                            "bytes copied from" << qPrintable(iSource) <<
                            "to" << qPrintable(iDest));
                    }
                    dest.remove();
                }
            } else {
                HWARN("failed to open" << qPrintable(iDest));
            }
            src.close();
        } else {
            HWARN("failed to open" << qPrintable(iSource));
        }
    }
}

// ==========================================================================
// BooksShelf::DeleteTask
// ==========================================================================

class BooksShelf::DeleteTask : public BooksTask
{
    Q_OBJECT
public:
    DeleteTask(BooksBook* aBook);
    ~DeleteTask();
    void performTask();

public:
    BooksBook* iBook;
};

BooksShelf::DeleteTask::DeleteTask(BooksBook* aBook) :
    iBook(aBook)
{
    iBook->retain();
    iBook->cancelCoverRequest();
}

BooksShelf::DeleteTask::~DeleteTask()
{
    iBook->release();
}

void BooksShelf::DeleteTask::performTask()
{
    if (isCanceled()) {
        HDEBUG("cancelled" << iBook->title());
    } else {
        HDEBUG(iBook->title());
        iBook->deleteFiles();
    }
}

// ==========================================================================
// BooksShelf
// ==========================================================================

BooksShelf::BooksShelf(QObject* aParent) :
    QAbstractListModel(aParent),
    iLoadTask(NULL),
    iDummyItemIndex(-1),
    iEditMode(false),
    iRef(-1),
    iSaveTimer(new BooksSaveTimer(this)),
    iTaskQueue(BooksTaskQueue::instance())
{
#if QT_VERSION < 0x050000
    setRoleNames(roleNames());
#endif
    QQmlEngine::setObjectOwnership(&iStorage, QQmlEngine::CppOwnership);
    connect(iSaveTimer, SIGNAL(save()), SLOT(saveState()));
}

BooksShelf::~BooksShelf()
{
    const int n = iDeleteTasks.count();
    for (int i=0; i<n; i++) iDeleteTasks.at(i)->release(this);
    if (iLoadTask) iLoadTask->release(this);
    if (iSaveTimer->saveRequested()) saveState();
    removeAllBooks();
    HDEBUG("destroyed");
}

void BooksShelf::removeAllBooks()
{
    while (!iList.isEmpty()) {
        Data* data = iList.takeLast();
        BooksBook* book = data->book();
        if (book) {
            Q_EMIT bookRemoved(book);
        }
        delete data;
    }
}

void BooksShelf::setRelativePath(QString aPath)
{
    if (iRelativePath != aPath) {
        iRelativePath = aPath;
        updatePath();
        Q_EMIT relativePathChanged();
    }
}

void BooksShelf::setDevice(QString aDevice)
{
    if (device() != aDevice) {
        iStorage = BooksStorageManager::instance()->storageForDevice(aDevice);
        updatePath();
        Q_EMIT deviceChanged();
    }
}

void BooksShelf::updatePath()
{
    const QString oldPath = iPath;
    iPath.clear();
    if (iStorage.isValid()) {
        QString newPath(iStorage.root());
        if (!iRelativePath.isEmpty()) {
            if (!newPath.endsWith('/')) newPath += '/';
            newPath += iRelativePath;
        }
        iPath = QDir::cleanPath(newPath);
    }
    if (oldPath != iPath) {
        const int oldCount = iList.count();
        const int oldDummyItemIndex = iDummyItemIndex;
        beginResetModel();
        HDEBUG(iPath);
        removeAllBooks();
        iDummyItemIndex = -1;
        if (!iPath.isEmpty()) loadBookList();
        endResetModel();
        Q_EMIT pathChanged();
        if (oldDummyItemIndex != iDummyItemIndex) {
            Q_EMIT dummyItemIndexChanged();
            if (oldDummyItemIndex <0 || iDummyItemIndex < 0) {
                Q_EMIT hasDummyItemChanged();
            }
        }
        if (oldCount != iList.count()) {
            Q_EMIT countChanged();
        }
    }
}

void BooksShelf::onLoadTaskDone()
{
    HASSERT(iLoadTask);
    HASSERT(iLoadTask == sender());
    iLoadTask->release(this);
    iLoadTask = NULL;
    Q_EMIT loadingChanged();
}

void BooksShelf::onBookFound(BooksBook* aBook)
{
    if (iLoadTask && iLoadTask == sender()) {
        beginInsertRows(QModelIndex(), iList.count(), iList.count());
        iList.append(new Data(this, aBook->retain(), false));
        endInsertRows();
        Q_EMIT bookAdded(aBook);
        Q_EMIT countChanged();
    }
}

void BooksShelf::loadBookList()
{
    if (!iList.isEmpty()) {
        beginResetModel();
        removeAllBooks();
        endResetModel();
    }

    const bool wasLoading = loading();
    if (iLoadTask) iLoadTask->release(this);
    if (iPath.isEmpty()) {
        iLoadTask = NULL;
    } else {
        HDEBUG(iPath);
        iLoadTask = new LoadTask(iStorage, iPath, stateFileName());
        connect(iLoadTask, SIGNAL(bookFound(BooksBook*)),
            SLOT(onBookFound(BooksBook*)), Qt::QueuedConnection);
        connect(iLoadTask, SIGNAL(done()), SLOT(onLoadTaskDone()));
        iTaskQueue->submit(iLoadTask);
    }
    if (wasLoading != loading()) {
        Q_EMIT loadingChanged();
    }
}

void BooksShelf::saveState()
{
    QStringList order;
    const int n = iList.count();
    for (int i=0; i<n; i++) {
        order.append(iList.at(i)->fileName());
    }
    QVariantMap state;
    state.insert(SHELF_STATE_ORDER, order);
    if (HarbourJson::save(stateFileName(), state)) {
        HDEBUG("wrote" << stateFileName());
    }
}

void BooksShelf::queueStateSave()
{
    if (iEditMode) {
        iSaveTimer->requestSave();
    }
}

QString BooksShelf::stateFileName() const
{
    return iStorage.isValid() ?
        iStorage.configDir().path() + ("/" SHELF_STATE_FILE) :
        QString();
}

int BooksShelf::bookIndex(BooksBook* aBook) const
{
    if (aBook) {
        const int n = iList.count();
        for (int i=0; i<n; i++) {
            if (iList.at(i)->book() == aBook) {
                return i;
            }
        }
    }
    return -1;
}

int BooksShelf::itemIndex(QString aFileName, int aStartIndex) const
{
    if (!aFileName.isEmpty()) {
        const int n = iList.count();
        for (int i=aStartIndex; i<n; i++) {
            if (iList.at(i)->fileName() == aFileName) {
                return i;
            }
        }
    }
    return -1;
}

void BooksShelf::setHasDummyItem(bool aHasDummyItem)
{
    if (aHasDummyItem && !hasDummyItem()) {
        iDummyItemIndex = iList.count();
        beginInsertRows(QModelIndex(), iDummyItemIndex, iDummyItemIndex);
        iList.append(new Data(this, NULL, false));
        endInsertRows();
        Q_EMIT countChanged();
        Q_EMIT hasDummyItemChanged();
        Q_EMIT dummyItemIndexChanged();
    } else if (!aHasDummyItem && hasDummyItem()) {
        remove(iDummyItemIndex);
    }
}

void BooksShelf::setEditMode(bool aEditMode)
{
    if (iEditMode != aEditMode) {
        iEditMode = aEditMode;
        HDEBUG(iEditMode);
        if (iSaveTimer->saveRequested()) {
            iSaveTimer->cancelSave();
            saveState();
        }
        setHasDummyItem(false);
        Q_EMIT editModeChanged();
    }
}

void BooksShelf::setDummyItemIndex(int aIndex)
{
    if (validIndex(aIndex) && hasDummyItem() && iDummyItemIndex != aIndex) {
        const int oldDummyItemIndex = iDummyItemIndex;
        iDummyItemIndex = aIndex;
        move(oldDummyItemIndex, aIndex);
        Q_EMIT dummyItemIndexChanged();
    }
}

BooksItem* BooksShelf::retain()
{
    if (iRef.load() >= 0) {
        iRef.ref();
    }
    return this;
}

void BooksShelf::release()
{
    if (iRef.load() >= 0 && !iRef.deref()) {
        delete this;
    }
}

QObject* BooksShelf::object()
{
    return this;
}

BooksShelf* BooksShelf::shelf()
{
    return this;
}

BooksBook* BooksShelf::book()
{
    return NULL;
}

QString BooksShelf::name() const
{
    return iName;
}

QString BooksShelf::fileName() const
{
    return iFileName;
}

int BooksShelf::count() const
{
    return iList.count();
}

QObject* BooksShelf::get(int aIndex) const
{
    if (validIndex(aIndex)) {
        return iList.at(aIndex)->object();
    }
    HWARN("invalid index" << aIndex);
    return NULL;
}

BooksBook* BooksShelf::bookAt(int aIndex) const
{
    if (validIndex(aIndex)) {
        return iList.at(aIndex)->book();
    }
    HWARN("invalid index" << aIndex);
    return NULL;
}

bool BooksShelf::drop(QObject* aItem)
{
    if (iDummyItemIndex >= 0) {
        BooksBook* book = qobject_cast<BooksBook*>(aItem);
        if (!book) {
            HWARN("unexpected drop object");
        } else if (itemIndex(book->fileName()) >= 0) {
            HWARN("duplicate file name");
            setHasDummyItem(false);
        } else {
            HDEBUG("copying" << book->name() << "to" << qPrintable(path()));
            book->setCopyingOut(true);
            // Dropped object replaces the dummy placeholder object
            QModelIndex index(createIndex(iDummyItemIndex, 0));
            Data* data = iList.at(iDummyItemIndex);
            HASSERT(!data->iItem);
            iDummyItemIndex = -1;
            // Don't connect signals since it's not our item
            data->setBook(book, true);
            // Start copying the data
            iTaskQueue->submit(new CopyTask(data, book));
            Q_EMIT hasDummyItemChanged();
            Q_EMIT dummyItemIndexChanged();
            Q_EMIT dataChanged(index, index);
            return true;
        }
    } else {
        HWARN("unexpected drop");
    }
    return false;
}

void BooksShelf::move(int aFrom, int aTo)
{
    if (aFrom != aTo) {
        if (validIndex(aFrom) && validIndex(aTo)) {
            HDEBUG(iList.at(aFrom)->name() << "from" << aFrom << "to" << aTo);
            int dest = (aTo < aFrom) ? aTo : (aTo+1);
            beginMoveRows(QModelIndex(), aFrom, aFrom, QModelIndex(), dest);
            iList.move(aFrom, aTo);
            queueStateSave();
            endMoveRows();
        } else {
            HWARN("invalid move" << aFrom << "->" << aTo);
        }
    }
}

void BooksShelf::remove(int aIndex)
{
    BooksBook* book = removeBook(aIndex);
    if (book) {
        Q_EMIT bookRemoved(book);
    }
}

BooksBook* BooksShelf::removeBook(int aIndex)
{
    if (validIndex(aIndex)) {
        HDEBUG(iList.at(aIndex)->name());
        beginRemoveRows(QModelIndex(), aIndex, aIndex);
        BooksBook* book = iList.at(aIndex)->book();
        if (book) {
            DeleteTask* task = new DeleteTask(book);
            iDeleteTasks.append(task);
            iTaskQueue->submit(task);
        }
        if (iDummyItemIndex == aIndex) {
            iDummyItemIndex = -1;
            Q_EMIT hasDummyItemChanged();
            Q_EMIT dummyItemIndexChanged();
        }
        delete iList.takeAt(aIndex);
        queueStateSave();
        Q_EMIT countChanged();
        endRemoveRows();
        return book;
    }
    return NULL;
}

void BooksShelf::removeAll()
{
    if (!iList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, iList.count()-1);
        const int n = iList.count();
        for (int i=0; i<n; i++) {
            BooksBook* book = iList.at(i)->book();
            if (book) {
                DeleteTask* task = new DeleteTask(book);
                iDeleteTasks.append(task);
                iTaskQueue->submit(task);
                Q_EMIT bookRemoved(book);
            }
        }
        if (iDummyItemIndex >= 0) {
            iDummyItemIndex = -1;
            Q_EMIT hasDummyItemChanged();
            Q_EMIT dummyItemIndexChanged();
        }
        qDeleteAll(iList);
        iList.clear();
        queueStateSave();
        Q_EMIT countChanged();
        endRemoveRows();
    }
}

bool BooksShelf::deleteRequested(int aIndex) const
{
    if (validIndex(aIndex)) {
        return iList.at(aIndex)->iDeleteRequested;
    } else {
        return false;
    }
}

void BooksShelf::setDeleteRequested(int aIndex, bool aValue)
{
    if (validIndex(aIndex)) {
        Data* data = iList.at(aIndex);
        if (data->iDeleteRequested != aValue) {
            if (aValue) {
                if (!data->copyingIn() && !data->copyingOut()) {
                    data->iDeleteRequested = true;
                    HDEBUG(aValue << data->name());
                    emitDataChangedSignal(aIndex, BooksItemDeleteRequested);
                }
            } else {
                data->iDeleteRequested = false;
                HDEBUG(aValue << data->name());
                emitDataChangedSignal(aIndex, BooksItemDeleteRequested);
            }
        }
    }
}

void BooksShelf::cancelAllDeleteRequests()
{
    for (int i=iList.count()-1; i>=0; i--) {
        setDeleteRequested(i, false);
    }
}

void BooksShelf::importBook(QObject* aBook)
{
    BooksBook* book = qobject_cast<BooksBook*>(aBook);
    if (!book) {
        HWARN("unexpected import object");
    } else if (itemIndex(book->fileName()) >= 0) {
        HWARN("duplicate file name" << book->fileName());
    } else {
        HDEBUG(qPrintable(book->path()) << "->" << qPrintable(iPath));
        beginInsertRows(QModelIndex(), 0, 0);
        Data* data = new Data(this, book->retain(), true);
        iList.insert(0, data);
        iTaskQueue->submit(new CopyTask(data, book));
        endInsertRows();
        Q_EMIT countChanged();
        saveState();
    }
}

void BooksShelf::emitDataChangedSignal(int aRow, int aRole)
{
    if (aRow >= 0) {
        QModelIndex index(createIndex(aRow, 0));
        QVector<int> roles;
        roles.append(aRole);
        Q_EMIT dataChanged(index, index, roles);
    }
}

void BooksShelf::onBookAccessibleChanged()
{
    int row = bookIndex(qobject_cast<BooksBook*>(sender()));
    if (row >= 0) {
        HDEBUG(iList.at(row)->name() << iList.at(row)->accessible());
        emitDataChangedSignal(row, BooksItemAccessible);
    }
}

void BooksShelf::onBookCopyingOutChanged()
{
    int row = bookIndex(qobject_cast<BooksBook*>(sender()));
    if (row >= 0) {
        HDEBUG(iList.at(row)->name() << iList.at(row)->copyingOut());
        emitDataChangedSignal(row, BooksItemCopyingOut);
    }
}

void BooksShelf::onBookMovedAway()
{
    BooksBook* book = qobject_cast<BooksBook*>(sender());
    HASSERT(book);
    if (book) {
        const int row = bookIndex(book);
        HDEBUG(book->title() << row);
        if (row >= 0) {
            HDEBUG(iList.at(row)->name());
            remove(row);
        }
    }
}

void BooksShelf::onCopyTaskPercentChanged()
{
    CopyTask* task = qobject_cast<CopyTask*>(sender());
    HASSERT(task);
    if (task) {
        HDEBUG(task->iDest << task->iCopyPercent);
        const int row = iList.indexOf(task->iData);
        emitDataChangedSignal(row, BooksItemCopyPercent);
    }
}

void BooksShelf::onCopyTaskDone()
{
    CopyTask* task = qobject_cast<CopyTask*>(sender());
    HASSERT(task);
    if (task) {
        HDEBUG(qPrintable(task->iSource) << "->" << qPrintable(task->iDest) <<
            "copy" << (task->iSuccess ? "done" : "FAILED"));

        Data* data = task->iData;
        const int row = iList.indexOf(data);
        HASSERT(row >= 0);

        BooksBook* copy = NULL;
        BooksBook* src = data->book();
        HASSERT(src);
        if (src) {
            src->retain();
            if (task->iSuccess) {
                ZLFile file(task->iDest.toStdString());
                shared_ptr<Book> book = Book::loadFromFile(file);
                if (!book.isNull()) {
                    copy = new BooksBook(iStorage, book);
                    copy->setLastPos(src->lastPos());
                    copy->setCoverImage(src->coverImage());
                    copy->requestCoverImage();
                } else {
                    HWARN("can't open copied book:" << file.path().c_str());
                }
            }
        }

        // Disassociate book data from the copy task
        data->iCopyTask = NULL;
        task->iData = NULL;
        task->release(this);

        // Notify the source shelf. This will actually remove the source file.
        if (copy) {
            Q_EMIT src->movedAway();
        }

        if (src) {
            src->setCopyingOut(false);
            src->release();
        }

        if (copy) {
            // We own this item now, connect the signals
            data->setBook(copy, false);
            Q_EMIT bookAdded(copy);

            // The entire row has changed
            QModelIndex index(createIndex(row, 0));
            Q_EMIT dataChanged(index, index);
        } else {
            removeBook(row);
        }
    }
}

void BooksShelf::onDeleteTaskDone()
{
    DeleteTask* task = qobject_cast<DeleteTask*>(sender());
    HASSERT(task);
    if (task) {
        task->release(this);
        HVERIFY(iDeleteTasks.removeOne(task));
    }
}

QHash<int,QByteArray> BooksShelf::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(BooksItemName, "name");
    roles.insert(BooksItemBook, "book");
    roles.insert(BooksItemShelf, "shelf");
    roles.insert(BooksItemAccessible, "accessible");
    roles.insert(BooksItemCopyingOut, "copyingOut");
    roles.insert(BooksItemCopyingIn, "copyingIn");
    roles.insert(BooksItemCopyPercent, "copyPercent");
    roles.insert(BooksItemDummy, "dummy");
    roles.insert(BooksItemDeleteRequested, "deleteRequested");
    return roles;
}

int BooksShelf::rowCount(const QModelIndex&) const
{
    return iList.count();
}

QVariant BooksShelf::data(const QModelIndex& aIndex, int aRole) const
{
    const int i = aIndex.row();
    if (validIndex(i)) {
        Data* data = iList.at(i);
        switch (aRole) {
        case BooksItemName: return data->name();
        case BooksItemBook: return QVariant::fromValue(data->book());
        case BooksItemShelf: return QVariant::fromValue(data->shelf());
        case BooksItemAccessible: return data->accessible();
        case BooksItemCopyingOut: return data->copyingOut();
        case BooksItemCopyingIn: return data->copyingIn();
        case BooksItemCopyPercent: return data->copyPercent();
        case BooksItemDummy: return QVariant::fromValue(!data->iItem);
        case BooksItemDeleteRequested: return data->iDeleteRequested;
        }
    }
    return QVariant();
}

#include "BooksShelf.moc"
