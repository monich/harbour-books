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

#include "BooksBook.h"
#include "BooksDefs.h"
#include "BooksTask.h"
#include "BooksTextView.h"
#include "BooksTextStyle.h"
#include "BooksPaintContext.h"

#include "HarbourJson.h"
#include "HarbourDebug.h"

#include "ZLImage.h"
#include "image/ZLQtImageManager.h"
#include "bookmodel/BookModel.h"
#include "library/Author.h"

#include <QFile>
#include <QDirIterator>
#include <QGuiApplication>
#include <QScreen>

#define BOOK_STATE_POSITION  "position"
#define BOOK_COVER_SUFFIX ".cover."

// ==========================================================================
// BooksBook::CoverContext
// ==========================================================================
class BooksBook::CoverPaintContext : public BooksPaintContext {
public:
    CoverPaintContext();

    void drawImage(int x, int y, const ZLImageData& image);
    void drawImage(int x, int y, const ZLImageData& image, int width, int height, ScalingType type);
    void handleImage(const ZLImageData& image);
    bool gotIt() const;

public:
    static QSize gMaxScreenSize;
    static bool gMaxScreenSizeKnown;

    QImage iImage;
};

QSize BooksBook::CoverPaintContext::gMaxScreenSize(480,640);
bool BooksBook::CoverPaintContext::gMaxScreenSizeKnown = false;

BooksBook::CoverPaintContext::CoverPaintContext()
{
    if (!gMaxScreenSizeKnown) {
        QList<QScreen*> screens = qGuiApp->screens();
        const int n = screens.count();
        for (int i=0; i<n; i++) {
            QSize s = screens.at(i)->size();
            gMaxScreenSize.setWidth(qMax(s.width(), gMaxScreenSize.width()));
            gMaxScreenSize.setHeight(qMax(s.width(), gMaxScreenSize.height()));
        }
    }

    setWidth(gMaxScreenSize.width());
    setHeight(gMaxScreenSize.height());
}

void BooksBook::CoverPaintContext::drawImage(int x, int y, const ZLImageData& image)
{
    handleImage(image);
}

void BooksBook::CoverPaintContext::drawImage(int x, int y, const ZLImageData& image,
    int width, int height, ScalingType type)
{
    handleImage(image);
}

void BooksBook::CoverPaintContext::handleImage(const ZLImageData& image)
{
    const QImage* qImage = ((ZLQtImageData&)image).image();
    HDEBUG(image.width() << 'x' << image.height());
    if (qImage->height() > qImage->width() &&
        qImage->width() > iImage.width() &&
        qImage->height() > iImage.height()) {
        iImage = *qImage;
    }
}

bool BooksBook::CoverPaintContext::gotIt() const
{
    return iImage.width() >= 50 &&
           iImage.height() >= 80 &&
           iImage.height() > iImage.width();
}

// ==========================================================================
// BooksBook::CoverTask
// ==========================================================================

class BooksBook::CoverTask : public BooksTask
{
public:
    CoverTask(BooksStorage aStorage, shared_ptr<Book> aBook) :
        iStorage(aStorage), iBook(aBook), iCoverMissing(false) {}

    bool hasImage() const;

public:
    BooksStorage iStorage;
    shared_ptr<Book> iBook;
    QImage iCoverImage;
    bool iCoverMissing;
};

inline bool BooksBook::CoverTask::hasImage() const
{
    return iCoverImage.width() > 0 && iCoverImage.height() > 0;
}

// ==========================================================================
// BooksBook::LoadCoverTask
// ==========================================================================

class BooksBook::LoadCoverTask : public BooksBook::CoverTask
{
public:
    LoadCoverTask(BooksStorage aStorage, shared_ptr<Book> aBook,
        shared_ptr<FormatPlugin> aFormatPlugin) :
        BooksBook::CoverTask(aStorage, aBook),
        iFormatPlugin(aFormatPlugin) {}

    virtual void performTask();

public:
    shared_ptr<FormatPlugin> iFormatPlugin;
};

