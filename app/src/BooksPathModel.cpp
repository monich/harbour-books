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

#include "BooksPathModel.h"

#include "HarbourDebug.h"

#include <errno.h>

enum BooksPathModelRole {
    BooksPathModelName = Qt::UserRole,
    BooksPathModelPath
};

BooksPathModel::BooksPathModel(QObject* aParent) :
    QAbstractListModel(aParent)
{
#if QT_VERSION < 0x050000
    setRoleNames(roleNames());
#endif
}

void BooksPathModel::setPath(QString aPath)
{
    HDEBUG(aPath);
    if (iPath != aPath) {
        iPath = aPath;

        QStringList newNames = aPath.split('/', QString::SkipEmptyParts);
        const int oldSize = iList.size();
        const int newSize = newNames.size();

        int i;
        QString path;
        QStringList pathList;
        for (i=0; i<newSize; i++) {
            if (!path.isEmpty()) path += "/";
            path += newNames.at(i);
            pathList.append(path);
        }

        if (oldSize < newSize) {
            beginInsertRows(QModelIndex(), oldSize, newSize-1);
            for (int i=oldSize; i<newSize; i++) {
                Data data(pathList.at(i), newNames.at(i));
                iList.append(data);
            }
            endInsertRows();
            Q_EMIT countChanged();
        } else if (oldSize > newSize) {
            beginRemoveRows(QModelIndex(), newSize, oldSize-1);
            do iList.removeLast(); while (iList.size() > newSize);
            endRemoveRows();
            Q_EMIT countChanged();
        }
        for (i=0; i<newSize; i++) {
            bool changed = false;
            if (iList.at(i).iName != newNames.at(i)) {
                iList[i].iName = newNames.at(i);
                changed = true;
            }
            if (iList.at(i).iPath != pathList.at(i)) {
                iList[i].iPath = pathList.at(i);
                changed = true;
            }
            if (changed) {
                QModelIndex index = createIndex(i, 0);
                Q_EMIT dataChanged(index, index);
            }
        }
        Q_EMIT pathChanged();
    }
}

QHash<int,QByteArray> BooksPathModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(BooksPathModelName, "name");
    roles.insert(BooksPathModelPath, "path");
    return roles;
}

int BooksPathModel::rowCount(const QModelIndex&) const
{
    return iList.count();
}

QVariant BooksPathModel::data(const QModelIndex& aIndex, int aRole) const
{
    const int i = aIndex.row();
    if (validIndex(i)) {
        switch (aRole) {
        case BooksPathModelName: return iList.at(i).iName;
        case BooksPathModelPath: return iList.at(i).iPath;
        }
    }
    return QVariant();
}
