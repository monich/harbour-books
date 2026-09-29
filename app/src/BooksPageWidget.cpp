/*
 * Copyright (C) 2015-2026 Slava Monich <slava@monich.com>
 * Copyright (C) 2015-2022 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "BooksPageWidget.h"
#include "BooksImageProvider.h"
#include "BooksTextStyle.h"
#include "BooksDefs.h"

#include "bookmodel/FBTextKind.h"
#include "image/ZLQtImageManager.h"
#include "ZLStringUtil.h"

#include "HarbourDebug.h"
#include "HarbourTask.h"

#include <QtGui/QClipboard>
#include <QtGui/QPainter>

static const QString IMAGE_URL("image://%1/%2");

// ==========================================================================
// BooksPageWidget::Data
// ==========================================================================

class BooksPageWidget::Data
{
public:
    typedef shared_ptr<Data> Ptr;

    Data(shared_ptr<ZLTextModel> aModel, int aWidth, int aHeight) :
        iModel(aModel), iPaintContext(aWidth, aHeight) {}

    bool paint(QPainter*, BooksColorScheme);
    inline int width() const { return iPaintContext.width(); }
    inline int height() const { return iPaintContext.height(); }

public:
    shared_ptr<BooksTextView> iView;
    shared_ptr<ZLTextModel> iModel;
    BooksPaintContext iPaintContext;
};

bool
BooksPageWidget::Data::paint(
    QPainter* aPainter,
    BooksColorScheme aColors)
{
    if (!iView.isNull()) {
        iPaintContext.iColors = aColors;
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

class BooksPageWidget::ResetTask :
    public HarbourTask
{
public:
    ResetTask(QThreadPool*, shared_ptr<ZLTextModel>, shared_ptr<ZLTextStyle>,
        int width, int height, const BooksMargins&, const BooksPos&);
    ~ResetTask();

    void performTask() Q_DECL_OVERRIDE;

public:
    BooksPageWidget::Data* iData;
    shared_ptr<ZLTextStyle> iTextStyle;
    BooksMargins iMargins;
    BooksPos iPosition;
};

BooksPageWidget::ResetTask::ResetTask(
    QThreadPool* aPool,
    shared_ptr<ZLTextModel> aModel,
    shared_ptr<ZLTextStyle> aTextStyle,
    int aWidth,
    int aHeight,
    const BooksMargins& aMargins,
    const BooksPos& aPosition) :
    HarbourTask(aPool),
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

void
BooksPageWidget::ResetTask::performTask()
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
                    return;
                }
            }
        }
        delete view;
    }
}

// ==========================================================================
// BooksPageWidget::RenderTask
// ==========================================================================

class BooksPageWidget::RenderTask :
    public HarbourTask
{
public:
    RenderTask(QThreadPool* aPool, Data::Ptr aData, BooksColorScheme aColors) :
        HarbourTask(aPool), iData(aData), iColors(aColors) {}

    void performTask() Q_DECL_OVERRIDE;

public:
    Data::Ptr iData;
    BooksColorScheme iColors;
    QImage iImage;
};

void
BooksPageWidget::RenderTask::performTask()
{
    if (!iData.isNull() && !iData->iView.isNull()) {
        const int width = iData->width();
        const int height = iData->height();

        if (width > 0 && height > 0) {
            iImage = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
            if (!isCanceled()) {
                QPainter painter(&iImage);
                iData->paint(&painter, iColors);
            }
        }
    }
}

// ==========================================================================
// BooksPageWidget::ClearSelectionTask
// ==========================================================================

class BooksPageWidget::ClearSelectionTask :
    public HarbourTask
{
public:
    ClearSelectionTask(QThreadPool* aPool, Data::Ptr aData, BooksColorScheme aColors) :
        HarbourTask(aPool), iData(aData), iColors(aColors), iImageUpdated(false) {}

    void performTask() Q_DECL_OVERRIDE;

public:
    Data::Ptr iData;
    BooksColorScheme iColors;
    QImage iImage;
    bool iImageUpdated;
};

void
BooksPageWidget::ClearSelectionTask::performTask()
{
    if (!iData.isNull() && !iData->iView.isNull()) {
        iData->iView->endSelection();
        const int width = iData->width();
        const int height = iData->height();

        if (!isCanceled() && width > 0 && height > 0) {
            const ZLTextArea& area = iData->iView->textArea();

            if (!area.selectionIsEmpty()) {
                area.clearSelection();
                iImage = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
                if (!isCanceled()) {
                    QPainter painter(&iImage);
                    iData->paint(&painter, iColors);
                    iImageUpdated = true;
                }
            }
        }
    }
}

// ==========================================================================
// BooksPageWidget::StartSelectionTask
// ==========================================================================

class BooksPageWidget::StartSelectionTask :
    public HarbourTask
{
public:
    StartSelectionTask(QThreadPool* aPool, Data::Ptr aData, int aX, int aY,
        BooksColorScheme aColors) : HarbourTask(aPool), iData(aData),
        iX(aX), iY(aY), iColors(aColors), iSelectionEmpty(true) {}

    void performTask() Q_DECL_OVERRIDE;

public:
    Data::Ptr iData;
    const int iX;
    const int iY;
    const BooksColorScheme iColors;
    QImage iImage;
    bool iSelectionEmpty;
};

void
BooksPageWidget::StartSelectionTask::performTask()
{
    if (!iData.isNull() && !iData->iView.isNull()) {
        const int width = iData->width();
        const int height = iData->height();

        if (width > 0 && height > 0) {
            iData->iView->startSelection(iX, iY);
            iSelectionEmpty = iData->iView->textArea().selectionIsEmpty();
            if (!isCanceled()) {
                iImage = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
                if (!isCanceled()) {
                    QPainter painter(&iImage);
                    iData->paint(&painter, iColors);
                }
            }
        }
    }
}

// ==========================================================================
// BooksPageWidget::ExtendSelectionTask
// ==========================================================================

class BooksPageWidget::ExtendSelectionTask :
    public HarbourTask
{
public:
    ExtendSelectionTask(QThreadPool* aPool, Data::Ptr aData, int aX, int aY,
        BooksColorScheme aColors) : HarbourTask(aPool), iData(aData),
        iX(aX), iY(aY), iColors(aColors), iSelectionChanged(false),
        iSelectionEmpty(true) {}

    void performTask() Q_DECL_OVERRIDE;

public:
    Data::Ptr iData;
    const int iX;
    const int iY;
    const BooksColorScheme iColors;
    QImage iImage;
    bool iSelectionChanged;
    bool iSelectionEmpty;
};

void
BooksPageWidget::ExtendSelectionTask::performTask()
{
    if (!iData.isNull() && !iData->iView.isNull()) {
        const int width = iData->width();
        const int height = iData->height();

        if (width > 0 && height > 0) {
            iSelectionChanged = iData->iView->extendSelection(iX, iY);
            iSelectionEmpty = iData->iView->textArea().selectionIsEmpty();
            if (iSelectionChanged && !isCanceled()) {
                iImage = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
                if (!isCanceled()) {
                    QPainter painter(&iImage);
                    iData->paint(&painter, iColors);
                }
            }
        }
    }
}

// ==========================================================================
// BooksPageWidget::FootnoteTask
// ==========================================================================

class BooksPageWidget::FootnoteTask :
    public HarbourTask, ZLTextArea::Properties
{
public:
    FootnoteTask(QThreadPool* aPool, int aX, int aY, int aMaxWidth, int aMaxHeight,
        QString aPath, QString aLinkText, QString aRef,
        shared_ptr<ZLTextModel> aTextModel, shared_ptr<ZLTextStyle> aTextStyle,
        const BooksSettings* aSettings) : HarbourTask(aPool),
        iTextModel(aTextModel), iTextStyle(aTextStyle),
        iColors(aSettings->colorScheme()),
        iX(aX), iY(aY), iMaxWidth(aMaxWidth), iMaxHeight(aMaxHeight),
        iRef(aRef), iLinkText(aLinkText), iPath(aPath) {}
    ~FootnoteTask();

    void performTask() Q_DECL_OVERRIDE;

    // ZLTextArea::Properties
    shared_ptr<ZLTextStyle> baseStyle() const Q_DECL_OVERRIDE;
    ZLColor color(const std::string&) const Q_DECL_OVERRIDE;
    bool isSelectionEnabled() const Q_DECL_OVERRIDE;

public:
    shared_ptr<ZLTextModel> iTextModel;
    shared_ptr<ZLTextStyle> iTextStyle;
    const BooksColorScheme iColors;
    const int iX;
    const int iY;
    const int iMaxWidth;
    const int iMaxHeight;
    const QString iRef;
    const QString iLinkText;
    const QString iPath;
    QImage iImage;
};

BooksPageWidget::FootnoteTask::~FootnoteTask()
{}

shared_ptr<ZLTextStyle>
BooksPageWidget::FootnoteTask::baseStyle() const
{
    return iTextStyle;
}

ZLColor
BooksPageWidget::FootnoteTask::color(
    const std::string& aStyle) const
{
    return BooksPaintContext::realColor(aStyle, iColors);
}

bool
BooksPageWidget::FootnoteTask::isSelectionEnabled() const
{
    return false;
}

void
BooksPageWidget::FootnoteTask::performTask()
{
    if (!isCanceled()) {
        // Determine the size of the footnote canvas
        ZLTextParagraphCursorCache cache;
        BooksPaintContext sizeContext(iMaxWidth, iMaxHeight, iColors);
        ZLTextAreaController sizeController(sizeContext, *this, &cache);
        ZLSize size;

        sizeController.setModel(iTextModel);
        sizeController.preparePaintInfo();
        sizeController.area().paint(&size);
        if (!size.isEmpty() && !isCanceled()) {
            // Now actually paint it
            size.myWidth = (size.myWidth + 3) & -4;
            HDEBUG("footnote size:" << size.myWidth << "x" << size.myHeight);
            cache.clear();

            BooksPaintContext paintContext(size.myWidth, size.myHeight, iColors);
            ZLTextAreaController paintController(paintContext, *this, &cache);

            iImage = QImage(size.myWidth, size.myHeight, QImage::Format_ARGB32_Premultiplied);

            QPainter painter(&iImage);

            paintContext.beginPaint(&painter);
            paintContext.clear(ZLColor(0 /* transparent */));
            paintController.setModel(iTextModel);
            paintController.preparePaintInfo();
            paintController.area().paint();
            paintContext.endPaint();
        }
    }
}

