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

#ifndef BOOKS_PAGE_WIDGET_H
#define BOOKS_PAGE_WIDGET_H

#include "BooksTypes.h"
#include "BooksTaskQueue.h"
#include "BooksSettings.h"
#include "BooksBookModel.h"
#include "BooksPos.h"
#include "BooksPaintContext.h"
#include "BooksLoadingProperty.h"

#include "ZLTextStyle.h"

#include <QQuickPaintedItem>
#include <QTimer>
#include <QList>

class BooksPageWidget: public QQuickPaintedItem, private BooksLoadingProperty
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool selecting READ selecting NOTIFY selectingChanged)
    Q_PROPERTY(bool selectionEmpty READ selectionEmpty NOTIFY selectionEmptyChanged)
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed NOTIFY pressedChanged)
    Q_PROPERTY(bool currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(int leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(int rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(int topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
    Q_PROPERTY(int bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
    Q_PROPERTY(BooksBookModel* model READ model WRITE setModel NOTIFY modelChanged)

public:
    class Data;

    explicit BooksPageWidget(QQuickItem* aParent = NULL);
    ~BooksPageWidget();

    bool loading() const;
    bool selecting() const;
    bool selectionEmpty() const;

    bool pressed() const;
    void setPressed(bool aPressed);

    bool currentPage() const;
    void setCurrentPage(bool aCurrentPage);

    int page() const;
    void setPage(int aPage);

    BooksBookModel* model() const;
    void setModel(BooksBookModel* aModel);

    int leftMargin() const;
    int rightMargin() const;
    int topMargin() const;
    int bottomMargin() const;

    void setLeftMargin(int aMargin);
    void setRightMargin(int aMargin);
    void setTopMargin(int aMargin);
    void setBottomMargin(int aMargin);

    BooksMargins margins() const;

    Q_INVOKABLE void handlePress(int aX, int aY);
    Q_INVOKABLE void handleLongPress(int aX, int aY);
    Q_INVOKABLE void handlePositionChanged(int aX, int aY);
    Q_INVOKABLE void clearSelection();

Q_SIGNALS:
    void loadingChanged();
    void pressedChanged();
    void selectingChanged();
    void selectionEmptyChanged();
    void currentPageChanged();
    void pageChanged();
    void modelChanged();
    void leftMarginChanged();
    void rightMarginChanged();
    void topMarginChanged();
    void bottomMarginChanged();
    void browserLinkPressed(QString url);
    void imagePressed(QString imageId, QRect rect);
    void activeTouch(int touchX, int touchY);
    void jumpToPage(int page);
    void showFootnote(int touchX, int touchY, QString text, QString imageId);
    void pushPosition(BooksPos position);

private Q_SLOTS:
    void onWidthChanged();
    void onHeightChanged();
    void onResizeTimeout();
    void onBookModelChanged();
    void onBookModelDestroyed();
    void onBookModelPageMarksChanged();
    void onBookModelLoadingChanged();
    void onTextStyleChanged();
    void onColorsChanged();
    void onResetTaskDone();
    void onRenderTaskDone();
    void onClearSelectionTaskDone();
    void onStartSelectionTaskDone();
    void onExtendSelectionTaskDone();
    void onPressTaskDone();
    void onLongPressTaskDone();
    void onFootnoteTaskDone();

private:
    void paint(QPainter *painter);
    void updateSize();
    void resetView();
    void releaseExtendSelectionTasks();
    void scheduleRepaint();
    void cancelRepaint();

private:
    class ResetTask;
    class RenderTask;
    class PressTask;
    class ClearSelectionTask;
    class StartSelectionTask;
    class ExtendSelectionTask;
    class FootnoteTask;

    QSharedPointer<BooksSettings> iSettings;
    shared_ptr<BooksTaskQueue> iTaskQueue;
    shared_ptr<ZLTextStyle> iTextStyle;
    BooksPos iPageMark;
    QTimer* iResizeTimer;
    BooksBookModel* iModel;
    BooksMargins iMargins;
    shared_ptr<Data> iData;
    QImage iImage;
    ResetTask* iResetTask;
    RenderTask* iRenderTask;
    ClearSelectionTask* iClearSelectionTask;
    StartSelectionTask* iStartSelectionTask;
    QList<ExtendSelectionTask*> iExtendSelectionTasks;
    PressTask* iPressTask;
    PressTask* iLongPressTask;
    FootnoteTask* iFootnoteTask;
    bool iEmpty;
    bool iPressed;
    bool iSelecting;
    bool iSelectionEmpty;
    bool iCurrentPage;
    int iPage;
};

inline bool BooksPageWidget::pressed() const
    { return iPressed; }
inline bool BooksPageWidget::selecting() const
    { return iSelecting; }
inline bool BooksPageWidget::selectionEmpty() const
    { return iSelectionEmpty; }
inline bool BooksPageWidget::currentPage() const
    { return iCurrentPage; }
inline int BooksPageWidget::page() const
    { return iPage; }
inline BooksBookModel* BooksPageWidget::model() const
    { return iModel; }
inline int BooksPageWidget::leftMargin() const
    { return iMargins.iLeft; }
inline int BooksPageWidget::rightMargin() const
    { return iMargins.iRight; }
inline int BooksPageWidget::topMargin() const
    { return iMargins.iTop; }
inline int BooksPageWidget::bottomMargin() const
    { return iMargins.iBottom; }
inline BooksMargins BooksPageWidget::margins() const
    { return iMargins; }

#endif // BOOKS_PAGE_WIDGET_H
