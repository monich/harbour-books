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

#include "BooksPageWidget.h"
#include "BooksTextStyle.h"
#include "BooksDefs.h"

#include "HarbourDebug.h"

#include <QPainter>

// ==========================================================================
// BooksPageWidget::Data
// ==========================================================================

class BooksPageWidget::Data {
public:
    Data(shared_ptr<ZLTextModel> aModel, int aWidth, int aHeight) :
        iModel(aModel), iPaintContext(aWidth, aHeight) {}

    bool paint(QPainter* aPainter);

public:
    shared_ptr<ZLView> iView;
    shared_ptr<ZLTextModel> iModel;
    BooksPaintContext iPaintContext;
};

bool BooksPageWidget::Data::paint(QPainter* aPainter)
{
    if (!iView.isNull()) {
        iPaintContext.beginPaint(aPainter);
        iView->paint();
        iPaintContext.endPaint();
        return true;
    }
    return false;
}

// ==========================================================================
// BooksPageWidget::ResetTask
// ==========================================================================

class BooksPageWidget::ResetTask : public BooksTask
{
public:
    ResetTask(shared_ptr<ZLTextModel> aModel,
        shared_ptr<ZLTextStyle> aTextStyle, int aWidth, int aHeight,
        const BooksMargins& aMargins, const BooksPos& aPosition);
    ~ResetTask();

    void performTask();

public:
    BooksPageWidget::Data* iData;
    shared_ptr<ZLTextStyle> iTextStyle;
    BooksMargins iMargins;
    BooksPos iPosition;
};

BooksPageWidget::ResetTask::ResetTask(shared_ptr<ZLTextModel> aModel,
    shared_ptr<ZLTextStyle> aTextStyle, int aWidth, int aHeight,
    const BooksMargins& aMargins, const BooksPos& aPosition) :
    iData(new BooksPageWidget::Data(aModel, aWidth, aHeight)),
    iTextStyle(aTextStyle),
    iMargins(aMargins),
    iPosition(aPosition)
{
}

BooksPageWidget::ResetTask::~ResetTask()
{
    delete iData;
}

void BooksPageWidget::ResetTask::performTask()
{
    if (!isCanceled()) {
        BooksTextView* view = new BooksTextView(iData->iPaintContext,
            iTextStyle, iMargins);
        if (!isCanceled()) {
            view->setModel(iData->iModel);
            if (!isCanceled()) {
                view->gotoPosition(iPosition);
                if (!isCanceled()) {
                    iData->iView = view;
                } else {
                    delete view;
                }
            }
        }
    }
}

// ==========================================================================
// BooksPageWidget::RenderTask
// ==========================================================================

class BooksPageWidget::RenderTask : public BooksTask {
public:
    RenderTask(shared_ptr<BooksPageWidget::Data> aData, int aWidth, int aHeight) :
        iData(aData), iWidth(aWidth), iHeight(aHeight), iImage(NULL) {}

    void performTask();

public:
    shared_ptr<BooksPageWidget::Data> iData;
    int iWidth;
    int iHeight;
    shared_ptr<QImage> iImage;
};

void BooksPageWidget::RenderTask::performTask()
{
    if (!isCanceled() && !iData.isNull() && !iData->iView.isNull() &&
        iWidth > 0 && iHeight > 0) {
        iImage = new QImage(iWidth, iHeight, QImage::Format_RGB32);
        if (!isCanceled()) {
            QPainter painter(&*iImage);
            iData->paint(&painter);
        }
    }
}

// ==========================================================================
// BooksPageWidget
// ==========================================================================

BooksPageWidget::BooksPageWidget(QQuickItem* aParent) :
    QQuickPaintedItem(aParent),
    iTaskQueue(BooksTaskQueue::instance()),
    iTextStyle(BooksTextStyle::defaults()),
    iResizeTimer(new QTimer(this)),
    iModel(NULL),
    iSettings(NULL),
    iResetTask(NULL),
    iRenderTask(NULL),
    iEmpty(false),
    iPage(-1)
{
    setFlag(ItemHasContents, true);
    setFillColor(Qt::white);
    iResizeTimer->setSingleShot(true);
    iResizeTimer->setInterval(0);
    connect(iResizeTimer, SIGNAL(timeout()), SLOT(onResizeTimeout()));
    connect(this, SIGNAL(widthChanged()), SLOT(onWidthChanged()));
    connect(this, SIGNAL(heightChanged()), SLOT(onHeightChanged()));
}