// ==========================================================================
// BooksPageWidget::PressTask
// ==========================================================================

class BooksPageWidget::PressTask :
    public HarbourTask
{
public:
    PressTask(QThreadPool* aPool, Data::Ptr aData, int aX, int aY) :
        HarbourTask(aPool), iData(aData), iX(aX), iY(aY), iKind(REGULAR) {}

    void performTask() Q_DECL_OVERRIDE;
    QString getLinkText(ZLTextWordCursor&);

public:
    Data::Ptr iData;
    const int iX;
    const int iY;
    QRect iRect;
    ZLTextKind iKind;
    std::string iLink;
    std::string iLinkType;
    std::string iImageId;
    QString iLinkText;
    QImage iImage;
};

QString
BooksPageWidget::PressTask::getLinkText(
    ZLTextWordCursor& aCursor)
{
    QString text;

    while (!aCursor.isEndOfParagraph() && !isCanceled() &&
           aCursor.element().kind() != ZLTextElement::WORD_ELEMENT) {
        aCursor.nextWord();
    }
    while (!aCursor.isEndOfParagraph() && !isCanceled() &&
           aCursor.element().kind() == ZLTextElement::WORD_ELEMENT) {
        const ZLTextWord& word = (ZLTextWord&)aCursor.element();
        if (!text.isEmpty()) text.append(' ');
        text.append(QString::fromUtf8(word.Data, word.Size));
        aCursor.nextWord();
    }
    return text;
}

