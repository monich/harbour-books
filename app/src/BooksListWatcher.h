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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BOOKS_LIST_WATCHER_H
#define BOOKS_LIST_WATCHER_H

#include "BooksTypes.h"

#include <QQuickItem>
#include <QtQml>

class BooksListWatcher: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QQuickItem* listView READ listView WRITE setListView NOTIFY listViewChanged)
    Q_PROPERTY(qreal width READ width NOTIFY widthChanged)
    Q_PROPERTY(qreal height READ height NOTIFY heightChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)

public:
    explicit BooksListWatcher(QObject* aParent = NULL);

    int currentIndex() const { return iCurrentIndex; }
    QSize size() const { return iSize; }
    qreal width() const { return iSize.width(); }
    qreal height() const { return iSize.height(); }

    QQuickItem* listView() const { return iListView; }
    void setListView(QQuickItem* aView);

    Q_INVOKABLE void positionViewAtIndex(int aIndex);

private:
    qreal contentX();
    qreal contentY();
    qreal contentWidth();
    qreal contentHeight();
    qreal getRealProperty(const char *name, qreal defaultValue = 0.0);
    int getCurrentIndex();
    void doPositionViewAtIndex(int aIndex);
    void positionViewAtIndex(int aIndex, int aMode);
    void updateCurrentIndex();
    void tryToRestoreCurrentIndex();
    void updateSize();

private Q_SLOTS:
    void onResizeTimeout();
    void onWidthChanged();
    void onHeightChanged();
    void onContentXChanged();
    void onContentYChanged();
    void onContentSizeChanged();

Q_SIGNALS:
    void listViewChanged();
    void sizeChanged();
    void widthChanged();
    void heightChanged();
    void currentIndexChanged();

private:
    QSize iSize;
    int iCurrentIndex;
    qreal iContentX;
    qreal iContentY;
    QQuickItem* iListView;
    int iCenterMode;
    int iPositionIsChanging;
    QTimer* iResizeTimer;
};

QML_DECLARE_TYPE(BooksListWatcher)

#endif // BOOKS_LIST_WATCHER_H
