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

#ifndef BOOKS_STORAGE_H
#define BOOKS_STORAGE_H

#include <QObject>
#include <QList>
#include <QDir>

class BooksStorageManager;

class BooksStorage: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString root READ root CONSTANT)
    Q_PROPERTY(bool internal READ isInternal CONSTANT)
    Q_PROPERTY(bool valid READ isValid CONSTANT)

public:
    BooksStorage();
    BooksStorage(const BooksStorage& aStorage);
    ~BooksStorage();

    QString device() const;
    QDir booksDir() const;
    QDir configDir() const;
    QString label() const { return booksDir().dirName(); }
    QString root() const { return booksDir().path(); }
    QString fullConfigPath(QString aRelativePath) const;
    QString fullPath(QString aRelativePath) const;

    bool isValid() const { return iPrivate != NULL; }
    bool isInternal() const;
    bool isPresent() const;
    bool equal(const BooksStorage& aStorage) const;

    BooksStorage& operator = (const BooksStorage& aStorage);
    bool operator == (const BooksStorage& aStorage) const
        { return equal(aStorage); }

Q_SIGNALS:
    void removed();

private:
    friend class BooksStorageManager;
    BooksStorage(QString, QDir, bool);
    void connectNotify(const QMetaMethod& aSignal);

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
    BooksStorage storageForDevice(QString aDevice) const;
    BooksStorage storageForPath(QString aPath, QString* aRelPath = NULL) const;

Q_SIGNALS:
    void storageAdded(BooksStorage aStorage);
    void storageRemoved(BooksStorage aStorage);

private:
    BooksStorageManager();
    bool scanMounts();

private Q_SLOTS:
    void onDeviceEvent(int);
    void onScanMounts();

private:
    class Private;
    Private* iPrivate;
};

Q_DECLARE_METATYPE(BooksStorage)

#endif // BOOKS_STORAGE_H
