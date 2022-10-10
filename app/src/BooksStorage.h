/*
 * Copyright (C) 2015-2022 Jolla Ltd.
 * Copyright (C) 2015-2022 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

#ifndef BOOKS_STORAGE_H
#define BOOKS_STORAGE_H

#include <QObject>
#include <QList>
#include <QDir>

class BooksSettings;
class BooksStorageManager;

class BooksStorage: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString root READ root CONSTANT)
    Q_PROPERTY(bool internal READ isInternal CONSTANT)
    Q_PROPERTY(bool valid READ isValid CONSTANT)

    enum Type {
        InternalStorage,
        RemovableStorage,
        TmpStorage
    };

public:
    BooksStorage();
    BooksStorage(const BooksStorage&);
    ~BooksStorage();

    QString device() const;
    QDir booksDir() const;
    QDir configDir() const;
    QString label() const { return booksDir().dirName(); }
    QString root() const { return booksDir().path(); }
    QString fullConfigPath(const QString) const;
    QString fullPath(const QString) const;

    bool isValid() const { return iPrivate != NULL; }
    bool isInternal() const;
    bool isPresent() const;
    bool equal(const BooksStorage&) const;
    void set(const BooksStorage&);

    BooksStorage& operator = (const BooksStorage& aStorage)
        { set(aStorage); return *this; }
    bool operator == (const BooksStorage& aStorage) const
        { return equal(aStorage); }

    static BooksStorage tmpStorage();

Q_SIGNALS:
    void removed();

private:
    friend class BooksStorageManager;
    BooksStorage(const QString, const QString, const QString, Type);
    void connectNotify(const QMetaMethod&) Q_DECL_OVERRIDE;

private:
    class Private;
    Private* iPrivate;
    bool iPassThrough;
};

class BooksStorageManager: public QObject
{
    Q_OBJECT

public:
    static BooksStorageManager* instance();
    static void deleteInstance();
    ~BooksStorageManager();

    int count() const;
    QList<BooksStorage> storageList() const;
    BooksStorage internalStorage() const;
    BooksStorage storageForDevice(const QString) const;
    BooksStorage storageForPath(const QString, QString* aRelPath = NULL) const;

Q_SIGNALS:
    void storageAdded(BooksStorage);
    void storageRemoved(BooksStorage);
    void storageReplaced(BooksStorage, BooksStorage);

private:
    BooksStorageManager();

private:
    class Private;
    Private* iPrivate;
};

Q_DECLARE_METATYPE(BooksStorage)

#endif // BOOKS_STORAGE_H