void
BooksPageWidget::PressTask::performTask()
{
    if (!isCanceled()) {
        const BooksTextView& view = *iData->iView;
        const ZLTextArea& area = view.textArea();
        const ZLTextElementRectangle* rect = area.elementByCoordinates(iX, iY);

        if (rect && !isCanceled()) {
            iRect.setLeft(rect->XStart);
            iRect.setRight(rect->XEnd);
            iRect.setTop(rect->YStart);
            iRect.setBottom(rect->YEnd);
            iRect.translate(view.leftMargin(), view.topMargin());
            if (rect->Kind == ZLTextElement::WORD_ELEMENT) {
                ZLTextWordCursor cursor = area.startCursor();

                cursor.moveToParagraph(rect->ParagraphIndex);
                cursor.moveTo(rect->ElementIndex, 0);

                // Basically, the idea is that we are going backwards
                // looking for the START of the link. If we have crossed
                // the END of the linked element, it means that we have
                // missed it. We are recording which stop control elements
                // we have encountered, to avoid making assumptions which
                // ones are links and which are not. We rely on isHyperlink()
                // method to tell us the ultimate truth.
                bool stopped[NUM_KINDS];

                memset(stopped, 0, sizeof(stopped));
                while (!cursor.isStartOfParagraph() && !isCanceled()) {
                    cursor.previousWord();

                    const ZLTextElement& element = cursor.element();

                    if (element.kind() == ZLTextElement::CONTROL_ELEMENT) {
                        const ZLTextControlEntry& entry =
                            ((ZLTextControlElement&)element).entry();
                        ZLTextKind kind = entry.kind();

                        if (kind < NUM_KINDS && !entry.isStart()) {
                            stopped[kind] = true;
                        }
                        if (entry.isHyperlink()) {
                            if (entry.isStart() && !stopped[entry.kind()]) {
                                const ZLTextHyperlinkControlEntry& link =
                                    (ZLTextHyperlinkControlEntry&) entry;
                                iKind = kind;
                                iLink = link.label();
                                iLinkType = link.hyperlinkType();
                                iLinkText = getLinkText(cursor);
                                HDEBUG("link" << kind << iLinkText <<
                                    iLink.c_str());
                            }
                            return;
                        }
                    }
                }
            } else if (rect->Kind == ZLTextElement::IMAGE_ELEMENT) {
                ZLTextWordCursor cursor = area.startCursor();

                cursor.moveToParagraph(rect->ParagraphIndex);
                cursor.moveTo(rect->ElementIndex, 0);
                const ZLTextElement& element = cursor.element();

                HASSERT(element.kind() == ZLTextElement::IMAGE_ELEMENT);
                if (element.kind() == ZLTextElement::IMAGE_ELEMENT) {
                    const ZLTextImageElement& imageElement =
                        (const ZLTextImageElement&)element;
                    shared_ptr<ZLImageData> data = imageElement.image();

                    if (!data.isNull()) {
                        const QImage* image = ((ZLQtImageData&)(*data)).image();

                        if (image && !image->isNull()) {
                            iKind = IMAGE;
                            iImage = *image;
                            iImageId = imageElement.id();
                            HDEBUG("image element" << iImageId.c_str() <<
                                iImage.width() << iImage.height());
                        }
                    }
                }
            }
        }
    }
}

