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
#include "BooksUtil.h"

#include "HarbourJson.h"
#include "HarbourDebug.h"

#include <errno.h>

enum BooksItemRole {
    BooksItemName = Qt::UserRole,
    BooksItemBook,
    BooksItemShelf,
    BooksItemAccessible,
    BooksItemCopyingOut,
    BooksItemCopyingIn,
    BooksItemCopyProgress,
    BooksItemDummy,
    BooksItemDeleteRequested
};

#define SHELF_STATE_FILE    BOOKS_STATE_FILE_SUFFIX
#define SHELF_STATE_ORDER   "order"

class BooksShelf::CopyTask : public BooksTask, BooksItem::CopyOperation
{
    Q_OBJECT

public:
    CopyTask(BooksShelf::Data* aDestData, BooksItem* aSrcItem);
    ~CopyTask();

    void performTask();
    QString srcPath() const;
    QString destPath() const;

    // BooksItem::CopyOperation
    virtual bool isCanceled() const;
    virtual void copyProgressChanged(int aProgress);

Q_SIGNALS:
    void copyProgressChanged();

public:
    BooksShelf::Data* iDestData;
    BooksItem* iSrcItem;
    int iCopyProgress;
    bool iSuccess;
};

// ==========================================================================
// BooksShelf::LoadTask
// ==========================================================================

class BooksShelf::LoadTask : public BooksTask
{
public:
    LoadTask(BooksStorage aStorage, QString aRelPath, QString aStateFile) :
        iStorage(aStorage), iRelativePath(aRelPath),
        iStateFilePath(aStateFile) {}
    ~LoadTask();

    void performTask();

    int findBook(QString aFileName) const;
    static int find(QFileInfoList aList, QString aFileName, int aStart);

public:
    BooksStorage iStorage;
    QString iRelativePath;
    QString iStateFilePath;
    QList<BooksItem*> iItems;
};

BooksShelf::LoadTask::~LoadTask()
{
    const int n = iItems.count();
    for (int i=0; i<n; i++) iItems.at(i)->release();
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
        const int n = iItems.count();
        for (int i=0; i<n; i++) {
            BooksItem* item = iItems.at(i);
            if (item->book() && item->fileName() == aFileName) {
                return i;
            }
        }
    }
    return -1;
}

