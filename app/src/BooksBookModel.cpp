/*
 * Copyright (C) 2015-2016 Jolla Ltd.
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

#include "BooksBookModel.h"
#include "BooksTextStyle.h"

#include "HarbourDebug.h"

#include "ZLTextHyphenator.h"

// ==========================================================================
// BooksBookModel::Data
// ==========================================================================
class BooksBookModel::Data {
public:
    Data(int aWidth, int aHeight) : iWidth(aWidth), iHeight(aHeight) {}

    int pickPage(const BooksPos& aPagePos) const;
    int pickPage(const BooksPos& aPagePos, const BooksPos& aNextPagePos,
        int aPageCount) const;

public:
    int iWidth;
    int iHeight;
    shared_ptr<BookModel> iBookModel;
    BooksPos::List iPageMarks;
};

int BooksBookModel::Data::pickPage(const BooksPos& aPagePos) const
{
    int page = 0;
    if (aPagePos.valid()) {
        BooksPos::ConstIterator it = qFind(iPageMarks, aPagePos);
        if (it == iPageMarks.end()) {
            it = qUpperBound(iPageMarks, aPagePos);
            page = (int)(it - iPageMarks.begin()) - 1;
            HDEBUG("using page" << page << "for" << aPagePos);
        } else {
            page = it - iPageMarks.begin();
            HDEBUG("found" << aPagePos << "at page" << page);
        }
    }
    return page;
}

int BooksBookModel::Data::pickPage(const BooksPos& aPagePos,
    const BooksPos& aNextPagePos, int aPageCount) const
{
    int page = 0;
    if (aPagePos.valid()) {
        if (!aNextPagePos.valid()) {
            // Last page stays the last
            page = iPageMarks.count() - 1;
            HDEBUG("last page" << page);
        } else {
            BooksPos::ConstIterator it = qFind(iPageMarks, aPagePos);
            if (it == iPageMarks.end()) {
                // Two 90-degrees rotations should return the reader
                // back to the same page. That's what this is about.
                const BooksPos& pos = (iPageMarks.count() > aPageCount) ?
                    aPagePos : aNextPagePos;
                it = qUpperBound(iPageMarks, pos);
                page = (int)(it - iPageMarks.begin());
                if (page > 0) page--;
                HDEBUG("using page" << page << "for" << pos);
            } else {
                page = it - iPageMarks.begin();
                HDEBUG("found" << aPagePos << "at page" << page);
            }
        }
    }
    return page;
}

// ==========================================================================
// BooksBookModel::Task
// ==========================================================================

class BooksBookModel::Task : public BooksTask
{
    Q_OBJECT

public:
    Task(BooksBookModel* aReceiver, shared_ptr<Book> aBook,
        const BooksPos& aPagePos, const BooksPos& aNextPagePos,
        const BooksPos& aLastPos, int aPageCount);
    ~Task();

    void performTask();

Q_SIGNALS:
    void progress(int aProgress);

public:
    shared_ptr<Book> iBook;
    shared_ptr<ZLTextStyle> iTextStyle;
    BooksMargins iMargins;
    BooksPaintContext iPaint;
    BooksBookModel::Data* iData;
    BooksPos iPagePos;
    BooksPos iNextPagePos;
    BooksPos iLastPos;
    int iOldPageCount;
    int iPage;
};

BooksBookModel::Task::Task(BooksBookModel* aModel,
    shared_ptr<Book> aBook, const BooksPos& aPagePos,
    const BooksPos& aNextPagePos, const BooksPos& aLastPos, int aPageCount) :
    iBook(aBook),
    iTextStyle(aModel->textStyle()),
    iMargins(aModel->margins()),
    iPaint(aModel->width(), aModel->height()),
    iData(NULL),
    iPagePos(aPagePos),
    iNextPagePos(aNextPagePos),
    iLastPos(aLastPos),
    iOldPageCount(aPageCount),
    iPage(-1)
{
    aModel->connect(this, SIGNAL(done()), SLOT(onResetDone()));
    aModel->connect(this, SIGNAL(progress(int)), SLOT(onResetProgress(int)),
        Qt::QueuedConnection);
}

BooksBookModel::Task::~Task()
{
    delete iData;
}

void BooksBookModel::Task::performTask()
{
    if (!isCanceled()) {
        iData = new BooksBookModel::Data(iPaint.width(), iPaint.height());
        iData->iBookModel = new BookModel(iBook);
        shared_ptr<ZLTextModel> model(iData->iBookModel->bookTextModel());
        ZLTextHyphenator::Instance().load(iBook->language());
        if (!isCanceled()) {
            BooksTextView view(iPaint, iTextStyle, iMargins);
            view.setModel(model);
            if (model->paragraphsNumber() > 0) {
                BooksPos mark = view.rewind();
                iData->iPageMarks.append(mark);
                Q_EMIT progress(iData->iPageMarks.count());
                while (!isCanceled() && view.nextPage()) {
                    mark = view.position();
                    iData->iPageMarks.append(mark);
                    Q_EMIT progress(iData->iPageMarks.count());
                }
            }
        }
    }
    if (!isCanceled()) {
        HDEBUG(iData->iPageMarks.count() << "page(s)" << qPrintable(
            QString("%1x%2").arg(iData->iWidth).arg(iData->iHeight)));
        iPage = iPagePos.valid() ?
            iData->pickPage(iPagePos, iNextPagePos, iOldPageCount) :
            iData->pickPage(iLastPos);
    } else {
        HDEBUG("giving up" << qPrintable(QString("%1x%2").arg(iPaint.width()).
            arg(iPaint.height())) << "paging");
    }
}

// ==========================================================================
// BooksBookModel
// ==========================================================================

enum BooksBookModelRole {
    BooksBookModelPageIndex = Qt::UserRole
};

BooksBookModel::BooksBookModel(QObject* aParent) :
    QAbstractListModel(aParent),
    iResetReason(ReasonUnknown),
    iCurrentPage(-1),
    iProgress(0),
    iBook(NULL),
    iTask(NULL),
    iData(NULL),
    iData2(NULL),
    iSettings(BooksSettings::sharedInstance()),
    iTaskQueue(BooksTaskQueue::defaultQueue())
{
    iTextStyle = iSettings->textStyle(fontSizeAdjust());
    connect(iSettings.data(), SIGNAL(textStyleChanged()), SLOT(onTextStyleChanged()));
    HDEBUG("created");
#if QT_VERSION < 0x050000
    setRoleNames(roleNames());
#endif
}

BooksBookModel::~BooksBookModel()
{
    if (iTask) iTask->release(this);
    if (iBook) {
        iBook->disconnect(this);
        iBook->release();
        iBook = NULL;
    }
    delete iData;
    delete iData2;
    HDEBUG("destroyed");
}

void BooksBookModel::setBook(BooksBook* aBook)
{
    shared_ptr<Book> oldBook;
    shared_ptr<Book> newBook;
    if (iBook != aBook) {
        const QString oldTitle(iTitle);
        if (iBook) {
            iBook->disconnect(this);
            iBook->release();
        }
        if (aBook) {
            (iBook = aBook)->retain();
            iBookRef = newBook;
            iTitle = iBook->title();
            iTextStyle = iSettings->textStyle(fontSizeAdjust());
            connect(iBook, SIGNAL(fontSizeAdjustChanged()), SLOT(onTextStyleChanged()));
            HDEBUG(iTitle);
        } else {
            iBook = NULL;
            iBookRef.reset();
            iTitle = QString();
            HDEBUG("<none>");
        }
        startReset(ReasonLoading, true);
        if (oldTitle != iTitle) {
            Q_EMIT titleChanged();
        }
        Q_EMIT textStyleChanged();
        Q_EMIT bookModelChanged();
        Q_EMIT bookChanged();
    }
}

bool BooksBookModel::loading() const
{
    return (iTask != NULL);
}

bool BooksBookModel::increaseFontSize()
{
    return iBook && iBook->setFontSizeAdjust(iBook->fontSizeAdjust()+1);
}

bool BooksBookModel::decreaseFontSize()
{
    return iBook && iBook->setFontSizeAdjust(iBook->fontSizeAdjust()-1);
}

void BooksBookModel::setCurrentPage(int aPage)
{
    if (iCurrentPage != aPage) {
        iCurrentPage = aPage;
        if (iData &&
            iCurrentPage >= 0 &&
            iCurrentPage < iData->iPageMarks.count()) {
            iBook->setLastPos(iData->iPageMarks.at(iCurrentPage));
            HDEBUG(aPage << iBook->lastPos());
        } else {
            HDEBUG(aPage);
        }
        Q_EMIT currentPageChanged();
    }
}

int BooksBookModel::pageCount() const
{
    return iData ? iData->iPageMarks.count() : 0;
}

BooksPos::List BooksBookModel::pageMarks() const
{
    return iData ? iData->iPageMarks : BooksPos::List();
}

int BooksBookModel::fontSizeAdjust() const
{
    return iBook ? iBook->fontSizeAdjust() : 0;
}

BooksPos BooksBookModel::pageMark(int aPage) const
{
    if (aPage >= 0 && iData) {
        const int n = iData->iPageMarks.count();
        if (aPage < n) {
            return iData->iPageMarks.at(aPage);
        }
    }
    return BooksPos();
}

int BooksBookModel::linkToPage(const std::string& aLink) const
{
    if (iData && !iData->iBookModel.isNull()) {
        BookModel::Label label = iData->iBookModel->label(aLink);
        if (label.ParagraphNumber >= 0) {
            return iData->pickPage(BooksPos(label.ParagraphNumber, 0, 0));
        }
    }
    return -1;
}

shared_ptr<BookModel> BooksBookModel::bookModel() const
{
    return iData ? iData->iBookModel : NULL;
}

shared_ptr<ZLTextModel> BooksBookModel::bookTextModel() const
{
    shared_ptr<ZLTextModel> model;
    if (iData && !iData->iBookModel.isNull()) {
        model = iData->iBookModel->bookTextModel();
    }
    return model;
}

shared_ptr<ZLTextModel> BooksBookModel::footnoteModel(const std::string& aId) const
{
    shared_ptr<ZLTextModel> model;
    if (iData && !iData->iBookModel.isNull()) {
        model = iData->iBookModel->footnoteModel(aId);
    }
    return model;
}

shared_ptr<ZLTextModel> BooksBookModel::contentsModel() const
{
    shared_ptr<ZLTextModel> model;
    if (iData && !iData->iBookModel.isNull()) {
        model = iData->iBookModel->contentsModel();
    }
    return model;
}

void BooksBookModel::setLeftMargin(int aMargin)
{
    if (iMargins.iLeft != aMargin) {
        iMargins.iLeft = aMargin;
        HDEBUG(aMargin);
        startReset();
        Q_EMIT leftMarginChanged();
    }
}

void BooksBookModel::setRightMargin(int aMargin)
{
    if (iMargins.iRight != aMargin) {
        iMargins.iRight = aMargin;
        HDEBUG(aMargin);
        startReset();
        Q_EMIT rightMarginChanged();
    }
}

void BooksBookModel::setTopMargin(int aMargin)
{
    if (iMargins.iTop != aMargin) {
        iMargins.iTop = aMargin;
        HDEBUG(aMargin);
        startReset();
        Q_EMIT topMarginChanged();
    }
}

void BooksBookModel::setBottomMargin(int aMargin)
{
    if (iMargins.iBottom != aMargin) {
        iMargins.iBottom = aMargin;
        HDEBUG(aMargin);
        startReset();
        Q_EMIT bottomMarginChanged();
    }
}

void BooksBookModel::updateModel(int aPrevPageCount)
{
    const int newPageCount = pageCount();
    if (aPrevPageCount != newPageCount) {
        if (newPageCount > aPrevPageCount) {
            beginInsertRows(QModelIndex(), aPrevPageCount, newPageCount-1);
            endInsertRows();
        } else {
            beginRemoveRows(QModelIndex(), newPageCount, aPrevPageCount-1);
            endRemoveRows();
        }
        Q_EMIT pageCountChanged();
    }
}

void BooksBookModel::setSize(QSize aSize)
{
    if (iSize != aSize) {
        iSize = aSize;
        const int w = width();
        const int h = height();
        HDEBUG(aSize);
        if (iData && iData->iWidth == w && iData->iHeight == h) {
            HDEBUG("size didn't change");
        } else if (iData2 && iData2->iWidth == w && iData2->iHeight == h) {
            HDEBUG("switching to backup layout");
            const int oldModelPageCount = pageCount();
            int oldPageCount;
            BooksPos page1, page2;
            if (iTask) {
                // Layout has been switched back before the paging task
                // has completed
                HDEBUG("not so fast please...");
                oldPageCount = iTask->iOldPageCount;
                page1 = iTask->iPagePos;
                page2 = iTask->iNextPagePos;
            } else {
                oldPageCount = oldModelPageCount;
                page1 = pageMark(iCurrentPage);
                page2 = pageMark(iCurrentPage+1);
            }
            Data* tmp = iData;
            iData = iData2;
            iData2 = tmp;
            if (iData) {
                // Cancel unnecessary paging task
                if (iTask) {
                    BooksLoadingSignalBlocker block(this);
                    iTask->release(this);
                    iTask = NULL;
                }
                updateModel(oldModelPageCount);
                Q_EMIT pageMarksChanged();
                Q_EMIT jumpToPage(iData->pickPage(page1, page2, oldPageCount));
            } else {
                startReset(ReasonUnknown, false);
            }
        } else {
            startReset(ReasonUnknown, false);
        }
        Q_EMIT sizeChanged();
    }
}

void BooksBookModel::onTextStyleChanged()
{
    HDEBUG(iTitle);
    shared_ptr<ZLTextStyle> newStyle = iSettings->textStyle(fontSizeAdjust());
    const int newFontSize = newStyle->fontSize();
    const int oldFontSize = iTextStyle->fontSize();
    const ResetReason reason =
        (newFontSize > oldFontSize) ? ReasonIncreasingFontSize :
        (newFontSize < oldFontSize) ? ReasonDecreasingFontSize :
        ReasonUnknown;
    iTextStyle = newStyle;
    startReset(reason);
    Q_EMIT textStyleChanged();
}

void BooksBookModel::startReset(ResetReason aResetReason, bool aFullReset)
{
    BooksLoadingSignalBlocker block(this);
    const BooksPos thisPage = pageMark(iCurrentPage);
    const BooksPos nextPage = pageMark(iCurrentPage+1);
    if (aResetReason == ReasonUnknown) {
        if (iResetReason == ReasonUnknown) {
            if (!iData && !iData2) {
                aResetReason = ReasonLoading;
            }
        } else {
            aResetReason = iResetReason;
        }
    }
    if (iTask) {
        iTask->release(this);
        iTask = NULL;
    }
    const int oldPageCount(pageCount());
    if (oldPageCount > 0) {
        beginResetModel();
    }

    delete iData2;
    if (aFullReset) {
        delete iData;
        iData2 = NULL;
    } else {
        iData2 = iData;
    }
    iData = NULL;

    if (iBook && width() > 0 && height() > 0) {
        HDEBUG("starting" << qPrintable(QString("%1x%2").arg(width()).
            arg(height())) << "paging");
        iTask = new Task(this, iBook->bookRef(), thisPage, nextPage,
            iBook->lastPos(), oldPageCount);
        iTaskQueue->submit(iTask);
    }

    if (oldPageCount > 0) {
        endResetModel();
        Q_EMIT pageMarksChanged();
        Q_EMIT pageCountChanged();
    }

    if (iCurrentPage != 0) {
        iCurrentPage = 0;
        Q_EMIT currentPageChanged();
    }

    if (iProgress != 0) {
        iProgress = 0;
        Q_EMIT progressChanged();
    }

    if (iResetReason != aResetReason) {
        iResetReason = aResetReason;
        Q_EMIT resetReasonChanged();
    }
}

void BooksBookModel::onResetProgress(int aProgress)
{
    // progress -> onResetProgress is a queued connection, we may received
    // this event from the task that has already been canceled.
    if (iTask == sender() && aProgress > iProgress) {
        iProgress = aProgress;
        Q_EMIT progressChanged();
    }
}

void BooksBookModel::onResetDone()
{
    HASSERT(sender() == iTask);
    HASSERT(iTask->iData);
    HASSERT(!iData);

    const int oldPageCount(pageCount());
    shared_ptr<BookModel> oldBookModel(bookModel());
    BooksLoadingSignalBlocker block(this);
    int page = iTask->iPage;

    iData = iTask->iData;
    iTask->iData = NULL;
    iTask->release(this);
    iTask = NULL;

    updateModel(oldPageCount);
    Q_EMIT jumpToPage(page);
    Q_EMIT pageMarksChanged();
    if (oldBookModel != bookModel()) {
        Q_EMIT bookModelChanged();
    }
    if (iResetReason != ReasonUnknown) {
        iResetReason = ReasonUnknown;
        Q_EMIT resetReasonChanged();
    }
}

QHash<int,QByteArray> BooksBookModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(BooksBookModelPageIndex, "pageIndex");
    return roles;
}

int BooksBookModel::rowCount(const QModelIndex&) const
{
    return pageCount();
}

QVariant BooksBookModel::data(const QModelIndex& aIndex, int aRole) const
{
    const int i = aIndex.row();
    if (i >= 0 && i < pageCount()) {
        switch (aRole) {
        case BooksBookModelPageIndex: return i;
        }
    }
    return QVariant();
}

#include "BooksBookModel.moc"
