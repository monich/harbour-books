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

#include "BooksUtil.h"
#include "BooksDefs.h"

#include "HarbourBase45.h"
#include "HarbourDebug.h"
#include "HarbourTask.h"

#include "ZLDir.h"
#include "formats/FormatPlugin.h"

#include <queue>

#include <QCryptographicHash>

#include <sys/xattr.h>
#include <linux/xattr.h>
#include <errno.h>

#define DIGEST_XATTR    XATTR_USER_PREFIX BOOKS_APP_NAME ".md5-hash"
#define DIGEST_TYPE     (QCryptographicHash::Md5)
#define DIGEST_SIZE     (16)

// import Sailfish.Media 1.0; MediaKey{}
#define MEDIA_KEY_BASE45 \
    "YEDS9E5LE+347ECUVD+EDU7DDZ9AVCOCCZ96H46DZ9AVCMDCC$CNRF"

// import org.nemomobile.policy 1.0;Permissions{
//   autoRelease:true;applicationClass:"camera";
//   Resource{type:Resource.ScaleButton;optional:true}}
#define PERMISSIONS_BASE45 \
    "YEDS9E5LEN44$KE6*50$C+3ET3EXEDRZCS9EXVD+PC634Y$5JM75$CJ$DZQE EDF/" \
    "D+QF8%ED3E: CX CLQEOH76LE+ZCEECP9EOEDIEC EDC.DPVDZQEWF7GPCF$DVKEX" \
    "E4XIAVQE6%EKPCERF%FF*ZCXIAVQE6%EKPCO%5GPCTVD3I8MWE-3E5N7X9E ED..D" \
    "VUDKWE%$E+%F"

BooksUtil::BooksUtil(
    QObject* aParent) :
    QObject(aParent)
{
}

// Callback for qmlRegisterSingletonType<BooksUtil>
QObject*
BooksUtil::createSingleton(
    QQmlEngine*,
    QJSEngine*)
{
    return new BooksUtil();
}

QString
BooksUtil::mediaKeyQml()
{
    return HarbourBase45::fromBase45(QString::fromLatin1(MEDIA_KEY_BASE45));
}

QString
BooksUtil::permissionsQml()
{
    return HarbourBase45::fromBase45(QString::fromLatin1(PERMISSIONS_BASE45));
}

shared_ptr<Book>
BooksUtil::bookFromFile(
    const std::string& aPath)
{
    shared_ptr<Book> book;
    const ZLFile file(aPath);
    PluginCollection& plugins = PluginCollection::Instance();
    shared_ptr<FormatPlugin> plugin = plugins.plugin(file, false);
    if (!plugin.isNull()) {
        std::string error = plugin->tryOpen(file);
        if (error.empty()) {
            book = Book::loadFromFile(file);
        } else {
            HWARN(error.c_str());
        }
    } else if (file.isArchive()) {
        std::queue<std::string> archiveNames;
        std::vector<std::string> items;
        archiveNames.push(aPath);
        while (!archiveNames.empty() && book.isNull()) {
            shared_ptr<ZLDir> dir = ZLFile(archiveNames.front()).directory();
            archiveNames.pop();
            if (!dir.isNull()) {
                dir->collectFiles(items, true);
                for (std::vector<std::string>::const_iterator it = items.begin();
                     book.isNull() && it != items.end(); ++it) {
                    const std::string itemName = dir->itemPath(*it);
                    ZLFile subFile(itemName);
                    if (subFile.isArchive()) {
                        archiveNames.push(itemName);
                    } else {
                        book = Book::loadFromFile(subFile);
                    }
                }
                items.clear();
            }
        }
    }
    return book;
}

QByteArray
BooksUtil::computeFileHash(
    const QString aPath,
    const HarbourTask* aTask)
{
    QByteArray result;
    QFile file(aPath);
    if (file.open(QIODevice::ReadOnly)) {
        const qint64 size = file.size();
        uchar* map = file.map(0, size);
        if (map) {
            const char* ptr = (char*)map;
            qint64 bytesLeft = size;
            QCryptographicHash hash(DIGEST_TYPE);
            hash.reset();
            if (aTask) {
                while (!aTask->isCanceled() && bytesLeft > DIGEST_SIZE) {
                    hash.addData(ptr, DIGEST_SIZE);
                    bytesLeft -= DIGEST_SIZE;
                    ptr += DIGEST_SIZE;
                }
            } else {
                while (bytesLeft > (qint64)INT_MAX) {
                    hash.addData(ptr, INT_MAX);
                    bytesLeft -= INT_MAX;
                    ptr += INT_MAX;
                }
            }
            if (!aTask || !aTask->isCanceled()) {
                if (bytesLeft) {
                    hash.addData(ptr, bytesLeft);
                }
                result = hash.result();
                HASSERT(result.size() == DIGEST_SIZE);
                HDEBUG(qPrintable(aPath) << QString(result.toHex()));
            }
            file.unmap(map);
        } else {
            HWARN("error mapping" << qPrintable(aPath));
        }
        file.close();
    }
    return result;
}

QByteArray
BooksUtil::fileHashAttr(
    const QString aPath)
{
    QByteArray hash;
    const QByteArray fname(aPath.toLocal8Bit());
    char attr[DIGEST_SIZE];
    if (getxattr(fname, DIGEST_XATTR, attr, sizeof(attr)) == DIGEST_SIZE) {
        hash = QByteArray(attr, sizeof(attr));
        HDEBUG(qPrintable(aPath) << QString(hash.toHex()));
    }
    return hash;
}

bool
BooksUtil::setFileHashAttr(
    const QString aPath,
    const QByteArray aHash)
{
    if (aHash.size() == DIGEST_SIZE) {
        QByteArray fname(aPath.toLocal8Bit());
        if (setxattr(fname, DIGEST_XATTR, aHash, aHash.size(), 0) == 0) {
            return true;
        }
        HDEBUG("Failed to set " DIGEST_XATTR " xattr on" <<
            fname.constData() << ":" << strerror(errno));
    }
    return false;
}

QByteArray
BooksUtil::computeFileHashAndSetAttr(
    const QString aPath,
    const HarbourTask* aTask)
{
    QByteArray hash = computeFileHash(aPath, aTask);
    if (!hash.isEmpty()) {
        BooksUtil::setFileHashAttr(aPath, hash);
    }
    return hash;
}

bool
BooksUtil::isValidFileName(
    const QString aName)
{
    return !aName.isEmpty() &&
        aName != QStringLiteral(".") &&
        aName != QStringLiteral("..") &&
        !aName.contains('/');
}