// ==========================================================================
// BooksPageWidget
// ==========================================================================

BooksPageWidget::BooksPageWidget(QQuickItem* aParent) :
    QQuickPaintedItem(aParent),
    iSettings(BooksSettings::sharedInstance()),
    iTaskQueue(BooksTaskQueue::defaultQueue()),
    iTextStyle(BooksTextStyle::defaults()),
    iBackgroundColor(iSettings->pageBackgroundColor()),
    iModel(Q_NULLPTR),
    iResetTask(Q_NULLPTR),
    iRenderTask(Q_NULLPTR),
    iClearSelectionTask(Q_NULLPTR),
    iStartSelectionTask(Q_NULLPTR),
    iPressTask(Q_NULLPTR),
    iLongPressTask(Q_NULLPTR),
    iFootnoteTask(Q_NULLPTR),
    iEmpty(false),
    iPressed(false),
    iSelecting(false),
    iSelectionEmpty(true),
    iCurrentPage(false),
    iPage(-1)
{
    connect(iSettings.data(), SIGNAL(colorSchemeChanged()), SLOT(onColorsChanged()));
    setFlag(ItemHasContents, true);
    connect(this, SIGNAL(widthChanged()), SLOT(onWidthChanged()));
    connect(this, SIGNAL(heightChanged()), SLOT(onHeightChanged()));
}

BooksPageWidget::~BooksPageWidget()
{
    HDEBUG("page" << iPage);
    releaseExtendSelectionTasks();
    if (iResetTask) iResetTask->release();
    if (iRenderTask) iRenderTask->release();
    if (iClearSelectionTask) iClearSelectionTask->release();
    if (iStartSelectionTask) iStartSelectionTask->release();
    if (iPressTask) iPressTask->release();
    if (iLongPressTask) iLongPressTask->release();
    if (iFootnoteTask) iFootnoteTask->release();
}

