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

#ifndef BOOKS_BOOK_MODEL_H
#define BOOKS_BOOK_MODEL_H

#include "BooksBook.h"
#include "BooksTaskQueue.h"
#include "BooksTextView.h"
#include "BooksSettings.h"
#include "BooksPos.h"
#include "BooksPaintContext.h"
#include "BooksLoadingProperty.h"
#include "BooksPageStack.h"

#include "ZLTextStyle.h"
#include "bookmodel/BookModel.h"

#include <QHash>
#include <QList>
#include <QVariant>
#include <QByteArray>
#include <QAbstractListModel>
#include <QtQml>
#include <QQuickItem>

class BooksBookModel: public QAbstractListModel, private BooksLoadingProperty
{
    Q_OBJECT
    Q_ENUMS(ResetReason)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(int leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(int rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(int topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
    Q_PROPERTY(int bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
    Q_PROPERTY(BooksPageStack* pageStack READ pageStack CONSTANT)
    Q_PROPERTY(BooksBook* book READ book WRITE setBook NOTIFY bookChanged)
    Q_PROPERTY(ResetReason resetReason READ resetReason NOTIFY resetReasonChanged)

public:
    // NOTE: These have to match the labels in BooksBookView.qml
    enum ResetReason {
        ReasonUnknown,
        ReasonLoading,
        ReasonIncreasingFontSize,
        ReasonDecreasingFontSize
    };

    Q_INVOKABLE bool increaseFontSize();
    Q_INVOKABLE bool decreaseFontSize();

    explicit BooksBookModel(QObject* aParent = NULL);
    ~BooksBookModel();

    bool loading() const;
    int pageCount() const;
    int progress() const;
    QString title() const;
    int width() const;
    int height() const;
    ResetReason resetReason() const;
    BooksPageStack* pageStack() const;

    QSize size() const;
    void setSize(QSize aSize);

    BooksBook* book() const;
    void setBook(BooksBook* aBook);

    int leftMargin() const;
    int rightMargin() const;
    int topMargin() const;
    int bottomMargin() const;

    void setLeftMargin(int aMargin);
    void setRightMargin(int aMargin);
    void setTopMargin(int aMargin);
    void setBottomMargin(int aMargin);

    BooksPos::List pageMarks() const;
    BooksPos pageMark(int aPage) const;
    BooksMargins margins() const;
    shared_ptr<Book> bookRef() const;
    shared_ptr<BookModel> bookModel() const;
    shared_ptr<ZLTextModel> bookTextModel() const;
    shared_ptr<ZLTextModel> contentsModel() const;
    shared_ptr<ZLTextModel> footnoteModel(const std::string& aId) const;
    shared_ptr<ZLTextStyle> textStyle() const;
    BooksPos linkPosition(const std::string& aLink) const;
    int fontSizeAdjust() const;

    // QAbstractListModel
    virtual QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex& aParent) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& aIndex, int aRole) const Q_DECL_OVERRIDE;

private:
    void updateSize();
    void updateModel(int aPrevPageCount);
    void startReset(ResetReason aReason = ReasonUnknown, bool aFull = true);
    void emitBookPosChanged();

private Q_SLOTS:
    void onResetProgress(int aProgress);
    void onResetDone();
    void onTextStyleChanged();
    void onPageStackChanged();
    void onHashChanged();

Q_SIGNALS:
    void loadingChanged() Q_DECL_OVERRIDE;
    void sizeChanged();
    void bookChanged();
    void bookModelChanged();
    void titleChanged();
    void pageCountChanged();
    void pageMarksChanged();
    void progressChanged();
    void leftMarginChanged();
    void rightMarginChanged();
    void topMarginChanged();
    void bottomMarginChanged();
    void resetReasonChanged();
    bool textStyleChanged();
    void jumpToPage(int page);

private:
    class Data;
    class PagingTask;

    ResetReason iResetReason;
    int iProgress;
    QSize iSize;
    QString iTitle;
    BooksMargins iMargins;
    BooksBook* iBook;
    shared_ptr<Book> iBookRef;
    PagingTask* iPagingTask;
    Data* iData;
    Data* iData2;
    QSharedPointer<BooksSettings> iSettings;
    shared_ptr<BooksTaskQueue> iTaskQueue;
    shared_ptr<ZLTextStyle> iTextStyle;
    BooksPageStack* iPageStack;
};

QML_DECLARE_TYPE(BooksBookModel)

inline int BooksBookModel::progress() const
    { return iProgress; }
inline QString BooksBookModel::title() const
    { return iTitle; }
inline int BooksBookModel::width() const
    { return iSize.width(); }
inline int BooksBookModel::height() const
    { return iSize.height(); }
inline BooksBookModel::ResetReason BooksBookModel::resetReason() const
    { return iResetReason; }
inline BooksPageStack* BooksBookModel::pageStack() const
    { return iPageStack; }
inline QSize BooksBookModel::size() const
    { return iSize; }
inline BooksBook* BooksBookModel::book() const
    { return iBook; }
inline int BooksBookModel::leftMargin() const
    { return iMargins.iLeft; }
inline int BooksBookModel::rightMargin() const
    { return iMargins.iRight; }
inline int BooksBookModel::topMargin() const
    { return iMargins.iTop; }
inline int BooksBookModel::bottomMargin() const
    { return iMargins.iBottom; }
inline BooksMargins BooksBookModel::margins() const
    { return iMargins; }
inline shared_ptr<Book> BooksBookModel::bookRef() const
    { return iBookRef; }
inline shared_ptr<ZLTextStyle> BooksBookModel::textStyle() const
    { return iTextStyle; }

#endif // BOOKS_BOOK_MODEL_H
