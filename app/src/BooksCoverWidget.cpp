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

#include "BooksCoverWidget.h"

#include "HarbourDebug.h"
#include "HarbourTask.h"

#include "ZLibrary.h"

#include <QPainter>

// ==========================================================================
// BooksCoverWidget::ScaleTask
// ==========================================================================

class BooksCoverWidget::ScaleTask : public HarbourTask
{
public:
    ScaleTask(QThreadPool* aPool, QImage aImage, int aWidth, int aHeight,
        bool aStretch);
    static QImage scale(QImage aImage, int aWidth, int aHeight, bool aStretch);
    static QColor leftBackground(const QImage& aImage);
    static QColor rightBackground(const QImage& aImage);
    static QColor topBackground(const QImage& aImage);
    static QColor bottomBackground(const QImage& aImage);
    static QColor pickColor(const QHash<QRgb,int>& aColorCounts);
    void performTask();

public:
    QImage iImage;
    QImage iScaledImage;
    QColor iBackground1; // Left or top
    QColor iBackground2; // Right or bottom
    int iWidth;
    int iHeight;
    bool iStretch;
};

BooksCoverWidget::ScaleTask::ScaleTask(QThreadPool* aPool, QImage aImage,
    int aWidth, int aHeight, bool aStretch) : HarbourTask(aPool), iImage(aImage),
    iBackground1(Qt::transparent), iBackground2(Qt::transparent),
    iWidth(aWidth), iHeight(aHeight),
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

// The idea is to pick the colors which occur more often
// at the edges of the picture.
QColor BooksCoverWidget::ScaleTask::leftBackground(const QImage& aImage)
{
    QHash<QRgb,int> counts;
    if (aImage.width() > 0) {
        const int h = aImage.height();
        for (int y = 0; y < h; y++) {
            const QRgb left(aImage.pixel(0, y));
            counts.insert(left, counts.value(left) + 1);
        }
    }
    const QColor color(pickColor(counts));
    HDEBUG(color << "left" << counts.count());
    return color;
}

QColor BooksCoverWidget::ScaleTask::rightBackground(const QImage& aImage)
{
    QHash<QRgb,int> counts;
    const int w = aImage.width();
    if (w > 0) {
        const int h = aImage.height();
        for (int y = 0; y < h; y++) {
            const QRgb right(aImage.pixel(w - 1, y));
            counts.insert(right, counts.value(right) + 1);
        }
    }
    const QColor color(pickColor(counts));
    HDEBUG(color << "right" << counts.count());
    return color;
}

QColor BooksCoverWidget::ScaleTask::topBackground(const QImage& aImage)
{
    QHash<QRgb,int> counts;
    if (aImage.height() > 0) {
        const int w = aImage.width();
        for (int x = 0; x < w; x++) {
            const QRgb left(aImage.pixel(x, 0));
            counts.insert(left, counts.value(left) + 1);
        }
    }
    const QColor color(pickColor(counts));
    HDEBUG(color << "top" << counts.count());
    return color;
}

QColor BooksCoverWidget::ScaleTask::bottomBackground(const QImage& aImage)
{
    QHash<QRgb,int> counts;
    const int h = aImage.height();
    if (h > 0) {
        const int w = aImage.width();
        for (int x = 0; x < w; x++) {
            const QRgb left(aImage.pixel(x, h - 1));
            counts.insert(left, counts.value(left) + 1);
        }
    }
    const QColor color(pickColor(counts));
    HDEBUG(color << "bottom" << counts.count());
    return color;
}

QColor BooksCoverWidget::ScaleTask::pickColor(const QHash<QRgb,int>& aCounts)
{
    if (aCounts.size() > 0) {
        QRgb rgb;
        int max;
        QHashIterator<QRgb,int> it(aCounts);
        for (max = 0; it.hasNext();) {
            it.next();
            const int count = it.value();
            if (max < count) {
                max = count;
                rgb = it.key();
            }
        }
        return QColor(rgb);
    }
    return QColor();
}