void
BooksPageWidget::releaseExtendSelectionTasks()
{
    while (!iExtendSelectionTasks.isEmpty()) {
        const int i = iExtendSelectionTasks.count()-1;

        iExtendSelectionTasks.at(i)->release();
        iExtendSelectionTasks.removeAt(i);
    }
}

void
BooksPageWidget::setPressed(
    bool aPressed)
{
    if (iPressed != aPressed) {
        iPressed = aPressed;
        if (!iPressed && iSelecting) {
            HDEBUG("page" << iPage << "leaving selection mode");
            iSelecting = false;
            Q_EMIT selectingChanged();
        }
        Q_EMIT pressedChanged();
    }
}

void
BooksPageWidget::setCurrentPage(
    bool aCurrentPage)
{
    if (iCurrentPage != aCurrentPage) {
        iCurrentPage = aCurrentPage;
        HDEBUG("page" << iPage << iCurrentPage);
        if (!iCurrentPage) clearSelection();
        Q_EMIT currentPageChanged();
    }
}

void
BooksPageWidget::setModel(
    BooksBookModel* aModel)
{
    if (iModel != aModel) {
        if (iModel) iModel->disconnect(this);
        iModel = aModel;
        if (iModel) {
#if HARBOUR_DEBUG
            if (iPage >= 0) {
                HDEBUG(iModel->title() << iPage);
            } else {
                HDEBUG(iModel->title());
            }
#endif // HARBOUR_DEBUG
            iTextStyle = iModel->textStyle();
            connect(iModel, SIGNAL(destroyed()),
                SLOT(onBookModelDestroyed()));
            connect(iModel, SIGNAL(textStyleChanged()),
                SLOT(onTextStyleChanged()));
        } else {
            iTextStyle = BooksTextStyle::defaults();
        }
        resetView();
        Q_EMIT modelChanged();
    }
}

void
BooksPageWidget::onTextStyleChanged()
{
    HDEBUG(iPage);
    HASSERT(sender() == iModel);
    iTextStyle = iModel->textStyle();
    resetView();
}

void
BooksPageWidget::onColorsChanged()
{
    HDEBUG(iPage);
    HASSERT(sender() == iSettings);
    scheduleRepaint();
}

void
BooksPageWidget::onBookModelDestroyed()
{
    BooksLoadingSignalBlocker block(this);

    HDEBUG("model destroyed");
    HASSERT(iModel == sender());
    iModel = Q_NULLPTR;
    Q_EMIT modelChanged();
    resetView();
}

void
BooksPageWidget::setPage(
    int aPage)
{
    if (iPage != aPage) {
        BooksLoadingSignalBlocker block(this);

        iPage = aPage;
        HDEBUG(iPage);
        Q_EMIT pageChanged();
    }
}

void
BooksPageWidget::setBookPos(
    const BooksPos& aBookPos)
{
    if (iBookPos != aBookPos) {
        iBookPos = aBookPos;
        HDEBUG("page" << iPage << iBookPos);
        resetView();
        Q_EMIT bookPosChanged();
    }
}

void
BooksPageWidget::setLeftMargin(
    int aMargin)
{
    if (iMargins.iLeft != aMargin) {
        iMargins.iLeft = aMargin;
        HVERBOSE(aMargin);
        resetView();
        Q_EMIT leftMarginChanged();
    }
}

void
BooksPageWidget::setRightMargin(
    int aMargin)
{
    if (iMargins.iRight != aMargin) {
        iMargins.iRight = aMargin;
        HVERBOSE(aMargin);
        resetView();
        Q_EMIT rightMarginChanged();
    }
}

void
BooksPageWidget::setTopMargin(
    int aMargin)
{
    if (iMargins.iTop != aMargin) {
        iMargins.iTop = aMargin;
        HVERBOSE(aMargin);
        resetView();
        Q_EMIT topMarginChanged();
    }
}

void
BooksPageWidget::setBottomMargin(
    int aMargin)
{
    if (iMargins.iBottom != aMargin) {
        iMargins.iBottom = aMargin;
        HVERBOSE(aMargin);
        resetView();
        Q_EMIT bottomMarginChanged();
    }
}

