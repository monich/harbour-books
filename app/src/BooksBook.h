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

#ifndef BOOKS_BOOK_H
#define BOOKS_BOOK_H

#include "BooksItem.h"
#include "BooksPos.h"
#include "BooksStorage.h"
#include "BooksSaveTimer.h"
#include "BooksTaskQueue.h"

#include "ZLImageManager.h"
#include "formats/FormatPlugin.h"
#include "library/Book.h"

#include <QImage>
#include <QtQml>

class BooksBook : public QObject, public BooksItem
{
    class HashTask;
    class CoverTask;
    class LoadCoverTask;
    class GuessCoverTask;
    class CoverPaintContext;

    Q_OBJECT
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(QString name READ title CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString authors READ authors CONSTANT)
    Q_PROPERTY(BooksBook* book READ book CONSTANT)
    Q_PROPERTY(bool accessible READ accessible NOTIFY accessibleChanged)
    Q_PROPERTY(bool loadingCover READ loadingCover NOTIFY loadingCoverChanged)
    Q_PROPERTY(bool copyingOut READ copyingOut NOTIFY copyingOutChanged)
    Q_PROPERTY(bool fontSizeAdjust READ fontSizeAdjust WRITE setFontSizeAdjust NOTIFY fontSizeAdjustChanged)

public:
    explicit BooksBook(QObject* aParent = Q_NULLPTR);
    BooksBook(const BooksStorage& aStorage, QString aRelativePath,
        shared_ptr<Book> aBook);
    ~BooksBook();

    static BooksBook* newBook(QString aFullPath);
    static BooksBook* newBook(const BooksStorage& aStorage, QString aRelPath,
        QString aFileName);

    QString title() const;
    QString authors() const;
    QByteArray hash() const;
    int fontSizeAdjust() const;
    bool setFontSizeAdjust(int aFontSizeAdjust);
    int pageStackPos() const;
    BooksPos::List pageStack() const;
    void setPageStack(BooksPos::List aStack, int aStackPos);
    shared_ptr<Book> bookRef() const;
    QString storageFile(QString aSuffix) const;

    bool copyingOut() const;
    bool loadingCover() const;
    bool hasCoverImage() const;
    bool requestCoverImage();
    void cancelCoverRequest();
    void setCoverImage(QImage aImage);
    QImage coverImage() const;
    void setCopyingOut(bool aValue);

    // BooksItem
    virtual BooksItem* retain();
    virtual void release();
    virtual QObject* object();
    virtual BooksShelf* shelf();
    virtual BooksBook* book();
    virtual QString name() const;
    virtual QString fileName() const;
    virtual QString path() const;
    virtual bool accessible() const;
    virtual void deleteFiles();
    virtual BooksItem* copyTo(const BooksStorage& aStorage, QString aRelPath,
        CopyOperation* aObserver);

Q_SIGNALS:
    void coverImageChanged();
    void loadingCoverChanged();
    void accessibleChanged();
    void copyingOutChanged();
    void fontSizeAdjustChanged();
    void hashChanged();
    void movedAway();

private Q_SLOTS:
    void onLoadCoverTaskDone();
    void onGuessCoverTaskDone();
    void onHashTaskDone();
    void saveState();

private:
    void init();
    bool coverTaskDone();
    bool makeLink(QString aDestPath);
    void requestSave();
    QString cachedImagePath() const;
    static bool isCanceled(CopyOperation* aOperation);

private:
    QAtomicInt iRef;
    int iFontSizeAdjust;
    int iPageStackPos;
    BooksPos::List iPageStack;
    BooksStorage iStorage;
    shared_ptr<Book> iBook;
    QImage iCoverImage;
    shared_ptr<FormatPlugin> iFormatPlugin;
    shared_ptr<BooksTaskQueue> iTaskQueue;
    shared_ptr<BooksTaskQueue> iHashTaskQueue;
    BooksSaveTimer* iSaveTimer;
    CoverTask* iCoverTask;
    HashTask* iHashTask;
    bool iCoverTasksDone;
    bool iCopyingOut;
    QString iTitle;
    QString iAuthors;
    QString iPath;
    QString iFileName;
    QString iStateDir;
    QString iStateFilePath;
    QString iStateFileBase;
    QByteArray iHash;
};

QML_DECLARE_TYPE(BooksBook)

inline QString BooksBook::title() const
    { return iTitle; }
inline QString BooksBook::authors() const
    { return iAuthors; }
inline QByteArray BooksBook::hash() const
    { return iHash; }
inline int BooksBook::fontSizeAdjust() const
    { return iFontSizeAdjust; }
inline int BooksBook::pageStackPos() const
    { return iPageStackPos; }
inline BooksPos::List BooksBook::pageStack() const
    { return iPageStack; }
inline shared_ptr<Book> BooksBook::bookRef() const
    { return iBook; }
inline bool BooksBook::copyingOut() const
    { return iCopyingOut; }
inline bool BooksBook::loadingCover() const
    { return !iCoverTasksDone; }
inline bool BooksBook::isCanceled(CopyOperation* aObserver)
    { return aObserver && aObserver->isCanceled(); }
inline QImage BooksBook::coverImage() const
    { return iCoverImage; }
inline QString BooksBook::storageFile(QString aSuffix) const
    { return iStateFileBase + aSuffix; }

#endif // BOOKS_BOOK_H
