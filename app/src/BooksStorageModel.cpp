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

#include "BooksStorageModel.h"

#include "HarbourDebug.h"

enum BooksStorageRole {
    BooksStorageRoot = Qt::UserRole,
    BooksStorageDevice,
    BooksStorageRemovable,
    BooksStorageDeleteAllRequest
};

// ==========================================================================
// BooksStorageModel::Data
// ==========================================================================

class BooksStorageModel::Data {
public:
    Data(const BooksStorage& aStorage);

    QString device() const { return iStorage.device(); }
    QString root() const { return iStorage.root(); }
    bool isRemovable() const { return !iStorage.isInternal(); }

public:
    BooksStorage iStorage;
    bool iDeleteAllRequest;
};

BooksStorageModel::Data::Data(const BooksStorage& aStorage) :
    iStorage(aStorage),
    iDeleteAllRequest(false)
{
    // Pointers to these objects can be returned to QML
    QQmlEngine::setObjectOwnership(&iStorage, QQmlEngine::CppOwnership);
}

// ==========================================================================
// BooksStorageModel
// ==========================================================================

BooksStorageModel::BooksStorageModel(QObject* aParent) :
    QAbstractListModel(aParent)
{
#if QT_VERSION < 0x050000
    setRoleNames(roleNames());
#endif

    BooksStorageManager* mgr = BooksStorageManager::instance();
    QList<BooksStorage> list = mgr->storageList();
    const int n = list.count();
    for (int i=0; i<n; i++) iList.append(new Data(list.at(i)));
    iInternalStorage = list.at(0);
    QQmlEngine::setObjectOwnership(&iInternalStorage, QQmlEngine::CppOwnership);
    connect(mgr,
        SIGNAL(storageAdded(BooksStorage)),
        SLOT(onStorageAdded(BooksStorage)));
    connect(mgr,
        SIGNAL(storageRemoved(BooksStorage)),
        SLOT(onStorageRemoved(BooksStorage)));
}

BooksStorageModel::~BooksStorageModel()
{
    HDEBUG("destroyed");
}

int BooksStorageModel::count() const
{
    return iList.count();
}

int BooksStorageModel::deviceIndex(QString aDevice) const
{
    if (!aDevice.isEmpty()) {
        for (int i=iList.count()-1; i>=0; i--) {
            if (iList.at(i)->device() == aDevice) {
                return i;
            }
        }
    }
    return -1;
}

QString BooksStorageModel::deviceAt(int aIndex) const
{
    return validIndex(aIndex) ? iList.at(aIndex)->device() : QString();
}

void BooksStorageModel::setDeleteAllRequest(int aIndex, bool aValue)
{
    if (validIndex(aIndex)) {
        Data* data = iList.at(aIndex);
        if (data->iDeleteAllRequest != aValue) {
            data->iDeleteAllRequest = aValue;
            QModelIndex index(createIndex(aIndex, 0));
            QVector<int> roles;
            roles.append(BooksStorageDeleteAllRequest);
            HDEBUG(aValue << data->root());
            Q_EMIT dataChanged(index, index, roles);
        }
    }
}

void BooksStorageModel::cancelDeleteAllRequests()
{
    for (int i=iList.count()-1; i>=0; i--) {
        setDeleteAllRequest(i, false);
    }
}

QObject* BooksStorageModel::get(int aIndex) const
{
    if (validIndex(aIndex)) {
        return &(iList.at(aIndex)->iStorage);
    }
    return NULL;
}

QHash<int,QByteArray> BooksStorageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(BooksStorageRemovable, "removable");
    roles.insert(BooksStorageRoot, "root");
    roles.insert(BooksStorageDevice, "device");
    roles.insert(BooksStorageDeleteAllRequest, "deleteAllRequest");
    return roles;
}

int BooksStorageModel::rowCount(const QModelIndex&) const
{
    return iList.count();
}

QVariant BooksStorageModel::data(const QModelIndex& aIndex, int aRole) const
{
    const int i = aIndex.row();
    if (validIndex(i)) {
        Data* data = iList.at(i);
        switch (aRole) {
        case BooksStorageRoot: return data->root();
        case BooksStorageDevice: return data->device();
        case BooksStorageRemovable: return data->isRemovable();
        case BooksStorageDeleteAllRequest: return data->iDeleteAllRequest;
        }
    }
    return QVariant();
}

int BooksStorageModel::find(const BooksStorage& aStorage) const
{
    const int n = iList.count();
    for (int i=0; i<n; i++) {
        if (iList.at(i)->iStorage.equal(aStorage)) {
            return i;
        }
    }
    return -1;
}

void BooksStorageModel::onStorageAdded(BooksStorage aStorage)
{
    HDEBUG(aStorage.device() << "found");
    const int index = iList.count();
    beginInsertRows(QModelIndex(), index, index);
    iList.append(new Data(aStorage));
    endInsertRows();
    Q_EMIT countChanged();
    Q_EMIT newStorage(index);
}

void BooksStorageModel::onStorageRemoved(BooksStorage aStorage)
{
    int index = find(aStorage);
    if (index >=0) {
        beginRemoveRows(QModelIndex(), index, index);
        delete iList.takeAt(index);
        endRemoveRows();
        Q_EMIT countChanged();
    } else {
        HWARN("device not dfound on the list");
    }
}
