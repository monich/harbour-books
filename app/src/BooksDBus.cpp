/*
 * Copyright (C) 2021 Jolla Ltd.
 * Copyright (C) 2021 Slava Monich <slava.monich@jolla.com>
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

#include "BooksDefs.h"
#include "BooksDBus.h"

#include "HarbourDebug.h"

#include <QDBusConnection>

// ==========================================================================
// BooksDBus::Adaptor
// ==========================================================================

class BooksDBus::Adaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", BOOKS_DBUS_INTERFACE)

public:
    Adaptor(QObject* aParent);

    void open(QString aPathOrUrl);

public Q_SLOTS:
    void Open(QString path);
    void Open(QStringList args);
};

BooksDBus::Adaptor::Adaptor(QObject* aParent) :
    QDBusAbstractAdaptor(aParent)
{
}

void BooksDBus::Adaptor::open(QString aPathOrUrl)
{
    BooksDBus* publicObject = qobject_cast<BooksDBus*>(parent());
    if (!aPathOrUrl.isEmpty()) {
        static const QString fileUrlPrefix("file://");
        if (aPathOrUrl.startsWith(fileUrlPrefix)) {
            const QString path(aPathOrUrl.right(aPathOrUrl.length() -
                fileUrlPrefix.length()));
            if (!path.isEmpty()) {
                HDEBUG(qPrintable(path));
                Q_EMIT publicObject->openBook(path);
            }
        } else {
            // Assume it's a path
            HDEBUG(qPrintable(aPathOrUrl));
            Q_EMIT publicObject->openBook(aPathOrUrl);
        }
    }
    Q_EMIT publicObject->activate();
}

void BooksDBus::Adaptor::Open(QString aArg)
{
    HDEBUG(aArg);
    open(aArg);
}

void BooksDBus::Adaptor::Open(QStringList aArgs)
{
    HDEBUG(aArgs);
    open(aArgs.isEmpty() ? QString() : aArgs.at(0));
}

// ==========================================================================
// BooksDBus
// ==========================================================================
BooksDBus::BooksDBus(QObject* aParent) :
    QObject(aParent),
    iAdaptor(new Adaptor(this))
{
}

BooksDBus* BooksDBus::create(QObject* aParent)
{
    BooksDBus* handler = new BooksDBus(aParent);
    QDBusConnection sessionBus(QDBusConnection::sessionBus());
    if (sessionBus.registerObject("/", handler) &&
        sessionBus.registerService(BOOKS_DBUS_SERVICE)) {
        HDEBUG("Registered D-Bus handler");
        return handler;
    } else {
        HDEBUG("Failed to registered D-Bus handler");
        delete handler;
        return Q_NULLPTR;
    }
}

#include "BooksDBus.moc"
