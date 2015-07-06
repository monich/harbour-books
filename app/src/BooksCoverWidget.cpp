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

#include "BooksCoverWidget.h"

#include "HarbourDebug.h"

#include "ZLibrary.h"

#include <QPainter>

// ==========================================================================
// BooksCoverWidget::ScaleTask
// ==========================================================================

class BooksCoverWidget::ScaleTask : public BooksTask
{
public:
    ScaleTask(QImage aImage, int aWidth, int aHeight, bool aStretch);
    static QImage scale(QImage aImage, int aWidth, int aHeight, bool aStretch);
    void performTask();

public:
    QImage iImage;
    QImage iScaledImage;
    int iWidth;
    int iHeight;
    bool iStretch;
};

BooksCoverWidget::ScaleTask::ScaleTask(QImage aImage, int aWidth, int aHeight,
    bool aStretch) :
    iImage(aImage),
    iWidth(aWidth),
    iHeight(aHeight),
    iStretch(aStretch)
{
}

QImage BooksCoverWidget::ScaleTask::scale(QImage aImage,
    int aWidth, int aHeight, bool aStretch)
{
    if (aStretch){
        return aImage.scaled(aWidth, aHeight, Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation);
    } else {
        return (aWidth*aImage.height() > aImage.width()*aHeight) ?
            aImage.scaledToHeight(aHeight, Qt::SmoothTransformation) :
            aImage.scaledToWidth(aWidth, Qt::SmoothTransformation);
    }
}

void BooksCoverWidget::ScaleTask::performTask()
{
    if (!iImage.isNull() && !isCanceled()) {
        iScaledImage = scale(iImage, iWidth, iHeight, iStretch);
    }
}

// ==========================================================================
// BooksCoverWidget::DefaultImage
// ==========================================================================

// Image shared by all items in the bookshelf view
class BooksCoverWidget::DefaultImage
{
public:
    static QImage scaled(int aWidth, int aHeight);
    static QImage* retain();
    static void release(QImage* aImage);

private:
    static const char* iImageName;
    static QImage* iImage;
    static QImage* iScaledImage;
    static int iRefCount;
    static bool iMissing;
};

const char* BooksCoverWidget::DefaultImage::iImageName = "default-cover.jpg";
QImage* BooksCoverWidget::DefaultImage::iImage = NULL;
QImage* BooksCoverWidget::DefaultImage::iScaledImage = NULL;
int BooksCoverWidget::DefaultImage::iRefCount = 0;
bool BooksCoverWidget::DefaultImage::iMissing = false;

QImage* BooksCoverWidget::DefaultImage::retain()
{
    if (!iImage && !iMissing) {
        QString path(QString::fromStdString(
            ZLibrary::DefaultFilesPathPrefix() + iImageName));
        iImage = new QImage(path);
        if (iImage->isNull() || !iImage->width() || !iImage->height()) {
            HWARN("Failed to load" << qPrintable(path));
            delete iImage;
            iImage = NULL;
            iMissing = true;
        } else {
            HDEBUG("loaded" << qPrintable(path));
        }
    }
    if (iImage) {
        iRefCount++;
    }
    return iImage;
}

QImage BooksCoverWidget::DefaultImage::scaled(int aWidth, int aHeight)
{
    QImage scaled;
    HASSERT(iImage);
    if (iImage) {
        const int iw = iImage->width();
        const int ih = iImage->height();
        if (aWidth*ih > iw*aHeight) {
            // Scaling to height
            if (iScaledImage && iScaledImage->height() != aHeight) {
                delete iScaledImage;
                iScaledImage = NULL;
            }
            if (iScaledImage) {
                scaled = *iScaledImage;
            } else {
                HDEBUG("scaling to height" << aHeight);
                scaled = iImage->scaledToHeight(aHeight,
                    Qt::SmoothTransformation);
                iScaledImage = new QImage(scaled);
            }
        } else {
            // Scaling to width
            if (iScaledImage && iScaledImage->width() != aWidth) {
                delete iScaledImage;
                iScaledImage = NULL;
            }
            if (iScaledImage) {
                scaled = *iScaledImage;
            } else {
                HDEBUG("scaling to width" << aHeight);
                scaled = iImage->scaledToWidth(aWidth,
                    Qt::SmoothTransformation);
                iScaledImage = new QImage(scaled);
            }
        }
    }
    return scaled;
}

