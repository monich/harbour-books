/*
 * Copyright (C) 2015-2021 Jolla Ltd.
 * Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>
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

#ifndef BOOKS_COVER_WIDGET_H
#define BOOKS_COVER_WIDGET_H

#include "BooksTypes.h"
#include "BooksTaskQueue.h"
#include "BooksBook.h"

#include "ZLImage.h"
#include "image/ZLQtImageManager.h"

#include <QImage>
#include <QQuickPaintedItem>

class BooksCoverWidget: public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool synchronous READ synchronous WRITE setSynchronous NOTIFY synchronousChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(qreal borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QUrl defaultCover READ defaultCover WRITE setDefaultCover NOTIFY defaultCoverChanged)
    Q_PROPERTY(BooksBook* book READ book WRITE setBook NOTIFY bookChanged)
    Q_PROPERTY(qreal centerX READ centerX NOTIFY centerXChanged)
    Q_PROPERTY(qreal centerY READ centerY NOTIFY centerYChanged)
    Q_PROPERTY(QPoint center READ center NOTIFY centerChanged)
    Q_ENUMS(Mode)

public:
    enum Mode {
        Fill,
        Stretch,
        Bottom
    };

    BooksCoverWidget(QQuickItem* aParent = NULL);
    ~BooksCoverWidget();

    bool empty() const;
    bool loading() const;

    qreal borderWidth() const { return iBorderRadius; }
    void setBorderWidth(qreal aWidth);

    qreal borderRadius() const { return iBorderRadius; }
    void setBorderRadius(qreal aRadius);

    QColor borderColor() const { return iBorderColor; }
    void setBorderColor(QColor aColor);

    QUrl defaultCover() const { return iDefaultCover; }
    void setDefaultCover(QUrl aUrl);

    BooksBook* book() const { return iBook; }
    void setBook(BooksBook* aBook);

    Mode mode() const { return iMode; }
    void setMode(Mode aMode);

    bool synchronous() const { return iSynchronous; }
    void setSynchronous(bool aValue);

    qreal centerX() const { return iCenter.x(); }
    qreal centerY() const { return iCenter.y(); }
    QPoint center() const { return iCenter; }

Q_SIGNALS:
    void bookChanged();
    void emptyChanged();
    void loadingChanged();
    void synchronousChanged();
    void modeChanged();
    void borderWidthChanged();
    void borderRadiusChanged();
    void borderColorChanged();
    void defaultCoverChanged();
    void centerXChanged();
    void centerYChanged();
    void centerChanged();

private Q_SLOTS:
    void onCoverImageChanged();
    void onSizeChanged();
    void onScaleTaskDone();

private:
    void paint(QPainter *painter);
    void scaleImage(bool aWasEmpty);
    void scaleImage() { scaleImage(empty()); }
    void updateCenter();

private:
    class Scaler;
    class ScaleTask;
    class DefaultImage;
    shared_ptr<BooksTaskQueue> iTaskQueue;
    ScaleTask* iScaleTask;
    QImage iScaledImage;
    QImage iCoverImage;
    QColor iBackground1;
    QColor iBackground2;
    BooksBook* iBook;
    QImage* iDefaultImage;
    qreal iBorderWidth;
    qreal iBorderRadius;
    QColor iBorderColor;
    QUrl iDefaultCover;
    QString iTitle;
    QPoint iCenter;
    Mode iMode;
    bool iSynchronous;
};

#endif // BOOKS_COVER_WIDGET_H