BooksPageWidget::~BooksPageWidget()
{
    HDEBUG("page" << iPage);
    if (iResetTask) iResetTask->release(this);
    if (iRenderTask) iRenderTask->release(this);
}

void BooksPageWidget::setModel(BooksBookModel* aModel)
{
    if (iModel != aModel) {
        if (iModel) {
            iModel->disconnect(this);
            iModel = NULL;
        }
        iModel = aModel;
        if (iModel) {
#if HARBOUR_DEBUG
            if (iPage >= 0) {
                HDEBUG(iModel->title() << iPage);
            } else {
                HDEBUG(iModel->title());
            }
#endif // HARBOUR_DEBUG
            iPageMark = iModel->pageMark(iPage);
            connect(iModel, SIGNAL(bookModelChanged()),
                SLOT(onBookModelChanged()));
            connect(iModel, SIGNAL(destroyed()),
                SLOT(onBookModelDestroyed()));
            connect(iModel, SIGNAL(pageMarksChanged()),
                SLOT(onBookModelPageMarksChanged()));
            connect(iModel, SIGNAL(loadingChanged()),
                SLOT(onBookModelLoadingChanged()));
        } else {
            iPageMark.invalidate();
        }
        resetView();
        Q_EMIT modelChanged();
    }
}

void BooksPageWidget::setSettings(BooksSettings* aSettings)
{
    if (iSettings != aSettings) {
        shared_ptr<ZLTextStyle> oldTextStyle(iTextStyle);
        if (iSettings) iSettings->disconnect(this);
        iSettings = aSettings;
        if (iSettings) {
            iTextStyle = iSettings->textStyle();
            connect(iSettings, SIGNAL(textStyleChanged()), SLOT(onTextStyleChanged()));
        } else {
            iTextStyle = BooksTextStyle::defaults();
        }
        if (!BooksTextStyle::equalLayout(oldTextStyle, iTextStyle)) {
            resetView();
        }
        Q_EMIT settingsChanged();
    }
}

void BooksPageWidget::onTextStyleChanged()
{
    HDEBUG(iPage);
    iTextStyle = iSettings->textStyle();
    resetView();
}

void BooksPageWidget::onBookModelChanged()
{
    HDEBUG(iModel->title());
    BooksLoadingSignalBlocker block(this);
    iPageMark = iModel->pageMark(iPage);
    resetView();
}

void BooksPageWidget::onBookModelDestroyed()
{
    HDEBUG("model destroyed");
    HASSERT(iModel == sender());
    BooksLoadingSignalBlocker block(this);
    iModel = NULL;
    Q_EMIT modelChanged();
    resetView();
}

void BooksPageWidget::onBookModelPageMarksChanged()
{
    const BooksPos pos = iModel->pageMark(iPage);
    if (iPageMark != pos) {
        BooksLoadingSignalBlocker block(this);
        iPageMark = pos;
        HDEBUG("page" << iPage);
        resetView();
    }
}

void BooksPageWidget::onBookModelLoadingChanged()
{
    BooksLoadingSignalBlocker block(this);
    if (!iModel->loading()) {
        HDEBUG("page" << iPage);
        const BooksPos pos = iModel->pageMark(iPage);
        if (iPageMark != pos) {
            iPageMark = pos;
            resetView();
        }
    }
}

void BooksPageWidget::setPage(int aPage)
{
    if (iPage != aPage) {
        BooksLoadingSignalBlocker block(this);
        iPage = aPage;
        HDEBUG(iPage);
        const BooksPos pos = iModel->pageMark(iPage);
        if (iPageMark != pos) {
            iPageMark = pos;
            resetView();
        }
        resetView();
        Q_EMIT pageChanged();
    }
}

void BooksPageWidget::setLeftMargin(int aMargin)
{
    if (iMargins.iLeft != aMargin) {
        iMargins.iLeft = aMargin;
        HDEBUG(aMargin);
        resetView();
        Q_EMIT leftMarginChanged();
    }
}