void
BooksPageWidget::paint(
    QPainter* aPainter)
{
    if (!iImage.isNull()) {
        HDEBUG("page" << iPage);
        aPainter->drawImage(0, 0, iImage);
        iEmpty = false;
    } else if (iPage >= 0 && iBookPos.valid() && !iData.isNull()) {
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

bool
BooksPageWidget::loading() const
{
    return iPage >= 0 && iImage.isNull() && (iResetTask || iRenderTask);
}

void
BooksPageWidget::resetView()
{
    BooksLoadingSignalBlocker block(this);

    if (iResetTask) {
        iResetTask->release();
        iResetTask = Q_NULLPTR;
    }
    if (iPressTask) {
        iPressTask->release();
        iPressTask = Q_NULLPTR;
    }
    if (iLongPressTask) {
        iLongPressTask->release();
        iLongPressTask = Q_NULLPTR;
    }
    if (iFootnoteTask) {
        iFootnoteTask->release();
        iFootnoteTask = Q_NULLPTR;
    }
    iImage = QImage();
    iData.reset();
    if (iPage >= 0 && iBookPos.valid() &&
        width() > 0 && height() > 0 && iModel) {
        shared_ptr<ZLTextModel> textModel = iModel->bookTextModel();
        if (!textModel.isNull()) {
            (iResetTask = new ResetTask(iTaskQueue->pool(), textModel, iTextStyle,
                width(), height(), iMargins, iBookPos))->
                submit(this, SLOT(onResetTaskDone()));
            cancelRepaint();
        }
    }
    if (!iEmpty) {
        updateNow();
    }
}

void
BooksPageWidget::cancelRepaint()
{
    BooksLoadingSignalBlocker block(this);

    if (iRenderTask) {
        iRenderTask->release();
        iRenderTask = Q_NULLPTR;
    }
}

void
BooksPageWidget::scheduleRepaint()
{
    BooksLoadingSignalBlocker block(this);

    cancelRepaint();
    if (width() > 0 && height() > 0) {
        if (!iData.isNull() && !iData->iView.isNull()) {
            (iRenderTask = new RenderTask(iTaskQueue->pool(), iData,
                iSettings->colorScheme()))->submit(this,
                     SLOT(onRenderTaskDone()));
        } else {
            updateNow();
        }
    }
}

void
BooksPageWidget::updateNow()
{
    iDelayUpdateTimer.stop();
    update();
}

void
BooksPageWidget::onResetTaskDone()
{
    BooksLoadingSignalBlocker block(this);

    HASSERT(sender() == iResetTask);
    iData = iResetTask->iData;
    iResetTask->iData = Q_NULLPTR;
    iResetTask->release();
    iResetTask = Q_NULLPTR;
    scheduleRepaint();
}

void
BooksPageWidget::renderTaskDone()
{
    RenderTask* task = iRenderTask;
    const QColor bg(task->iColors.background());

    HASSERT(sender() == task);
    iRenderTask = Q_NULLPTR;
    iImage = task->iImage;
    if (iBackgroundColor != bg) {
        iBackgroundColor = bg;
        Q_EMIT backgroundColorChanged();
    }
    task->release();
}

void
BooksPageWidget::onRenderTaskDone()
{
    BooksLoadingSignalBlocker block(this);
    renderTaskDone();
    updateNow();
}

void
BooksPageWidget::timerEvent(
    QTimerEvent* aEvent)
{
    const int timerId = aEvent->timerId();

    if (timerId == iResizeTimer.timerId()) {
        // This can only happen if only width or height has changed.
        // Normally, width change is followed by height change and
        // the size is updated from the setHeight() method
        updateSize();
    } else if (timerId == iDelayUpdateTimer.timerId()) {
        updateNow();
    } else {
        return QQuickPaintedItem::timerEvent(aEvent);
    }
}

void
BooksPageWidget::onPressTaskDone()
{
    HASSERT(sender() == iPressTask);
    HDEBUG(iPressTask->iKind);

    PressTask* task = iPressTask;
    iPressTask = Q_NULLPTR;

    if (task->iKind != REGULAR) {
        Q_EMIT activeTouch(task->iX, task->iY);
    }

    task->release();
}

void
BooksPageWidget::onClearSelectionTaskDone()
{
    ClearSelectionTask* task = iClearSelectionTask;

    HASSERT(sender() == task);
    iClearSelectionTask = Q_NULLPTR;

    if (!iSelectionEmpty) {
        iSelectionEmpty = true;
        HDEBUG("selection cleared");
        Q_EMIT selectionEmptyChanged();
    }

    if (task->iImageUpdated) {
        iImage = task->iImage;
        updateNow();
    }

    task->release();
}

void
BooksPageWidget::onStartSelectionTaskDone()
{
    StartSelectionTask* task = iStartSelectionTask;

    HASSERT(sender() == task);
    iStartSelectionTask = Q_NULLPTR;

    if (iPressed) {
        bool emitSelectionEmpty;

        iImage = task->iImage;
        // Emit signals when we are in a consistent state
        if (iSelectionEmpty != task->iSelectionEmpty) {
            iSelectionEmpty = task->iSelectionEmpty;
            HDEBUG("selection" << iSelectionEmpty);
            emitSelectionEmpty = true;
        }
        if (!iSelecting) {
            iSelecting = true;
            HDEBUG("entering selection mode");
            Q_EMIT selectingChanged();
        }
        if (emitSelectionEmpty) {
            Q_EMIT selectionEmptyChanged();
        }
        updateNow();
    }

    task->release();
}

void
BooksPageWidget::onExtendSelectionTaskDone()
{
    ExtendSelectionTask* task = (ExtendSelectionTask*)sender();

    HASSERT(iExtendSelectionTasks.contains(task));
    iExtendSelectionTasks.removeOne(task);

    if (iSelecting && task->iSelectionChanged) {
        iImage = task->iImage;
        if (iSelectionEmpty != task->iSelectionEmpty) {
            iSelectionEmpty = task->iSelectionEmpty;
            HDEBUG("selection" << iSelectionEmpty);
            Q_EMIT selectionEmptyChanged();
        }
        updateNow();
    }

    task->release();
}

void
BooksPageWidget::onFootnoteTaskDone()
{
    FootnoteTask* task = iFootnoteTask;

    HASSERT(sender() == task);
    iFootnoteTask = Q_NULLPTR;
    if (!task->iImage.isNull()) {
        // Footnotes with normal and inverted background need to
        // have different ids so that the cached image with the wrong
        // background doesn't show up after we invert the colors
        static const QString FOOTNOTE_ID("footnote/%1#%2?p=%3&c=%4&s=%5x%6");
        const QString id = FOOTNOTE_ID.arg(task->iPath, task->iRef).
            arg(iPage).arg(task->iColors.schemeId()).
            arg(task->iImage.width()).arg(task->iImage.height());
        const QString url = IMAGE_URL.arg(BooksImageProvider::PROVIDER_ID, id);

        HDEBUG(url);
        BooksImageProvider::instance()->addImage(iModel, id, task->iImage);
        Q_EMIT showFootnote(task->iX, task->iY, task->iLinkText, url);
    }

    task->release();
}

void
BooksPageWidget::onLongPressTaskDone()
{
    PressTask* task = iLongPressTask;

    HASSERT(sender() == task);
    HDEBUG(iLongPressTask->iKind);
    iLongPressTask = Q_NULLPTR;

    if (task->iKind == EXTERNAL_HYPERLINK) {
        static const std::string HTTP("http://");
        static const std::string HTTPS("https://");

        if (ZLStringUtil::stringStartsWith(task->iLink, HTTP) ||
            ZLStringUtil::stringStartsWith(task->iLink, HTTPS)) {
            QString url(QString::fromStdString(task->iLink));
            Q_EMIT browserLinkPressed(url);
        }
    } else if (task->iKind == INTERNAL_HYPERLINK) {
        if (iModel) {
            BooksPos pos = iModel->linkPosition(task->iLink);

            if (pos.valid()) {
                HDEBUG("link to" << pos);
                Q_EMIT pushPosition(pos);
            }
        }
    } else if (task->iKind == FOOTNOTE) {
        if (iModel && task->iLink.length() > 0) {
            shared_ptr<ZLTextModel> note = iModel->footnoteModel(task->iLink);
            BooksBook* book = iModel->book();

            if (!note.isNull() && book) {
                // Render the footnote
                HDEBUG("footnote" << QString(task->iLink.c_str()));
                if (iFootnoteTask) iFootnoteTask->release();
                (iFootnoteTask = new FootnoteTask(iTaskQueue->pool(),
                    task->iX, task->iY, width()*3/4, height()*10, book->path(),
                    task->iLinkText, QString::fromStdString(task->iLink), note,
                    iTextStyle, iSettings.data()))->
                    submit(this, SLOT(onFootnoteTaskDone()));
            } else {
                HDEBUG("bad footnote" << QString(task->iLink.c_str()));
            }
        }
    } else if (task->iKind == IMAGE) {
        // Make sure that the book path is mixed into the image id to handle
        // the case of different books having images with identical ids
        QString imageId = QString::fromStdString(task->iImageId);
        QString path;

        if (iModel) {
            BooksBook* book = iModel->book();
            if (book) {
                path = book->path();
                if (!path.isEmpty()) {
                    if (!imageId.contains(path)) {
                        QString old = imageId;
                        imageId = path + ":" + old;
                        HDEBUG(old << "-> " << imageId);
                    }
                }
            }
        }

        static const QString IMAGE_ID("image/%1");
        const QString id = IMAGE_ID.arg(imageId);
        BooksImageProvider::instance()->addImage(iModel, id, task->iImage);

        Q_EMIT imagePressed(IMAGE_URL.arg(BooksImageProvider::PROVIDER_ID, id),
            task->iRect);
    } else if (!iData.isNull()) {
        if (iStartSelectionTask) iStartSelectionTask->release();
        (iStartSelectionTask = new StartSelectionTask(iTaskQueue->pool(), iData,
            task->iX, task->iY, iSettings->colorScheme()))->
                submit(this, SLOT(onStartSelectionTaskDone()));
    }

    task->release();
}

void
BooksPageWidget::updateSize()
{
    HDEBUG("page" << iPage << QSize(width(), height()));
    iResizeTimer.stop();
    iImage = QImage();
    resetView();
}

void
BooksPageWidget::onWidthChanged()
{
    HVERBOSE((int)width());
    // Width change will probably be followed by height change
    iResizeTimer.start(0, this);
    iImage = QImage();
    updateNow();
}

void
BooksPageWidget::onHeightChanged()
{
    HVERBOSE((int)height());
    if (iResizeTimer.isActive()) { // Started by onWidthChanged()
        // Height is usually changed after width, repaint right away
        updateSize();
    } else {
        iResizeTimer.start(0, this);
        iImage = QImage();
        updateNow();
    }
}

void
BooksPageWidget::handleLongPress(
    int aX,
    int aY)
{
    HDEBUG(aX << aY);
    if (!iResetTask && !iRenderTask && !iData.isNull()) {
        if (iLongPressTask) iLongPressTask->release();
        (iLongPressTask = new PressTask(iTaskQueue->pool(), iData, aX, aY))->
            submit(this, SLOT(onLongPressTaskDone()));
    }
}

void
BooksPageWidget::handlePress(
    int aX,
    int aY)
{
    HDEBUG(aX << aY);
    if (!iResetTask && !iRenderTask && !iData.isNull()) {
        if (iPressTask) iPressTask->release();
        (iPressTask = new PressTask(iTaskQueue->pool(), iData, aX, aY))->
            submit(this, SLOT(onPressTaskDone()));
    }
}

void
BooksPageWidget::handlePositionChanged(
    int aX,
    int aY)
{
    if (iSelecting && !iData.isNull()) {
        ExtendSelectionTask* task;

        HDEBUG(aX << aY);
        // Drop the tasks which haven't been started yet
        for (int i = iExtendSelectionTasks.count()-1; i >= 0; i--) {
            task = iExtendSelectionTasks.at(i);
            if (task->isStarted()) {
                break;
            } else {
                task->release();
                iExtendSelectionTasks.removeAt(i);
                HDEBUG("dropped queued task," << i << "left");
            }
        }
        (task = new ExtendSelectionTask(iTaskQueue->pool(), iData,
            aX, aY, iSettings->colorScheme()))->
                submit(this, SLOT(onExtendSelectionTaskDone()));
        iExtendSelectionTasks.append(task);
    } else {
        // Finger was moved before we entered selection mode
        if (iStartSelectionTask) {
            iStartSelectionTask->release();
            iStartSelectionTask = Q_NULLPTR;
            HDEBUG("oops");
        }
    }
}

void
BooksPageWidget::clearSelection()
{
    if (!iData.isNull()) {
        if (iClearSelectionTask) iClearSelectionTask->release();
        (iClearSelectionTask =new ClearSelectionTask(iTaskQueue->pool(),
            iData, iSettings->colorScheme()))->
                submit(this, SLOT(onClearSelectionTaskDone()));
    }
    if (iSelecting) {
        iSelecting = false;
        Q_EMIT selectingChanged();
    }
}