void BooksShelf::LoadTask::performTask()
{
    if (!isCanceled()) {
        QString path(iStorage.fullPath(iRelativePath));
        HDEBUG("checking" << path);
        QDir dir(path);
        QFileInfoList list = dir.entryInfoList(QDir::Files |
            QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);

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
            const QFileInfo& info = list.at(i);
            QString path(info.filePath());
            if (info.isDir()) {
                HDEBUG("directory:" << qPrintable(path));
                QString folderPath(iRelativePath);
                if (!folderPath.isEmpty() && !folderPath.endsWith('/')) {
                    folderPath += '/';
                }
                folderPath += info.fileName();
                BooksShelf* newShelf = new BooksShelf(iStorage, folderPath);
                newShelf->moveToThread(thread());
                iItems.append(newShelf);
            } else {
                shared_ptr<Book> book = BooksUtil::bookFromFile(path);
                if (!book.isNull()) {
                    BooksBook* newBook = new BooksBook(iStorage,
                        iRelativePath, book);
                    newBook->moveToThread(thread());
                    iItems.append(newBook);
                    HDEBUG("[" << iItems.size() << "]" <<
                        qPrintable(newBook->fileName()) <<
                        newBook->title());
                } else {
                    HDEBUG("not a book:" << qPrintable(path));
                }
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
    QString path() { return iItem ? iItem->path() : QString(); }
    QObject* object() { return iItem ? iItem->object() : NULL; }
    BooksBook* book() { return iItem ? iItem->book() : NULL; }
    BooksShelf* shelf() { return iItem ? iItem->shelf() : NULL; }
    bool accessible() const { return !iCopyTask && iItem && iItem->accessible(); }
    bool isBook() const { return iItem && iItem->book(); }
    bool isShelf() const { return iItem && iItem->shelf(); }
    bool copyingOut();
    bool copyingIn() { return iCopyTask != NULL; }
    double copyProgress() { return iCopyTask ? (iCopyTask->iCopyProgress/
        ((double)PROGRESS_PRECISION)) : 0.0; }

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

inline bool BooksShelf::Data::copyingOut()
{
    BooksBook* bookItem = book();
    return bookItem && bookItem->copyingOut();
}

// ==========================================================================
// BooksShelf::CopyTask
// ==========================================================================

BooksShelf::CopyTask::CopyTask(BooksShelf::Data* aDestData, BooksItem* aSrcItem) :
    iDestData(aDestData),
    iSrcItem(aSrcItem->retain()),
    iCopyProgress(0),
    iSuccess(false)
{
    if (iDestData->iCopyTask) {
        iDestData->iCopyTask->release(iDestData->iShelf);
    }
    iDestData->iCopyTask = this;
    iDestData->iShelf->connect(this, SIGNAL(done()), SLOT(onCopyTaskDone()));
    iDestData->iShelf->connect(this, SIGNAL(copyProgressChanged()),
        SLOT(onCopyTaskProgressChanged()), Qt::QueuedConnection);
    HDEBUG(qPrintable(aSrcItem->path()) << "->" << destPath());
}

BooksShelf::CopyTask::~CopyTask()
{
    HASSERT(!iDestData);
    iSrcItem->release();
}

inline QString BooksShelf::CopyTask::srcPath() const
{
    return iSrcItem->path();
}

inline QString BooksShelf::CopyTask::destPath() const
{
    return QFileInfo(iDestData->iShelf->path(),
        iSrcItem->fileName()).absoluteFilePath();
}

void BooksShelf::CopyTask::performTask()
{
    iSuccess = iSrcItem->copyTo(QDir(iDestData->iShelf->path()), this);
}

bool BooksShelf::CopyTask::isCanceled() const
{
    return BooksTask::isCanceled();
}

void BooksShelf::CopyTask::copyProgressChanged(int aProgress)
{
    iCopyProgress = aProgress;
    Q_EMIT copyProgressChanged();
}

// ==========================================================================
// BooksShelf::DeleteTask
// ==========================================================================

class BooksShelf::DeleteTask : public BooksTask
{
    Q_OBJECT
public:
    DeleteTask(BooksItem* aItem);
    ~DeleteTask();
    void performTask();

public:
    BooksItem* iItem;
};

BooksShelf::DeleteTask::DeleteTask(BooksItem* aItem) :
    iItem(aItem)
{
    iItem->retain();
}

BooksShelf::DeleteTask::~DeleteTask()
{
    iItem->release();
}

void BooksShelf::DeleteTask::performTask()
{
    if (isCanceled()) {
        HDEBUG("cancelled" << iItem->fileName());
    } else {
        iItem->deleteFiles();
    }
}

// ==========================================================================
// BooksShelf::Counts
// ==========================================================================

class BooksShelf::Counts {
public:
    Counts(BooksShelf* aShelf);
    void count(BooksShelf* aShelf);
    void emitSignals(BooksShelf* aShelf);

    int iTotalCount;
    int iBookCount;
    int iShelfCount;
};

BooksShelf::Counts::Counts(BooksShelf* aShelf)
{
    count(aShelf);
}

void BooksShelf::Counts::count(BooksShelf* aShelf)
{
    iTotalCount = aShelf->iList.count(),
    iBookCount = 0;
    iShelfCount = 0;
    for (int i=0; i<iTotalCount; i++) {
        const Data* data = aShelf->iList.at(i);
        if (data->isBook()) {
            iBookCount++;
        } else if (data->isShelf()) {
            iShelfCount++;
        }
    }
}

void BooksShelf::Counts::emitSignals(BooksShelf* aShelf)
{
    const int oldTotalCount = iTotalCount;
    const int oldBookCount = iBookCount;
    const int oldShelfCount = iShelfCount;
    count(aShelf);
    if (oldBookCount != iBookCount) {
        Q_EMIT aShelf->bookCountChanged();
    }
    if (oldShelfCount != iShelfCount) {
        Q_EMIT aShelf->shelfCountChanged();
    }
    if (oldTotalCount != iTotalCount) {
        Q_EMIT aShelf->countChanged();
    }
}

// ==========================================================================
// BooksShelf
// ==========================================================================

BooksShelf::BooksShelf(QObject* aParent) :
    QAbstractListModel(aParent),
    iLoadBookList(true),
    iLoadTask(NULL),
    iDummyItemIndex(-1),
    iEditMode(false),
    iRef(-1),
    iSaveTimer(new BooksSaveTimer(this)),
    iTaskQueue(BooksTaskQueue::instance())
{
    init();
    connect(iSaveTimer, SIGNAL(save()), SLOT(saveState()));
}

BooksShelf::BooksShelf(BooksStorage aStorage, QString aRelativePath) :
    iLoadBookList(false),
    iLoadTask(NULL),
    iRelativePath(aRelativePath),
    iStorage(aStorage),
    iDummyItemIndex(-1),
    iEditMode(false),
    iRef(1),
    iSaveTimer(NULL),
    iTaskQueue(BooksTaskQueue::instance())
{
    init();
    // Refcounted BooksShelf objects are managed by C++ code
    // They also don't need to read the content of the directory -
    // only the objects allocated by QML do that.
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    updatePath();
}

BooksShelf::~BooksShelf()
{
    const int n = iDeleteTasks.count();
    for (int i=0; i<n; i++) iDeleteTasks.at(i)->release(this);
    if (iLoadTask) iLoadTask->release(this);
    if (iSaveTimer && iSaveTimer->saveRequested()) saveState();
    qDeleteAll(iList);
    HDEBUG("destroyed");
}

void BooksShelf::init()
{
#if QT_VERSION < 0x050000
    setRoleNames(roleNames());
#endif
    QQmlEngine::setObjectOwnership(&iStorage, QQmlEngine::CppOwnership);
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

void BooksShelf::setName(QString aName)
{
    if (iStorage.isValid() &&
        BooksUtil::isValidFileName(aName) &&
        iFileName != aName) {
        QString parentDir;
        const int lastSlash = iRelativePath.lastIndexOf('/');
        if (lastSlash > 0) {
            parentDir = iRelativePath.left(lastSlash);
        }

        QString newRelativePath;
        if (!parentDir.isEmpty()) {
            newRelativePath = parentDir;
            newRelativePath += '/';
        }
        newRelativePath += aName;

        QString oldPath = iStorage.fullPath(iRelativePath);
        QString newPath = iStorage.fullPath(newRelativePath);

        HDEBUG("renaming" << qPrintable(oldPath) << "->" << qPrintable(newPath));
        if (rename(qPrintable(oldPath), qPrintable(newPath)) == 0) {
            const QString oldFileName(iFileName);
            iRelativePath = newRelativePath;
            iPath = iStorage.fullPath(iRelativePath);
            updateFileName();
            Q_EMIT pathChanged();
            Q_EMIT relativePathChanged();

            // Since this directiry has been renamed, we need to update the
            // order of objects in the parent directory.
            QVariantMap state;
            const QString stateFile = stateFileName(parentDir);
            if (HarbourJson::load(stateFile, state)) {
                QVariantList order = state.value(SHELF_STATE_ORDER).toList();
                int i, n = order.count();
                for (i=0; i<n; i++) {
                    if (order.at(i).toString() == oldFileName) {
                        order[i] = iFileName;
                        state.insert(SHELF_STATE_ORDER, order);
                        if (HarbourJson::save(stateFile, state)) {
                            HDEBUG("wrote" << qPrintable(stateFile));
                        }
                        break;
                    }
                }
            }
        } else {
            HDEBUG(strerror(errno));
        }
    }
}

void BooksShelf::updatePath()
{
    BooksLoadingSignalBlocker block(this);
    const QString oldPath = iPath;
    iPath.clear();
    if (iStorage.isValid()) {
        iPath = iStorage.fullPath(iRelativePath);
    }
    updateFileName();
    if (oldPath != iPath) {
        const int oldDummyItemIndex = iDummyItemIndex;
        Counts counts(this);
        HDEBUG(iPath);
        // Clear the model
        if (!iList.isEmpty()) {
            beginRemoveRows(QModelIndex(), 0, iList.size()-1);
            while (!iList.isEmpty()) {
                Data* data = iList.takeLast();
                BooksBook* book = data->book();
                if (book) {
                    Q_EMIT bookRemoved(book);
                }
                delete data;
            }
            endRemoveRows();
        }
        iDummyItemIndex = -1;
        if (!iPath.isEmpty() && iLoadBookList) loadBookList();
        Q_EMIT pathChanged();
        if (oldDummyItemIndex != iDummyItemIndex) {
            Q_EMIT dummyItemIndexChanged();
            if (oldDummyItemIndex <0 || iDummyItemIndex < 0) {
                Q_EMIT hasDummyItemChanged();
            }
        }
        counts.emitSignals(this);
    }
}

void BooksShelf::updateFileName()
{
    const int slashPos = iRelativePath.lastIndexOf('/');
    const QString fileName = (slashPos >= 0) ?
        iRelativePath.right(iRelativePath.length() - slashPos - 1) :
        iRelativePath;
    if (iFileName != fileName) {
        iFileName = fileName;
        Q_EMIT nameChanged();
    }
}

void BooksShelf::onLoadTaskDone()
{
    HASSERT(iLoadTask);
    HASSERT(iLoadTask == sender());
    if (iLoadTask && iLoadTask == sender()) {
        BooksLoadingSignalBlocker block(this);
        const int oldSize = iList.size();
        const int newSize = iLoadTask->iItems.size();
        HASSERT(iList.isEmpty());
        if (newSize > 0) {
            Counts counts(this);
            beginInsertRows(QModelIndex(), oldSize, oldSize + newSize - 1);
            for (int i=0; i<newSize; i++) {
                BooksItem* item = iLoadTask->iItems.at(i);
                BooksBook* book = item->book();
                if (book) {
                    Q_EMIT bookAdded(book);
                }
                iList.append(new Data(this, item->retain(), false));
            }
            endInsertRows();
            counts.emitSignals(this);
        }
        iLoadTask->release(this);
        iLoadTask = NULL;
    }
}

void BooksShelf::loadBookList()
{
    BooksLoadingSignalBlocker block(this);
    if (iLoadTask) iLoadTask->release(this);
    if (iPath.isEmpty()) {
        iLoadTask = NULL;
    } else {
        HDEBUG(iPath);
        iLoadTask = new LoadTask(iStorage, iRelativePath, stateFileName());
        connect(iLoadTask, SIGNAL(done()), SLOT(onLoadTaskDone()));
        iTaskQueue->submit(iLoadTask);
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
        HDEBUG("wrote" << qPrintable(stateFileName()));
    }
}

void BooksShelf::queueStateSave()
{
    if (iEditMode && iSaveTimer) {
        iSaveTimer->requestSave();
    }
}

QString BooksShelf::stateFileName(QString aRelativePath) const
{
    if (iStorage.isValid()) {
        QString path(iStorage.configDir().path());
        if (!aRelativePath.isEmpty()) {
            path += "/";
            path += aRelativePath;
        }
        path += "/" SHELF_STATE_FILE;
        return path;
    } else {
        return QString();
    }
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
        if (iSaveTimer && iSaveTimer->saveRequested()) {
            iSaveTimer->saveNow();
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
    return iFileName;
}

QString BooksShelf::fileName() const
{
    return iFileName;
}

QString BooksShelf::path() const
{
    return iPath;
}

bool BooksShelf::accessible() const
{
    return true;
}

int BooksShelf::count() const
{
    return iList.count();
}

int BooksShelf::bookCount() const
{
    int n=0, total = iList.count();
    for(int i=0; i<total; i++) {
        if (iList.at(i)->book()) n++;
    }
    return n;
}

int BooksShelf::shelfCount() const
{
    int n=0, total = iList.count();
    for(int i=0; i<total; i++) {
        if (iList.at(i)->shelf()) n++;
    }
    return n;
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

void BooksShelf::submitDeleteTask(int aIndex)
{
    BooksItem* item = iList.at(aIndex)->iItem;
    if (item) {
        DeleteTask* task = new DeleteTask(item);
        iDeleteTasks.append(task);
        iTaskQueue->submit(task);
        BooksBook* book = item->book();
        if (book) {
            book->cancelCoverRequest();
            Q_EMIT bookRemoved(book);
        }
    }
}

void BooksShelf::remove(int aIndex)
{
    if (validIndex(aIndex)) {
        Counts counts(this);
        HDEBUG(iList.at(aIndex)->name());
        beginRemoveRows(QModelIndex(), aIndex, aIndex);
        submitDeleteTask(aIndex);
        if (iDummyItemIndex == aIndex) {
            iDummyItemIndex = -1;
            Q_EMIT hasDummyItemChanged();
            Q_EMIT dummyItemIndexChanged();
        }
        delete iList.takeAt(aIndex);
        queueStateSave();
        counts.emitSignals(this);
        endRemoveRows();
    }
}

void BooksShelf::removeAll()
{
    if (!iList.isEmpty()) {
        Counts counts(this);
        beginRemoveRows(QModelIndex(), 0, iList.count()-1);
        const int n = iList.count();
        for (int i=0; i<n; i++) {
            submitDeleteTask(i);
        }
        if (iDummyItemIndex >= 0) {
            iDummyItemIndex = -1;
            Q_EMIT hasDummyItemChanged();
            Q_EMIT dummyItemIndexChanged();
        }
        qDeleteAll(iList);
        iList.clear();
        queueStateSave();
        counts.emitSignals(this);
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
        Counts counts(this);
        Data* data = new Data(this, book->retain(), true);
        iList.insert(0, data);
        iTaskQueue->submit(new CopyTask(data, book));
        counts.emitSignals(this);
        endInsertRows();
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
            remove(row);
        }
    }
}

void BooksShelf::onCopyTaskProgressChanged()
{
    CopyTask* task = qobject_cast<CopyTask*>(sender());
    HASSERT(task);
    if (task) {
        HDEBUG(task->destPath() << task->iCopyProgress);
        const int row = iList.indexOf(task->iDestData);
        emitDataChangedSignal(row, BooksItemCopyProgress);
    }
}

void BooksShelf::onCopyTaskDone()
{
    CopyTask* task = qobject_cast<CopyTask*>(sender());
    HASSERT(task);
    if (task) {
        QString dest = task->destPath();
        HDEBUG(qPrintable(task->srcPath()) << "->" << qPrintable(dest) <<
            "copy" << (task->iSuccess ? "done" : "FAILED"));

        Data* data = task->iDestData;
        const int row = iList.indexOf(data);
        HASSERT(row >= 0);

        BooksBook* copy = NULL;
        BooksBook* src = data->book();
        HASSERT(src);
        if (src) {
            src->retain();
            if (task->iSuccess) {
                shared_ptr<Book> book = BooksUtil::bookFromFile(dest);
                if (!book.isNull()) {
                    copy = new BooksBook(iStorage, iRelativePath, book);
                    copy->setLastPos(src->lastPos());
                    copy->setCoverImage(src->coverImage());
                    copy->requestCoverImage();
                } else {
                    HWARN("can't open copied book" << qPrintable(dest));
                }
            }
        }

        // Disassociate book data from the copy task
        data->iCopyTask = NULL;
        task->iDestData = NULL;
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
            remove(row);
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

void BooksShelf::deleteFiles()
{
    if (iStorage.isValid()) {
        QString path(iStorage.fullPath(iRelativePath));
        HDEBUG("removing" << path);
        if (!QDir(path).removeRecursively()) {
            HWARN("some content couldn't be deleted under" << path);
        }
        path = iStorage.configDir().path() + "/" + iRelativePath;
        HDEBUG("removing" << path);
        if (!QDir(path).removeRecursively()) {
            HWARN("some content couldn't be deleted under" << path);
        }
    }
}

bool BooksShelf::copyTo(QDir aDestDir, CopyOperation* aOperation)
{
    HWARN("copying folders is not implemented!!");
    return false;
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
    roles.insert(BooksItemCopyProgress, "copyProgress");
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
        case BooksItemCopyProgress: return data->copyProgress();
        case BooksItemDummy: return QVariant::fromValue(!data->iItem);
        case BooksItemDeleteRequested: return data->iDeleteRequested;
        }
    }
    return QVariant();
}

#include "BooksShelf.moc"