void BooksPageWidget::setRightMargin(int aMargin)
{
    if (iMargins.iRight != aMargin) {
        iMargins.iRight = aMargin;
        HDEBUG(aMargin);
        resetView();
        Q_EMIT rightMarginChanged();
    }
}

void BooksPageWidget::setTopMargin(int aMargin)
{
    if (iMargins.iTop != aMargin) {
        iMargins.iTop = aMargin;
        HDEBUG(aMargin);
        resetView();
        Q_EMIT topMarginChanged();
    }
}

void BooksPageWidget::setBottomMargin(int aMargin)
{
    if (iMargins.iBottom != aMargin) {
        iMargins.iBottom = aMargin;
        HDEBUG(aMargin);
        resetView();
        Q_EMIT bottomMarginChanged();
    }
}

void BooksPageWidget::paint(QPainter* aPainter)
{
    if (!iImage.isNull()) {
        HDEBUG("page" << iPage);
        aPainter->drawImage(0, 0, *iImage);
        iEmpty = false;
    } else if (iPage >= 0 && iPageMark.valid()) {
        if (!iRenderTask) {
            HDEBUG("page" << iPage << "(scheduled)");
            scheduleRepaint();
        } else {
            HDEBUG("page" << iPage << "(not yet ready)");
        }
        iEmpty = true;
    } else {
        HDEBUG("page" << iPage << "(empty)");
        iEmpty = true;
    }
}

bool BooksPageWidget::loading() const
{
    return iPage >= 0 && iResetTask && iRenderTask;
}

void BooksPageWidget::resetView()
{
    BooksLoadingSignalBlocker block(this);
    if (iResetTask) {
        iResetTask->release(this);
        iResetTask = NULL;
    }
    iData.reset();
    if (iPage >= 0 && iPageMark.valid() &&
        width() > 0 && height() > 0 && iModel) {
        shared_ptr<ZLTextModel> textModel = iModel->bookTextModel();
        if (!textModel.isNull()) {
            iResetTask = new ResetTask(textModel, iTextStyle,
                width(), height(), iMargins, iPageMark);
            iTaskQueue->submit(iResetTask, this, SLOT(onResetTaskDone()));
            cancelRepaint();
        }
    }
    if (!iResetTask && !iEmpty) {
        update();
    }
}

void BooksPageWidget::cancelRepaint()
{
    BooksLoadingSignalBlocker block(this);
    if (iRenderTask) {
        iRenderTask->release(this);
        iRenderTask = NULL;
    }
}

void BooksPageWidget::scheduleRepaint()
{
    BooksLoadingSignalBlocker block(this);
    cancelRepaint();
    if (width() > 0 && height() > 0 && !iData.isNull()) {
        iRenderTask = new RenderTask(iData, width(), height());
        iTaskQueue->submit(iRenderTask, this, SLOT(onRenderTaskDone()));
    } else {
        update();
    }
}

void BooksPageWidget::onResetTaskDone()
{
    BooksLoadingSignalBlocker block(this);
    HASSERT(sender() == iResetTask);
    iData = iResetTask->iData;
    iResetTask->iData = NULL;
    iResetTask->release(this);
    iResetTask = NULL;
    scheduleRepaint();
}

void BooksPageWidget::onRenderTaskDone()
{
    BooksLoadingSignalBlocker block(this);
    HASSERT(sender() == iRenderTask);
    iImage = iRenderTask->iImage;
    iRenderTask->release(this);
    iRenderTask = NULL;
    update();
}

void BooksPageWidget::updateSize()
{
    HDEBUG("page" << iPage << QSize(width(), height()));
    iImage.reset();
    resetView();
}

void BooksPageWidget::onWidthChanged()
{
    HDEBUG((int)width());
    // Width change will probably be followed by height change
    iResizeTimer->start();
}

void BooksPageWidget::onHeightChanged()
{
    HDEBUG((int)height());
    if (iResizeTimer->isActive()) {
        // Height is usually changed after width, repaint right away
        iResizeTimer->stop();
        updateSize();
    } else {
        iResizeTimer->start();
    }
}

void BooksPageWidget::onResizeTimeout()
{
    // This can only happen if only width or height has changed. Normally,
    // width change is followed by height change and view is reset from the
    // setHeight() method
    updateSize();
}
