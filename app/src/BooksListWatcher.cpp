/*
 * Copyright (C) 2015-2020 Jolla Ltd.
 * Copyright (C) 2015-2020 Slava Monich <slava.monich@jolla.com>
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE ``USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "BooksListWatcher.h"

#include "HarbourDebug.h"

// Properties
static const char LISTVIEW_CONTENT_X[] = "contentX";
static const char LISTVIEW_CONTENT_Y[] = "contentY";
static const char LISTVIEW_CONTENT_WIDTH[] = "contentWidth";
static const char LISTVIEW_CONTENT_HEIGHT[] = "contentHeight";
static const char LISTVIEW_CURRENT_INDEX[] = "currentIndex";

// Methods
static const char LISTVIEW_INDEX_AT[] = "indexAt";
static const char LISTVIEW_POSITION_VIEW_AT_INDEX[] = "positionViewAtIndex";

BooksListWatcher::BooksListWatcher(QObject* aParent) :
    QObject(aParent),
    iCurrentIndex(-1),
    iContentX(0),
    iContentY(0),
    iListView(NULL),
    iCenterMode(-1),
    iPositionIsChanging(0),
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
        iCenterMode = -1;
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
            iSize = QSize(iListView->width(), iListView->height());
        } else {
            iContentX = iContentY = 0;
            iSize = QSize(0,0);
        }
        Q_EMIT listViewChanged();
        updateCurrentIndex();
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

qreal BooksListWatcher::getRealProperty(const char* aName, qreal aDefaultValue)
{
    if (iListView) {
        QVariant value = iListView->property(aName);
        bool ok = false;
        if (value.isValid()) {
            ok = false;
            qreal r = value.toReal(&ok);
            if (ok) return r;
        }
    }
    return aDefaultValue;
}

void BooksListWatcher::positionViewAtIndex(int aIndex)
{
    if (iListView) {
        HDEBUG(aIndex);
        doPositionViewAtIndex(aIndex);
        updateCurrentIndex();
    }
}

void BooksListWatcher::doPositionViewAtIndex(int aIndex)
{
    if (iCenterMode < 0) {
        bool ok = false;
        const QMetaObject* metaObject = iListView->metaObject();
        if (metaObject) {
            int index = metaObject->indexOfEnumerator("PositionMode");
            if (index >= 0) {
                QMetaEnum metaEnum = metaObject->enumerator(index);
                int value = metaEnum.keyToValue("Center", &ok);
                if (ok) {
                    iCenterMode = value;
                    HDEBUG("Center =" << iCenterMode);
                }
            }
        }
        HASSERT(ok);
        if (!ok) {
            // This is what it normally is
            iCenterMode = 1;
        }
    }
    positionUpdatedStarted();
    HDEBUG(iListView->property(LISTVIEW_CURRENT_INDEX).toInt());
    iListView->setProperty(LISTVIEW_CURRENT_INDEX, aIndex);
    HDEBUG(iListView->property(LISTVIEW_CURRENT_INDEX).toInt());
    positionViewAtIndex(aIndex, iCenterMode);
    const int currentIndex = getCurrentIndex();
    if (currentIndex != aIndex) {
        // Didn't work from the first try, give it another go
        HDEBUG("still" << currentIndex << ", retrying...");
        positionViewAtIndex(aIndex, iCenterMode);
    }
    positionUpdatedFinished();
    updateCurrentIndex();
}

void BooksListWatcher::positionViewAtIndex(int aIndex, int aMode)
{
    if (iListView) {
        QMetaObject::invokeMethod(iListView,
            LISTVIEW_POSITION_VIEW_AT_INDEX,
            Q_ARG(int,aIndex), Q_ARG(int,aMode));
    }
}

void BooksListWatcher::positionUpdatedStarted()
{
    if (!iPositionIsChanging++) {
        Q_EMIT updatingViewPositionChanged();
    }
}

void BooksListWatcher::positionUpdatedFinished()
{
    if (!--iPositionIsChanging) {
        Q_EMIT updatingViewPositionChanged();
    }
}

qreal BooksListWatcher::contentX()
{
    return getRealProperty(LISTVIEW_CONTENT_X);
}

qreal BooksListWatcher::contentY()
{
    return getRealProperty(LISTVIEW_CONTENT_Y);
}

qreal BooksListWatcher::contentWidth()
{
    return getRealProperty(LISTVIEW_CONTENT_WIDTH);
}

qreal BooksListWatcher::contentHeight()
{
    return getRealProperty(LISTVIEW_CONTENT_HEIGHT);
}

int BooksListWatcher::getCurrentIndex()
{
    if (iListView) {
        int index = -1;
        if (QMetaObject::invokeMethod(iListView, LISTVIEW_INDEX_AT,
            Q_RETURN_ARG(int,index), Q_ARG(qreal,iContentX+width()/2),
            Q_ARG(qreal,iContentY+height()/2))) {
            return index;
        }
    }
    return -1;
}

void BooksListWatcher::tryToRestoreCurrentIndex()
{
    if (iCurrentIndex >= 0 && !updatingViewPosition()) {
        const int index = getCurrentIndex();
        if (iCurrentIndex != index) {
            HDEBUG(index << "->" << iCurrentIndex);
            doPositionViewAtIndex(iCurrentIndex);
            HDEBUG(getCurrentIndex());
        }
    }
}

void BooksListWatcher::updateCurrentIndex()
{
    if (!updatingViewPosition() && (contentWidth() > 0 || contentHeight() > 0)) {
        const int index = getCurrentIndex();
        if (iCurrentIndex != index && index >= 0) {
            iCurrentIndex = index;
            HDEBUG(index << contentWidth() << "x" << contentHeight());
            Q_EMIT currentIndexChanged();
        }
    } else {
        HDEBUG(contentWidth() << "x" << contentHeight());
    }
}

void BooksListWatcher::updateSize()
{
    const QSize size(iListView->width(), iListView->height());
    HDEBUG(size << getCurrentIndex());
    if (iSize != size) {
        const QSize oldSize(iSize);
        iSize = size;
        tryToRestoreCurrentIndex();
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
    if (!updatingViewPosition() && !iResizeTimer->isActive()) {
        updateCurrentIndex();
    }
}

void BooksListWatcher::onContentYChanged()
{
    HASSERT(sender() == iListView);
    iContentY = contentY();
    if (!updatingViewPosition() && !iResizeTimer->isActive()) {
        updateCurrentIndex();
    }
}

void BooksListWatcher::onContentSizeChanged()
{
    HASSERT(sender() == iListView);
    if (!updatingViewPosition() && !iResizeTimer->isActive()) {
        tryToRestoreCurrentIndex();
        updateCurrentIndex();
    }
}
