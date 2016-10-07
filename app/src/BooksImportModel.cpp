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

#include "BooksImportModel.h"
#include "BooksStorage.h"
#include "BooksTask.h"
#include "BooksUtil.h"
#include "BooksDefs.h"

#include "HarbourDebug.h"

#include <QDir>
#include <QCryptographicHash>

#include <sys/xattr.h>
#include <linux/xattr.h>
#include <errno.h>

#define DIGEST_XATTR    XATTR_USER_PREFIX BOOKS_APP_NAME ".md5-hash"
#define DIGEST_TYPE     (QCryptographicHash::Md5)
#define DIGEST_SIZE     (16)

enum BooksImportRole {
    BooksImportRoleTitle = Qt::UserRole,
    BooksImportRoleBook,
    BooksImportRolePath,
    BooksImportRoleFileName,
    BooksImportRoleSelected
};

// ==========================================================================
// BooksImportModel::Data
// ==========================================================================

class BooksImportModel::Data {
public:
    Data(BooksBook* iBook);
    ~Data();

    QString title() { return iBook->title(); }
    QString path() { return iBook->path(); }
    QString fileName() { return iBook->fileName(); }

public:
    BooksBook* iBook;
    bool iSelected;
};

BooksImportModel::Data::Data(BooksBook* aBook) :
    iBook(aBook),
    iSelected(false)
{
    iBook->retain();
}

BooksImportModel::Data::~Data()
{
    iBook->release();
}

// ==========================================================================
// BooksImportModel::Task
// ==========================================================================

class BooksImportModel::Task : public BooksTask
{
    Q_OBJECT

public:
    Task(QString aDest);
    ~Task();

    void performTask();
    void scanDir(QDir aDir);
    bool isDuplicate(QString aPath, QFileInfoList aFileList);
    QByteArray calculateFileHash(QString aPath);
    QByteArray getFileHash(QString aPath);

Q_SIGNALS:
    void bookFound(BooksBook* aBook);
    void progress(int aCount);

public:
    QList<BooksBook*> iBooks;
    QHash<QString,QByteArray> iFileHash;
    QHash<QByteArray,QString> iHashFile;
    QFileInfoList iDestFiles;
    QFileInfoList iSrcFiles;
    QString iDestDir;
    qint64 iBufSize;
    char* iBuf;
    int iProgress;
};

BooksImportModel::Task::Task(QString aDest) :
    iDestDir(aDest), iBufSize(0x1000), iBuf(NULL), iProgress(0)
{
}

BooksImportModel::Task::~Task()
{
    const int n = iBooks.count();
    for (int i=0; i<n; i++) iBooks.at(i)->release();
    delete [] iBuf;
}

QByteArray BooksImportModel::Task::calculateFileHash(QString aPath)
{
    QByteArray result;
    QFile file(aPath);
    if (file.open(QIODevice::ReadOnly)) {
        qint64 len = 0;
        QCryptographicHash hash(DIGEST_TYPE);
        hash.reset();
        if (!iBuf) iBuf = new char[iBufSize];
        while (!isCanceled() && (len = file.read(iBuf, iBufSize)) > 0) {
            hash.addData(iBuf, len);
        }
        if (len == 0) {
            if (!isCanceled()) {
                result = hash.result();
                HASSERT(result.size() == DIGEST_SIZE);
                HDEBUG(qPrintable(aPath) << QString(result.toHex()));
            }
        } else {
            HWARN("error reading" << qPrintable(aPath));
        }
        file.close();
    }
    return result;
}

QByteArray BooksImportModel::Task::getFileHash(QString aPath)
{
    if (iFileHash.contains(aPath)) {
        return iFileHash.value(aPath);
    } else {
        QByteArray hash;
        char attr[DIGEST_SIZE];
        QByteArray fname = aPath.toLocal8Bit();
        if (getxattr(fname, DIGEST_XATTR, attr, sizeof(attr)) == DIGEST_SIZE) {
            hash = QByteArray(attr, sizeof(attr));
            HDEBUG(qPrintable(aPath) << QString(hash.toHex()));
        } else {
            hash = calculateFileHash(aPath);
            if (hash.size() == DIGEST_SIZE &&
                setxattr(fname, DIGEST_XATTR, hash, hash.size(), 0)) {
                HDEBUG("Failed to set " DIGEST_XATTR " xattr on" <<
                    fname.constData() << ":" << strerror(errno));
            }
        }
        if (hash.size() == DIGEST_SIZE) {
            iFileHash.insert(aPath, hash);
            iHashFile.insert(hash, aPath);
        }
        return hash;
    }
}

