/*
 * Copyright (C) 2016 Jolla Ltd.
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
 *   * Neither the name of Jolla Ltd nor the names of its contributors
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

#include "BooksImageProvider.h"
#include "HarbourDebug.h"

const QString BooksImageProvider::PROVIDER_ID("bookImage");
BooksImageProvider* BooksImageProvider::gInstance = NULL;

BooksImageProvider*
BooksImageProvider::instance()
{
    if (!gInstance) {
        new BooksImageProvider;
    }
    return gInstance;
}

BooksImageProvider::BooksImageProvider(
    QObject* aParent) :
    QObject(aParent),
    QQuickImageProvider(QQuickImageProvider::Image)
{
    gInstance = this;
}

BooksImageProvider::~BooksImageProvider()
{
    if (gInstance == this) {
        gInstance = NULL;
    }
}

void
BooksImageProvider::addImage(
    QObject* aOwner,
    QString aId,
    QImage aImage)
{
    if (aOwner && !aId.isEmpty() && !aImage.isNull()) {
        QMutexLocker locker(&iMutex);
        QStringList ids = iOwnerMap.value(aOwner);
        if (ids.isEmpty()) {
            connect(aOwner, SIGNAL(destroyed(QObject*)),
                SLOT(releaseImages(QObject*)));
        }
        ids.append(aId);
        iOwnerMap.insert(aOwner, ids);
        iImageMap.insert(aId, aImage);
    }
}

void
BooksImageProvider::releaseImages(
    QObject* aOwner)
{
    QMutexLocker locker(&iMutex);
    const QStringList ids = iOwnerMap.take(aOwner);
    const int n = ids.count();
    for (int i=0; i<n; i++) {
        HDEBUG(ids.at(i));
        iImageMap.remove(ids.at(i));
    }
}

QImage
BooksImageProvider::requestImage(
    const QString& aId,
    QSize* aSize,
    const QSize& aRequestedSize)
{
    QMutexLocker locker(&iMutex);
    QImage image = iImageMap.value(aId);
    if (!image.isNull()) {
        HDEBUG(aId << image.size());
        if (aRequestedSize.isEmpty() || image.size() == aRequestedSize) {
            *aSize = image.size();
            return image;
        } else {
            *aSize = aRequestedSize;
            return image.scaled(aRequestedSize, Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation);
        }
    } else {
        HWARN("No such image:" << aId);
    }
    return QImage();
}