void BooksCoverWidget::DefaultImage::release(QImage* aImage)
{
    if (aImage) {
        HASSERT(aImage == iImage);
        if (!(--iRefCount)) {
            HDEBUG("deleting cached image");
            if (iImage) {
                delete iImage;
                iImage = NULL;
            }
            if (iScaledImage) {
                delete iScaledImage;
                iScaledImage = NULL;
            }
        }
    }
}

// ==========================================================================
// BooksViewWidget
// ==========================================================================

BooksCoverWidget::BooksCoverWidget(QQuickItem* aParent) :
    QQuickPaintedItem(aParent),
    iTaskQueue(BooksTaskQueue::instance()),
    iScaleTask(NULL),
    iBook(NULL),
    iDefaultImage(NULL),
    iBorderWidth(0),
    iBorderRadius(0),
    iBorderColor(Qt::transparent),
    iStretch(false),
    iSynchronous(false)
{
    setFlag(ItemHasContents, true);
    setFillColor(Qt::transparent);
    connect(this, SIGNAL(widthChanged()), SLOT(onSizeChanged()));
    connect(this, SIGNAL(heightChanged()), SLOT(onSizeChanged()));
}

BooksCoverWidget::~BooksCoverWidget()
{
    HDEBUG(iTitle);
    DefaultImage::release(iDefaultImage);
    if (iScaleTask) iScaleTask->release(this);
    if (iBook) iBook->release();
}

void BooksCoverWidget::setBook(BooksBook* aBook)
{
    if (iBook != aBook) {
        const bool wasEmpty(empty());
        const bool wasLoading = loading();
        if (iBook) {
            iBook->disconnect(this);
            iBook->release();
        }
        if (aBook) {
            (iBook = aBook)->retain();
            iBook->requestCoverImage();
            iBookRef = iBook->bookRef();
            iCoverImage = iBook->coverImage();
            iTitle = iBook->title();
            connect(iBook,
                SIGNAL(loadingCoverChanged()),
                SIGNAL(loadingChanged()));
            connect(iBook,
                SIGNAL(coverImageChanged()),
                SLOT(onCoverImageChanged()));
            HDEBUG(iTitle);
        } else {
            iBook = NULL;
            iBookRef.reset();
            iCoverImage = QImage();
            iTitle.clear();
            HDEBUG("<none>");
        }
        scaleImage(wasEmpty);
        Q_EMIT bookChanged();
        if (wasLoading != loading()) {
            Q_EMIT loadingChanged();
        }
    }
}

void BooksCoverWidget::onCoverImageChanged()
{
    HDEBUG(iTitle);
    const bool wasEmpty(empty());
    iCoverImage = iBook->coverImage();
    scaleImage(wasEmpty);
}

void BooksCoverWidget::setStretch(bool aValue)
{
    if (iStretch != aValue) {
        iStretch = aValue;
        HDEBUG(aValue);
        scaleImage();
        Q_EMIT stretchChanged();
    }
}

void BooksCoverWidget::setSynchronous(bool aValue)
{
    if (iSynchronous != aValue) {
        iSynchronous = aValue;
        HDEBUG(aValue);
        Q_EMIT synchronousChanged();
    }
}

void BooksCoverWidget::setBorderWidth(qreal aWidth)
{
    if (iBorderWidth != aWidth && aWidth >= 0) {
        iBorderWidth = aWidth;
        HDEBUG(iBorderWidth);
        update();
        Q_EMIT borderWidthChanged();
    }
}

void BooksCoverWidget::setBorderRadius(qreal aRadius)
{
    if (iBorderRadius != aRadius && aRadius >= 0) {
        iBorderRadius = aRadius;
        HDEBUG(iBorderRadius);
        update();
        Q_EMIT borderRadiusChanged();
    }
}

void BooksCoverWidget::setBorderColor(QColor aColor)
{
    if (iBorderColor != aColor) {
        iBorderColor = aColor;
        HDEBUG(iBorderColor);
        update();
        Q_EMIT borderColorChanged();
    }
}

void BooksCoverWidget::setDefaultCover(QUrl aUrl)
{
    if (iDefaultCover != aUrl) {
        iDefaultCover = aUrl;
        HDEBUG(iDefaultCover);
        scaleImage();
        Q_EMIT defaultCoverChanged();
    }
}