void BooksCoverWidget::ScaleTask::performTask()
{
    if (!iImage.isNull() && !isCanceled()) {
        iScaledImage = scale(iImage, iWidth, iHeight, iStretch);
        if (!isCanceled()) {
            if (iScaledImage.width() < iWidth) {
                iBackground1 = leftBackground(iScaledImage);
                if (!isCanceled()) {
                    iBackground2 = rightBackground(iScaledImage);
                }
            } else if (iScaledImage.height() < iHeight) {
                iBackground1 = topBackground(iScaledImage);
                if (!isCanceled()) {
                    iBackground2 = bottomBackground(iScaledImage);
                }
            }
        }
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
    iTaskQueue(BooksTaskQueue::scaleQueue()),
    iScaleTask(NULL),
    iBook(NULL),
    iDefaultImage(NULL),
    iBorderWidth(0),
    iBorderRadius(0),
    iBorderColor(Qt::transparent),
    iMode(Bottom),
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

void BooksCoverWidget::setMode(Mode aMode)
{
    if (iMode != aMode) {
        iMode = aMode;
        HDEBUG(aMode);
        scaleImage();
        Q_EMIT modeChanged();
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
    return iScaledImage.isNull();
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
            const bool stretch = (iMode == Stretch);
            if (iSynchronous) {
                iScaledImage = ScaleTask::scale(iCoverImage, w, h, stretch);
                if (iScaledImage.width() < w) {
                    iBackground1 = ScaleTask::leftBackground(iScaledImage);
                    iBackground2 = ScaleTask::rightBackground(iScaledImage);
                } else if (iScaledImage.height() < h) {
                    iBackground1 = ScaleTask::topBackground(iScaledImage);
                    iBackground2 = ScaleTask::bottomBackground(iScaledImage);
                }
                update();
            } else {
                (iScaleTask = new ScaleTask(iTaskQueue->pool(), iCoverImage,
                    w, h, stretch))->submit(this, SLOT(onScaleTaskDone()));
            }
        } else {
            iScaledImage = QImage();
            update();
        }
    } else {
        iScaledImage = QImage();
    }

    updateCenter();

    if (aWasEmpty != empty()) {
        Q_EMIT emptyChanged();
    }
}

void BooksCoverWidget::onScaleTaskDone()
{
    const bool wasEmpty(empty());
    HASSERT(iScaleTask == sender());
    iScaledImage = iScaleTask->iScaledImage;
    iBackground1 = iScaleTask->iBackground1;
    iBackground2 = iScaleTask->iBackground2;
    iScaleTask->release(this);
    iScaleTask = NULL;
    update();
    updateCenter();
    if (wasEmpty != empty()) {
        Q_EMIT emptyChanged();
    }
}

void BooksCoverWidget::paint(QPainter* aPainter)
{
    const qreal w = width();
    const qreal h = height();
    if (w > 0 && h > 0) {
        // This has to be consistent with updateCenter()
        const qreal sh = (iScaledImage.height() && iMode == Bottom) ?
            qMax((qreal)iScaledImage.height(), w) : h;

        QPainterPath path;
        qreal w1, h1, x1, y1;
        if (iBorderRadius > 0) {
            // The border rectangle is no less that 3*radius
            // and no more than the size of the item.
            const qreal d = 2*iBorderRadius;
            w1 = qMin(w, qMax(w, 2*d)) - iBorderWidth;
            h1 = qMin(h, qMax(sh, 3*d)) - iBorderWidth;
            x1 = floor((w - w1)/2);
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
            w1 = w - iBorderWidth;
            h1 = sh - iBorderWidth;
            x1 = floor((w - w1)/2);
            y1 = h - h1 - iBorderWidth/2;
        }

        const int x = floor((w - iScaledImage.width())/2);
        const int y = floor(h - (sh + iScaledImage.height())/2);

        if (!iScaledImage.isNull()) {
            if (x > 0) {
                aPainter->setPen(Qt::NoPen);
                if (iBackground1.isValid()) {
                    aPainter->setBrush(QBrush(iBackground1));
                    aPainter->drawRect(0, y, x, iScaledImage.height());
                }
                if (iBackground2.isValid()) {
                    const int left = x + iScaledImage.width();
                    aPainter->setBrush(QBrush(iBackground2));
                    aPainter->drawRect(left, y, x, iScaledImage.height());
                }
            } else {
                const int top = h - sh;
                if (y > top) {
                    aPainter->setPen(Qt::NoPen);
                    if (iBackground1.isValid()) {
                        aPainter->setBrush(QBrush(iBackground1));
                        aPainter->drawRect(0, 0, w, y - top);
                    }
                    if (iBackground2.isValid()) {
                        aPainter->setBrush(QBrush(iBackground2));
                        aPainter->drawRect(0, y + iScaledImage.height(), w, y - top);
                    }
                }
            }
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

void BooksCoverWidget::updateCenter()
{
    const QPoint oldCenter(iCenter);
    const qreal w = width();
    const qreal h = height();

    // This has to be consistent with paint()
    iCenter.setX(floor(w/2));
    if (iScaledImage.isNull()) {
        iCenter.setY(floor(h/2));
    } else {
        const qreal sh = qMax((qreal)iScaledImage.height(), w);
        iCenter.setY(floor(h - sh/2));
    }

    if (iCenter != oldCenter) {
        Q_EMIT centerChanged();
        if (iCenter.x() != oldCenter.x()) {
            Q_EMIT centerXChanged();
        }
        if (iCenter.y() != oldCenter.y()) {
            Q_EMIT centerYChanged();
        }
    }
}