void BooksBook::LoadCoverTask::performTask()
{
    if (!isCanceled()) {
        // Try to load cached (or custom) cover
        if (iStorage.isValid()) {
            QString coverPrefix(QString::fromStdString(
                iBook->file().name(false)) + BOOK_COVER_SUFFIX);
            QDirIterator it(iStorage.configDir());
            while (it.hasNext()) {
                QString path(it.next());
                if (it.fileName().startsWith(coverPrefix)) {
                    if (QFile(path).size() == 0) {
                        HDEBUG("no cover for" << iBook->title().c_str());
                        iCoverMissing = true;
                        return;
                    } else if (iCoverImage.load(path)) {
                        HDEBUG("loaded cover" << iCoverImage.width() << 'x' <<
                            iCoverImage.height() << "for" <<
                            iBook->title().c_str() << "from" <<
                            qPrintable(path));
                        return;
                    }
                }
            }
        }

        // OK, fetch one from the book file
        shared_ptr<ZLImageData> imageData;
        shared_ptr<ZLImage> image = iFormatPlugin->coverImage(iBook->file());
        if (!image.isNull()) {
            imageData = ZLImageManager::Instance().imageData(*image);
        }
        if (!imageData.isNull()) {
            QImage* qImage = (QImage*)((ZLQtImageData&)*imageData).image();
            if (qImage) iCoverImage = *qImage;
        }
    }
#if HARBOUR_DEBUG
    if (hasImage()) {
        HDEBUG("loaded cover" << iCoverImage.width() << 'x' <<
            iCoverImage.height() << "for" << iBook->title().c_str());
    } else if (isCanceled()) {
        HDEBUG("cancelled" << iBook->title().c_str());
    } else {
        HDEBUG("no cover found in" << iBook->title().c_str());
    }
#endif
}

// ==========================================================================
// BooksBook::GuessCoverTask
// ==========================================================================

class BooksBook::GuessCoverTask : public BooksBook::CoverTask
{
public:
    GuessCoverTask(BooksStorage aStorage, shared_ptr<Book> aBook) :
        BooksBook::CoverTask(aStorage, aBook) {}

    virtual void performTask();
};

void BooksBook::GuessCoverTask::performTask()
{
    if (!isCanceled()) {
        BooksMargins margins;
        CoverPaintContext context;
        BookModel bookModel(iBook);
        shared_ptr<ZLTextModel> model(bookModel.bookTextModel());
        BooksTextView view(context, BooksTextStyle::defaults(), margins);
        view.setModel(model);
        view.rewind();
        if (!isCanceled()) {
            view.rewind();
            if (!isCanceled()) {
                view.paint();
                iCoverImage = context.iImage;
            }
        }
    }

    QString coverPath;
    if (iStorage.isValid()) {
        coverPath = iStorage.configDir().path() + "/" +
            QString::fromStdString(iBook->file().name(false)) +
            BOOK_COVER_SUFFIX + "jpg";
    }

    if (hasImage()) {
        HDEBUG("using image" << iCoverImage.width() << 'x' <<
            iCoverImage.width() << "as cover for" << iBook->title().c_str());

        // Save the extracted image
        if (!coverPath.isEmpty() && iCoverImage.save(coverPath)) {
            HDEBUG("saved cover to" << qPrintable(coverPath));
        }
    } else if (isCanceled()) {
        HDEBUG("cancelled" << iBook->title().c_str());
    } else {
        HDEBUG("no cover for" << iBook->title().c_str());
        iCoverMissing = true;

        // Create empty file. Guessing the cover image is an expensive task,
        // we don't want to do it every time the application is started.
        if (!coverPath.isEmpty() &&
            // Check if the book file still exists - the failure could've
            // been caused by the SD-card removal.
            QFile::exists(QString::fromStdString(iBook->file().path()))) {
            QFile(coverPath).open(QIODevice::WriteOnly);
        }
    }
}

// ==========================================================================
// BooksBook
// ==========================================================================

// This constructor isn't really used, but is required by qmlRegisterType
BooksBook::BooksBook(QObject* aParent) :
    QObject(aParent),
    iRef(-1)
{
    init();
}