void BooksCoverWidget::onSizeChanged()
{
    scaleImage();
}

bool BooksCoverWidget::empty() const
{
    return !iBook || !iBook->hasCoverImage() || iScaledImage.isNull();
}

bool BooksCoverWidget::loading() const
{
    return iBook && iBook->loadingCover();
}

void BooksCoverWidget::scaleImage(bool aWasEmpty)
{
    const int w = width();
    const int h = height();

    if (iScaleTask) {
        iScaleTask->release(this);
        iScaleTask = NULL;
    }

    if (w > 0 && h > 0) {
        if ((!iBook || !iBook->hasCoverImage()) && iDefaultCover.isValid()) {
            QString path(iDefaultCover.toLocalFile());
            if (!iCoverImage.load(path)) {
                HWARN("Failed to load" << qPrintable(path));
            }
        }
        if (iCoverImage.isNull()) {
            if (!iDefaultImage) iDefaultImage = DefaultImage::retain();
            if (iDefaultImage) iCoverImage = *iDefaultImage;
        }

        if (!iCoverImage.isNull()) {
            if (iSynchronous) {
                iScaledImage = ScaleTask::scale(iCoverImage, w, h, iStretch);
                update();
            } else {
                iScaleTask = new ScaleTask(iCoverImage, w, h, iStretch);
                connect(iScaleTask, SIGNAL(done()), SLOT(onScaleTaskDone()));
                iTaskQueue->submit(iScaleTask);
            }
        } else {
            iScaledImage = QImage();
            update();
        }
    } else {
        iScaledImage = QImage();
    }

    if (aWasEmpty != empty()) {
        Q_EMIT emptyChanged();
    }
}

void BooksCoverWidget::onScaleTaskDone()
{
    const bool wasEmpty(empty());
    HASSERT(iScaleTask == sender());
    iScaledImage = iScaleTask->iScaledImage;
    iScaleTask->release(this);
    iScaleTask = NULL;
    update();
    if (wasEmpty != empty()) {
        Q_EMIT emptyChanged();
    }
}

void BooksCoverWidget::paint(QPainter* aPainter)
{
    const qreal w = width();
    const qreal h = height();
    if (w > 0 && h > 0) {
        qreal sw, sh;
        if (!iScaledImage.isNull()) {
            sw = iScaledImage.width();
            sh = iScaledImage.height();
        } else {
            sw = w;
            sh = h;
        }

        const int x = (w - sw)/2;
        const int y = h - sh;

        QPainterPath path;
        qreal w1, h1, x1, y1;

        if (iBorderRadius > 0) {
            // The border rectangle is no less that 3*radius
            // and no more than the size of the item.
            const qreal d = 2*iBorderRadius;
            w1 = qMin(w, qMax(sw, 2*d)) - iBorderWidth;
            h1 = qMin(h, qMax(sh, 3*d)) - iBorderWidth;
            x1 = (w - w1)/2;
            y1 = h - h1 - iBorderWidth/2;

            const qreal x2 = x1 + w1 - d;
            const qreal y2 = y1 + h1 - d;
            path.arcMoveTo(x1, y1, d, d, 180);
            path.arcTo(x1, y1, d, d, 180, -90);
            path.arcTo(x2, y1, d, d, 90, -90);
            path.arcTo(x2, y2, d, d, 0, -90);
            path.arcTo(x1, y2, d, d, 270, -90);
            path.closeSubpath();
            aPainter->setClipPath(path);
        } else {
            w1 = sw - iBorderWidth;
            h1 = sh - iBorderWidth;
            x1 = (w - w1)/2;
            y1 = h - h1 - iBorderWidth/2;
        }

        if (!iScaledImage.isNull()) {
            aPainter->drawImage(x, y, iScaledImage);
        }

        if (iBorderColor.alpha() && iBorderWidth > 0) {
            aPainter->setRenderHint(QPainter::Antialiasing);
            aPainter->setRenderHint(QPainter::HighQualityAntialiasing);
            aPainter->setBrush(Qt::NoBrush);
            aPainter->setPen(QPen(iBorderColor, iBorderWidth));
            if (iBorderRadius > 0) {
                aPainter->setClipping(false);
                aPainter->drawPath(path);
            } else {
                aPainter->drawRect(x1, y1, w1, h1);
            }
        }
    }
}
