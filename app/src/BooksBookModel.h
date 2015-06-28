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

#ifndef BOOKS_BOOK_MODEL_H
#define BOOKS_BOOK_MODEL_H

#include "BooksBook.h"
#include "BooksTask.h"
#include "BooksTaskQueue.h"
#include "BooksTextView.h"
#include "BooksSettings.h"
#include "BooksPos.h"
#include "BooksPaintContext.h"
#include "BooksLoadingProperty.h"

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
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(int rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(int topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
    Q_PROPERTY(int bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
    Q_PROPERTY(BooksBook* book READ book WRITE setBook NOTIFY bookChanged)
    Q_PROPERTY(BooksSettings* settings READ settings WRITE setSettings NOTIFY settingsChanged)

public:
    explicit BooksBookModel(QObject* aParent = NULL);
    ~BooksBookModel();

    bool loading() const;
    int pageCount() const;
    int progress() const { return iProgress; }
    QString title() const { return iTitle; }
    int width() const { return iSize.width(); }
    int height() const { return iSize.height(); }

    QSize size() const { return iSize; }
    void setSize(QSize aSize);

    int currentPage() const { return iCurrentPage; }
    void setCurrentPage(int aPage);

    BooksBook* book() const { return iBook; }
    void setBook(BooksBook* aBook);

    BooksSettings* settings() const { return iSettings; }
    void setSettings(BooksSettings* aModel);

    int leftMargin() const { return iMargins.iLeft; }
    int rightMargin() const { return iMargins.iRight; }
    int topMargin() const { return iMargins.iTop; }
    int bottomMargin() const { return iMargins.iBottom; }

    void setLeftMargin(int aMargin);
    void setRightMargin(int aMargin);
    void setTopMargin(int aMargin);
    void setBottomMargin(int aMargin);

    BooksPos::List pageMarks() const;
    BooksPos pageMark(int aPage) const;
    BooksMargins margins() const { return iMargins; }
    shared_ptr<Book> bookRef() const { return iBookRef; }
    shared_ptr<BookModel> bookModel() const;
    shared_ptr<ZLTextModel> bookTextModel() const;
    shared_ptr<ZLTextModel> contentsModel() const;
    shared_ptr<ZLTextStyle> textStyle() const { return iTextStyle; }

    // QAbstractListModel
    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& aParent) const;
    virtual QVariant data(const QModelIndex& aIndex, int aRole) const;

private:
    void updateSize();
    void updateModel(int aPrevPageCount);
    void startReset(bool aFullReset = true);

private Q_SLOTS:
    void onResetProgress(int aProgress);
    void onResetDone();
    void onTextStyleChanged();

Q_SIGNALS:
    void sizeChanged();
    void bookChanged();
    void bookModelChanged();
    void titleChanged();
    void loadingChanged();
    void pageCountChanged();
    void pageMarksChanged();
    void progressChanged();
    void currentPageChanged();
    void settingsChanged();
    void leftMarginChanged();
    void rightMarginChanged();
    void topMarginChanged();
    void bottomMarginChanged();
    void jumpToPage(int index);

private:
    class Data;
    class Task;

    int iCurrentPage;
    int iProgress;
    QSize iSize;
    QString iTitle;
    BooksMargins iMargins;
    BooksBook* iBook;
    shared_ptr<Book> iBookRef;
    BooksSettings* iSettings;
    Task* iTask;
    Data* iData;
    Data* iData2;
    shared_ptr<BooksTaskQueue> iTaskQueue;
    shared_ptr<ZLTextStyle> iTextStyle;
};

QML_DECLARE_TYPE(BooksBookModel)

#endif // BOOKS_BOOK_MODEL_H
