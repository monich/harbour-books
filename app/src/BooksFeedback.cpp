/*
 * Copyright (C) 2017-2020 Jolla Ltd.
 * Copyright (C) 2017-2020 Slava Monich <slava.monich@jolla.com>
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

#include "BooksFeedback.h"
#include "HarbourDebug.h"

#include <QDBusConnection>
#include <QDBusAbstractInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#define NGFD_CONNECTION         QDBusConnection::systemBus()
#define NGFD_SERVICE            "com.nokia.NonGraphicFeedback1.Backend"
#define NGFD_PATH               "/com/nokia/NonGraphicFeedback1"
#define NGFD_INTERFACE          "com.nokia.NonGraphicFeedback1"

#define NGFD_STATUS_FAILED      (0)
#define NGFD_STATUS_COMPLETED   (1)
#define NGFD_STATUS_PLAYING     (2)
#define NGFD_STATUS_PAUSED      (3)

class BooksFeedback::Private : QDBusAbstractInterface {
    Q_OBJECT

public:
    Private(QObject* aParent);

    bool start(QString aEvent);
    void stop();
    void stop(uint aEventId);

public Q_SLOTS:
    void onPlayFinished(QDBusPendingCallWatcher* aCall);
    void onStatusChanged(uint aId, uint aStatus);

Q_SIGNALS:
    void Status(uint aId, uint aStatus);

public:
    uint iEventId;
    bool iPlaying;
};

BooksFeedback::Private::Private(QObject* aParent) :
    QDBusAbstractInterface(NGFD_SERVICE,NGFD_PATH, NGFD_INTERFACE,
    NGFD_CONNECTION, aParent), iEventId(0), iPlaying(false)
{
    connect(this, SIGNAL(Status(uint,uint)), SLOT(onStatusChanged(uint,uint)));
}

bool BooksFeedback::Private::start(QString aEvent)
{
    if (!iPlaying) {
        iPlaying = true;
        HDEBUG(aEvent);
        connect(new QDBusPendingCallWatcher(asyncCall(QString("Play"),
            aEvent, QVariantMap()), this),
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(onPlayFinished(QDBusPendingCallWatcher*)));
        return true;
    }
    return false;
}

void BooksFeedback::Private::stop(uint aEventId)
{
    HDEBUG(aEventId);
    asyncCall(QString("Stop"), iPlaying);
}

void BooksFeedback::Private::stop()
{
    if (iPlaying) {
        iPlaying = false;
        if (iEventId) {
            stop(iEventId);
            iEventId = 0;
        }
    }
}

void BooksFeedback::Private::onPlayFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<uint> reply = *aCall;
    if (reply.isError()) {
        HWARN(reply.error());
        iPlaying = false;
    } else {
        uint eventId= reply.value();
        HDEBUG(eventId);
        if (iPlaying) {
            iEventId = eventId;
        } else {
            stop(eventId);
        }
    }
    aCall->deleteLater();
}

void BooksFeedback::Private::onStatusChanged(uint aId, uint aStatus)
{
    HDEBUG(aId << aStatus);
    switch (aStatus) {
    case NGFD_STATUS_PLAYING:
    case NGFD_STATUS_PAUSED:
        break;
    default:
        if (aId == iEventId) {
            iEventId = 0;
            iPlaying = false;
        }
        break;
    }
}

BooksFeedback::BooksFeedback(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

bool BooksFeedback::start(QString aEvent)
{
    return iPrivate->start(aEvent);
}

void BooksFeedback::stop()
{
    iPrivate->stop();
}

// Callback for qmlRegisterSingletonType<BooksFeedback>
QObject* BooksFeedback::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new BooksFeedback;
}

#include "BooksFeedback.moc"
