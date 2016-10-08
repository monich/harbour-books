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

#ifndef BOOKS_IMAGE_PROVIDER_H
#define BOOKS_IMAGE_PROVIDER_H

#include "ZLImageManager.h"

#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

class BooksImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT

public:
    static const QString PROVIDER_ID;

    static BooksImageProvider* instance();
    BooksImageProvider(QObject* aParent = NULL);
    virtual ~BooksImageProvider();

    void addImage(QObject* aOwner, QString aId, shared_ptr<ZLImageData> aData);
    virtual QImage requestImage(const QString& aId, QSize* aSize,
        const QSize& aRequestedSize);

public Q_SLOTS:
    void releaseImages(QObject* aOwner);

private:
    QMutex iMutex;
    QHash<QString, shared_ptr<ZLImageData> > iImageMap;
    QHash<QObject*, QStringList> iOwnerMap;
    static BooksImageProvider* gInstance;
};

#endif // BOOKS_IMAGE_PROVIDER_H