BooksBook::BooksBook(const BooksStorage& aStorage, shared_ptr<Book> aBook) :
    QObject(NULL),
    iRef(1),
    iStorage(aStorage),
    iBook(aBook),
    iTaskQueue(BooksTaskQueue::instance())
{
    init();
    HASSERT(!iBook.isNull());
    iTitle = QString::fromStdString(iBook->title());
    iPath = QString::fromStdString(iBook->file().path());
    iFileName = QString::fromStdString(iBook->file().name(false));
    iFormatPlugin = PluginCollection::Instance().plugin(*iBook);
    AuthorList authors(iBook->authors());
    const int n = authors.size();
    for (int i=0; i<n; i++) {
        if (i > 0) iAuthors += ", ";
        iAuthors += QString::fromStdString(authors[i]->name());
    }
    if (iStorage.isValid()) {
        iStateFilePath = iStorage.configDir().path() + "/" +
            iFileName + BOOKS_STATE_FILE_SUFFIX;
        // Load the state
        QVariantMap state;
        if (HarbourJson::load(iStateFilePath, state)) {
            iLastPos = BooksPos::fromVariant(state.value(BOOK_STATE_POSITION));
        }
    }
    // Refcounted BooksBook objects are managed by C++ code
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

void BooksBook::init()
{
    iCoverTask = NULL;
    iCoverTasksDone = false;
    iCopyingOut = false;
    iSaveTimer = NULL;
}

BooksBook::~BooksBook()
{
    HDEBUG(qPrintable(iPath));
    HASSERT(!iRef.load());
    if (iCoverTask) iCoverTask->release(this);
}

BooksItem* BooksBook::retain()
{
    if (iRef.load() >= 0) {
        iRef.ref();
    }
    return this;
}

void BooksBook::release()
{
    if (iRef.load() >= 0 && !iRef.deref()) {
        delete this;
    }
}

QObject* BooksBook::object()
{
    return this;
}

BooksShelf* BooksBook::shelf()
{
    return NULL;
}

BooksBook* BooksBook::book()
{
    return this;
}

QString BooksBook::name() const
{
    return iTitle;
}

QString BooksBook::fileName() const
{
    return iFileName;
}

void BooksBook::setLastPos(const BooksPos& aPos)
{
    if (iLastPos != aPos) {
        iLastPos = aPos;
        // We only need save timer if we have the state file
        if (!iSaveTimer && iStorage.isValid()) {
            iSaveTimer = new BooksSaveTimer(this);
            connect(iSaveTimer, SIGNAL(save()), SLOT(saveState()));
        }
        if (iSaveTimer) iSaveTimer->requestSave();
    }
}

void BooksBook::setCopyingOut(bool aValue)
{
    if (iCopyingOut != aValue) {
        const bool wasAccessible = accessible();
        iCopyingOut = aValue;
        Q_EMIT copyingOutChanged();
        if (wasAccessible != accessible()) {
            Q_EMIT accessibleChanged();
        }
    }
}

QImage BooksBook::coverImage()
{
    return iCoverImage;
}

bool BooksBook::hasCoverImage() const
{
    return iCoverImage.width() > 0 && iCoverImage.height() > 0;
}

void BooksBook::setCoverImage(QImage aImage)
{
    if (iCoverImage != aImage) {
        iCoverImage = aImage;
        Q_EMIT coverImageChanged();
    }
}

bool BooksBook::requestCoverImage()
{
    if (!iBook.isNull() && !iFormatPlugin.isNull() &&
        !iCoverTasksDone && !iCoverTask) {
        HDEBUG(iTitle);
        iCoverTask = new LoadCoverTask(iStorage, iBook, iFormatPlugin);
        connect(iCoverTask, SIGNAL(done()), SLOT(onLoadCoverTaskDone()));
        iTaskQueue->submit(iCoverTask);
        Q_EMIT loadingCoverChanged();
    }
    return iCoverTask != NULL;
}

void BooksBook::cancelCoverRequest()
{
    if (iCoverTask) {
        iCoverTask->release(this);
        iCoverTask = NULL;
    }
}

bool BooksBook::coverTaskDone()
{
    HASSERT(sender() == iCoverTask);
    HASSERT(!iCoverTasksDone);
    HDEBUG(iTitle << iCoverTask->hasImage());
    const bool gotCover = iCoverTask->hasImage();
    if (gotCover) {
        iCoverImage = iCoverTask->iCoverImage;
        Q_EMIT coverImageChanged();
    }
    iCoverTask->release(this);
    iCoverTask = NULL;
    return gotCover;
}

void BooksBook::onLoadCoverTaskDone()
{
    HDEBUG(iTitle);
    const bool coverMissing = iCoverTask->iCoverMissing;
    if (coverTaskDone() || coverMissing) {
        iCoverTasksDone = true;
        Q_EMIT loadingCoverChanged();
    } else {
        iCoverTask = new GuessCoverTask(iStorage, iBook);
        connect(iCoverTask, SIGNAL(done()), SLOT(onGuessCoverTaskDone()));
        iTaskQueue->submit(iCoverTask);
    }
}

void BooksBook::onGuessCoverTaskDone()
{
    HDEBUG(iTitle);
    coverTaskDone();
    iCoverTasksDone = true;
    Q_EMIT loadingCoverChanged();
}

void BooksBook::saveState()
{
    if (!iStateFilePath.isEmpty()) {
        QVariantMap state;
        HarbourJson::load(iStateFilePath, state);
        state.insert(BOOK_STATE_POSITION, iLastPos.toVariant());
        if (HarbourJson::save(iStateFilePath, state)) {
            HDEBUG("wrote" << iStateFilePath);
        }
    }
}

void BooksBook::deleteFiles()
{
    if (iStorage.isValid()) {
        if (QFile::remove(iPath)) {
            HDEBUG("deleted" << qPrintable(iPath));
        } else {
            HWARN("failed to delete" << qPrintable(iPath));
        }
        QDirIterator it(iStorage.configDir());
        while (it.hasNext()) {
            QString path(it.next());
            if (it.fileName().startsWith(iFileName)) {
                if (QFile::remove(path)) {
                    HDEBUG(qPrintable(path));
                } else {
                    HWARN("failed to delete" << qPrintable(path));
                }
            }
        }
    }
}
