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

#include "BooksListWatcher.h"

#include "HarbourDebug.h"

#define LISTVIEW_CONTENT_X              "contentX"
#define LISTVIEW_CONTENT_Y              "contentY"
#define LISTVIEW_INDEX_AT               "indexAt"

BooksListWatcher::BooksListWatcher(QObject* aParent) :
    QObject(aParent),
    iCurrentIndex(0),
    iContentX(0),
    iContentY(0),
    iListView(NULL),
    iResizeTimer(new QTimer(this))
{
    iResizeTimer->setSingleShot(true);
    iResizeTimer->setInterval(0);
    connect(iResizeTimer, SIGNAL(timeout()), SLOT(onResizeTimeout()));
}

void BooksListWatcher::setListView(QQuickItem* aView)
{
    if (iListView != aView) {
        const QSize oldSize(iSize);
        if (iListView) iListView->disconnect(this);
        iListView = aView;
        if (iListView) {
            connect(iListView,
                SIGNAL(widthChanged()),
                SLOT(onWidthChanged()));
            connect(iListView,
                SIGNAL(heightChanged()),
                SLOT(onHeightChanged()));
            connect(iListView,
                SIGNAL(contentXChanged()),
                SLOT(onContentXChanged()));
            connect(iListView,
                SIGNAL(contentYChanged()),
                SLOT(onContentYChanged()));
            connect(iListView,
                SIGNAL(contentWidthChanged()),
                SLOT(onContentSizeChanged()));
            connect(iListView,
                SIGNAL(contentHeightChanged()),
                SLOT(onContentSizeChanged()));
            iContentX = contentX();
            iContentY = contentY();
            updateCurrentIndex();
        } else {
            iContentX = iContentY = 0;
            iSize = QSize(0,0);
        }
        Q_EMIT listViewChanged();
        if (oldSize != iSize) {
            Q_EMIT sizeChanged();
        }
        if (oldSize.width() != iSize.width()) {
            Q_EMIT widthChanged();
        }
        if (oldSize.height() != iSize.height()) {
            Q_EMIT heightChanged();
        }
    }
}

qreal BooksListWatcher::getRealProperty(const char *name, qreal defaultValue)
{
    QVariant value = iListView->property(name);
    bool ok = false;
    if (value.isValid()) {
        ok = false;
        qreal r = value.toReal(&ok);
        if (ok) return r;
    }
    return defaultValue;
}

qreal BooksListWatcher::contentX()
{
    return getRealProperty(LISTVIEW_CONTENT_X);
}

qreal BooksListWatcher::contentY()
{
    return getRealProperty(LISTVIEW_CONTENT_Y);
}

void BooksListWatcher::updateCurrentIndex()
{
    int index = -1;
    if (QMetaObject::invokeMethod(iListView, LISTVIEW_INDEX_AT,
        Q_RETURN_ARG(int,index), Q_ARG(qreal,iContentX+width()/2),
        Q_ARG(qreal,iContentY+height()/2))) {
        if (iCurrentIndex != index) {
            HDEBUG(index);
            iCurrentIndex = index;
            Q_EMIT currentIndexChanged();
        }
    }
}

void BooksListWatcher::updateSize()
{
    const QSize size(iListView->width(), iListView->height());
    HDEBUG(size);
    if (iSize != size) {
        const QSize oldSize(iSize);
        iSize = size;
        Q_EMIT sizeChanged();
        if (oldSize.width() != iSize.width()) {
            Q_EMIT widthChanged();
        }
        if (oldSize.height() != iSize.height()) {
            Q_EMIT heightChanged();
        }
    }
}

void BooksListWatcher::onWidthChanged()
{
    HASSERT(sender() == iListView);
    HDEBUG(iListView->width());
    // Width change will probably be followed by height change
    iResizeTimer->start();
}

void BooksListWatcher::onHeightChanged()
{
    HASSERT(sender() == iListView);
    HDEBUG(iListView->height());
    if (iResizeTimer->isActive()) {
        // Height is usually changed after width
        iResizeTimer->stop();
        updateSize();
    } else {
        iResizeTimer->start();
    }
}

void BooksListWatcher::onResizeTimeout()
{
    // This can only happen if only width or height has changed. Normally,
    // width change is followed by height change and view is reset from the
    // setHeight() method
    updateSize();
}

void BooksListWatcher::onContentXChanged()
{
    HASSERT(sender() == iListView);
    iContentX = contentX();
    updateCurrentIndex();
}

void BooksListWatcher::onContentYChanged()
{
    HASSERT(sender() == iListView);
    iContentY = contentY();
    updateCurrentIndex();
}

void BooksListWatcher::onContentSizeChanged()
{
    HASSERT(sender() == iListView);
    updateCurrentIndex();
}