bool BooksImportModel::Task::isDuplicate(QString aPath, QFileInfoList aList)
{
    const int n = aList.count();
    if (n > 0) {
        QFileInfo file(aPath);
        QByteArray fileHash;
        for (int i=0; i<n && !isCanceled(); i++) {
            QFileInfo other = aList.at(i);
            if (other.size() == file.size()) {
                QByteArray otherHash(getFileHash(other.filePath()));
                if (!otherHash.isEmpty() && !isCanceled()) {
                    if (fileHash.isEmpty()) fileHash = getFileHash(aPath);
                    if (fileHash == otherHash) {
                        HDEBUG(qPrintable(aPath) << "and" <<
                            qPrintable(other.filePath()) <<
                            "are identical");
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void BooksImportModel::Task::performTask()
{
    if (!isCanceled()) {
        if (!iDestDir.isEmpty()) {
            iDestFiles = QDir(iDestDir).entryInfoList(QDir::Files);
        }
        scanDir(QDir(QDir::homePath() + "/Downloads"));
        scanDir(QDir(QDir::homePath() + "/android_storage/Download"));
    }
}

void BooksImportModel::Task::scanDir(QDir aDir)
{
    // Files first
    if (!isCanceled()) {
        HDEBUG("checking" << aDir.canonicalPath());
        BooksStorage dummy;
        QFileInfoList fileList = aDir.entryInfoList(QDir::Files |
            QDir::Readable, QDir::Time);
        const int n = fileList.count();
        for (int i=0; i<n && !isCanceled(); i++) {
            QFileInfo fileInfo(fileList.at(i));
            QString filePath(fileInfo.canonicalFilePath());
            std::string path(filePath.toStdString());
            shared_ptr<Book> book = BooksUtil::bookFromFile(path);
            if (!book.isNull()) {
                if (!isDuplicate(filePath, iDestFiles) &&
                    !isDuplicate(filePath, iSrcFiles)) {
                    BooksBook* newBook = new BooksBook(dummy, QString(), book);
                    newBook->moveToThread(thread());
                    iBooks.append(newBook);
                    iSrcFiles.append(fileInfo);
                    HDEBUG("found" << path.c_str() << newBook->title());
                    Q_EMIT bookFound(newBook);
                }
            } else {
                HDEBUG("not a book:" << path.c_str());
            }
            iProgress++;
            Q_EMIT progress(iProgress);
        }
    }

    // Then directories
    if (!isCanceled()) {
        QFileInfoList dirList = aDir.entryInfoList(QDir::Dirs |
            QDir::NoDotAndDotDot | QDir::Readable, QDir::Time);
        const int n = dirList.count();
        for (int i=0; i<n && !isCanceled(); i++) {
            QString dirPath(dirList.at(i).canonicalFilePath());
            HDEBUG(dirPath);
            if (!dirPath.isEmpty()) {
                scanDir(QDir(dirPath));
            }
        }
    }
}

// ==========================================================================
// BooksImportModel
// ==========================================================================

BooksImportModel::BooksImportModel(QObject* aParent) :
    QAbstractListModel(aParent),
    iProgress(0),
    iSelectedCount(0),
    iAutoRefresh(false),
    iTaskQueue(BooksTaskQueue::defaultQueue()),
    iTask(NULL)
{
    iSelectedRole.append(BooksImportRoleSelected);
    HDEBUG("created");
#if QT_VERSION < 0x050000
    setRoleNames(roleNames());
#endif
}

BooksImportModel::~BooksImportModel()
{
    HDEBUG("destroyed");
    qDeleteAll(iList);
    if (iTask) iTask->release(this);
}

void BooksImportModel::setDestination(QString aDestination)
{
    if (iDestination != aDestination) {
        iDestination = aDestination;
        HDEBUG(aDestination);
        Q_EMIT destinationChanged();
        if (iAutoRefresh) {
            if (iTask) {
                iTask->release(this);
                iTask = NULL;
            }
            refresh();
        }
    }
}

void BooksImportModel::refresh()
{
    iAutoRefresh = true;
    if (!iTask) {
        HDEBUG("refreshing the model");

        if (!iList.isEmpty()) {
            beginResetModel();
            qDeleteAll(iList);
            iList.clear();
            endInsertRows();
            Q_EMIT countChanged();
        }

        if (iProgress) {
            iProgress = 0;
            Q_EMIT progressChanged();
        }

        iTask = new Task(iDestination);
        connect(iTask, SIGNAL(bookFound(BooksBook*)),
            SLOT(onBookFound(BooksBook*)), Qt::QueuedConnection);
        connect(iTask, SIGNAL(done()), SLOT(onTaskDone()));
        connect(iTask, SIGNAL(progress(int)), SLOT(onScanProgress(int)),
            Qt::QueuedConnection);
        iTaskQueue->submit(iTask);
        Q_EMIT busyChanged();
    }
}

void BooksImportModel::setSelected(int aIndex, bool aSelected)
{
    if (validIndex(aIndex)) {
        Data* data = iList.at(aIndex);
        if (data->iSelected != aSelected) {
            HDEBUG(data->path() << aSelected);
            if (data->iSelected) iSelectedCount--;
            if (aSelected) iSelectedCount++;
            data->iSelected = aSelected;

            QModelIndex index(createIndex(aIndex, 0));
            Q_EMIT dataChanged(index, index, iSelectedRole);
            Q_EMIT selectedCountChanged();
        }
    }
}

QObject* BooksImportModel::selectedBook(int aIndex)
{
    const int n = iList.count();
    for (int i=0, k=0; i<n; i++) {
        Data* data = iList.at(i);
        if (data->iSelected) {
            if (k == aIndex) {
                return data->iBook;
            }
            k++;
        }
    }
    return NULL;
}

void BooksImportModel::onScanProgress(int aProgress)
{
    if (iTask && iTask == sender()) {
        iProgress = aProgress;
        Q_EMIT progressChanged();
    }
}

void BooksImportModel::onBookFound(BooksBook* aBook)
{
    if (iTask && iTask == sender()) {
        // When we find the first book, we add two items. The second item
        // is the "virtual" that will stay at the end of the list and will
        // be removed by onTaskDone() after scanning is finished. The idea
        // is to show the busy indicator at the end of the list (that's how
        // QML represents the dummy item) while we keep on scanning.
        const int n1 = iList.count();
        beginInsertRows(QModelIndex(), n1, n1 ? n1 : 1);
        iList.append(new Data(aBook));
        endInsertRows();
        Q_EMIT countChanged();
    }
}

void BooksImportModel::onTaskDone()
{
    HASSERT(iTask);
    HASSERT(iTask == sender());
    iTask->release(this);
    iTask = NULL;
    if (iList.count() > 0) {
        // Remove the "virtual" item at the end of the list
        beginRemoveRows(QModelIndex(),iList.count(), iList.count());
        endRemoveRows();
    }
    Q_EMIT busyChanged();
}

QHash<int,QByteArray> BooksImportModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(BooksImportRoleTitle, "title");
    roles.insert(BooksImportRoleBook, "book");
    roles.insert(BooksImportRolePath, "path");
    roles.insert(BooksImportRoleFileName, "fileName");
    roles.insert(BooksImportRoleSelected, "selected");
    return roles;
}

int BooksImportModel::rowCount(const QModelIndex&) const
{
    return iTask ? (iList.count() + 1) : iList.count();
}

QVariant BooksImportModel::data(const QModelIndex& aIndex, int aRole) const
{
    const int i = aIndex.row();
    if (validIndex(i)) {
        Data* data = iList.at(i);
        switch (aRole) {
        case BooksImportRoleTitle: return data->title();
        case BooksImportRoleBook: return QVariant::fromValue(data->iBook);
        case BooksImportRolePath: return data->path();
        case BooksImportRoleFileName: return data->fileName();
        case BooksImportRoleSelected: return data->iSelected;
        }
    } else if (i == iList.count()) {
        switch (aRole) {
        case BooksImportRoleTitle:
        case BooksImportRolePath:
        case BooksImportRoleFileName: return QString();
        case BooksImportRoleBook: return QVariant::fromValue((QObject*)NULL);
        case BooksImportRoleSelected: return false;
        }
    }
    return QVariant();
}

#include "BooksImportModel.moc"
