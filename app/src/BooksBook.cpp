/*
 * Copyright (C) 2015-2018 Jolla Ltd.
 * Copyright (C) 2015-2018 Slava Monich <slava.monich@jolla.com>
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

#include "BooksBook.h"
#include "BooksDefs.h"
#include "BooksTask.h"
#include "BooksTextView.h"
#include "BooksTextStyle.h"
#include "BooksPaintContext.h"
#include "BooksSettings.h"
#include "BooksUtil.h"

#include "HarbourJson.h"
#include "HarbourDebug.h"

#include "ZLImage.h"
#include "image/ZLQtImageManager.h"
#include "bookmodel/BookModel.h"
#include "library/Author.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QGuiApplication>
#include <QScreen>

#include <unistd.h>
#include <errno.h>

#define BOOK_STATE_PAGE_STACK_INDEX "pageStackIndex"
#define BOOK_STATE_FONT_SIZE_ADJUST "fontSizeAdjust"
#define BOOK_STATE_POSITION         "position"
#define BOOK_COVER_SUFFIX           ".cover."

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
    CoverTask(QString aStateDir, shared_ptr<Book> aBook, QString aImagePath) :
        iStateDir(aStateDir), iBook(aBook), iImagePath(aImagePath),
        iCoverMissing(false) {}

    bool hasImage() const;

public:
    QString iStateDir;
    shared_ptr<Book> iBook;
    QString iImagePath;
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
    LoadCoverTask(QString aStateDir, shared_ptr<Book> aBook,
        shared_ptr<FormatPlugin> aFormatPlugin, QString aImagePath) :
        BooksBook::CoverTask(aStateDir, aBook, aImagePath),
        iFormatPlugin(aFormatPlugin) {}

    virtual void performTask();

public:
    shared_ptr<FormatPlugin> iFormatPlugin;
};

void BooksBook::LoadCoverTask::performTask()
{
    if (!isCanceled()) {
        // Try to load cached (or custom) cover
        if (!iStateDir.isEmpty()) {
            QString coverPrefix(QString::fromStdString(
                iBook->file().name(false)) + BOOK_COVER_SUFFIX);
            QDirIterator it(iStateDir);
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
    GuessCoverTask(QString aStateDir, shared_ptr<Book> aBook, QString aImagePath) :
        BooksBook::CoverTask(aStateDir, aBook, aImagePath) {}

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

    if (hasImage()) {
        HDEBUG("using image" << iCoverImage.width() << 'x' <<
            iCoverImage.width() << "as cover for" << iBook->title().c_str());

        // Save the extracted image
        if (!iImagePath.isEmpty()) {
            QFileInfo file(iImagePath);
            QDir dir(file.dir());
            if (!dir.mkpath(dir.absolutePath())) {
                HWARN("failed to create" << qPrintable(dir.absolutePath()));
            }
            if (iCoverImage.save(iImagePath)) {
                HDEBUG("saved cover to" << qPrintable(iImagePath));
            } else {
                HWARN("failed to save" << qPrintable(iImagePath));
            }
        }
    } else if (isCanceled()) {
        HDEBUG("cancelled" << iBook->title().c_str());
    } else {
        HDEBUG("no cover for" << iBook->title().c_str());
        iCoverMissing = true;

        // Create empty file. Guessing the cover image is an expensive task,
        // we don't want to do it every time the application is started.
        if (!iImagePath.isEmpty() &&
            // Check if the book file still exists - the failure could've
            // been caused by the SD-card removal.
            QFile::exists(QString::fromStdString(iBook->file().path()))) {
            QFile(iImagePath).open(QIODevice::WriteOnly);
        }
    }
}

// ==========================================================================
// BooksBook::HashTask
// ==========================================================================

class BooksBook::HashTask : public BooksTask
{
public:
    HashTask(QString aPath, QThread* aThread);

    virtual void performTask();

public:
    QString iPath;
    QByteArray iHash;
};

BooksBook::HashTask::HashTask(QString aPath, QThread* aThread) :
    BooksTask(aThread), iPath(aPath)
{
}

void BooksBook::HashTask::performTask()
{
    iHash = BooksUtil::computeFileHashAndSetAttr(iPath, this);
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

BooksBook::BooksBook(const BooksStorage& aStorage, QString aRelativePath,
    shared_ptr<Book> aBook) :
    QObject(NULL),
    iRef(1),
    iStorage(aStorage),
    iBook(aBook),
    iFormatPlugin(PluginCollection::Instance().plugin(*iBook)),
    iTaskQueue(BooksTaskQueue::defaultQueue()),
    iTitle(QString::fromStdString(iBook->title())),
    iPath(QString::fromStdString(iBook->file().physicalFilePath())),
    iFileName(QFileInfo(iPath).fileName()),
    iHash(BooksUtil::fileHashAttr(iPath))
{
    init();
    AuthorList authors(iBook->authors());
    const int n = authors.size();
    for (int i=0; i<n; i++) {
        if (i > 0) iAuthors += ", ";
        iAuthors += QString::fromStdString(authors[i]->name());
    }
    if (iStorage.isValid()) {
        iStateDir = QDir::cleanPath(iStorage.configDir().path() +
            QDir::separator() + aRelativePath);
        iStateFileBase = QDir::cleanPath(iStateDir +
            QDir::separator() + iFileName);
        iStateFilePath =  storageFile(BOOKS_STATE_FILE_SUFFIX);
        // Load the state
        QVariantMap state;
        if (HarbourJson::load(iStateFilePath, state)) {
            iFontSizeAdjust = state.value(BOOK_STATE_FONT_SIZE_ADJUST).toInt();
#ifdef BOOK_STATE_PAGE_STACK_INDEX
            iPageStackPos = state.value(BOOK_STATE_PAGE_STACK_INDEX).toInt();
#endif
            // Current position can be stored in two formats - a single
            // position (older format) or a list of position (newer one).
            // We have to detect which one we are dealing with
            QVariant position(state.value(BOOK_STATE_POSITION));
            BooksPos bookPos(BooksPos::fromVariant(position));
            if (bookPos.valid()) {
                // Old format (single position)
                iPageStack.append(bookPos);
            } else {
                // New format (list of positions)
                iPageStack = BooksPos::List::fromVariant(position);
            }
        }
    }
    // Validate the state
    if (iPageStack.isEmpty()) {
        iPageStack.append(BooksPos(0,0,0));
    }
    if (iPageStackPos < 0) {
        iPageStackPos = 0;
    } else if (iPageStackPos >= iPageStack.count()) {
        iPageStackPos = iPageStack.count() - 1;
    }
    if (iHash.isEmpty()) {
        HDEBUG("need to calculate hash for" << qPrintable(iPath));
        iHashTask = new HashTask(iPath, thread());
        connect(iHashTask, SIGNAL(done()), SLOT(onHashTaskDone()));
        iTaskQueue->submit(iHashTask);
    }
    // Refcounted BooksBook objects are managed by C++ code
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

void BooksBook::init()
{
    iFontSizeAdjust = 0;
    iPageStackPos = 0;
    iCoverTask = NULL;
    iHashTask = NULL;
    iCoverTasksDone = false;
    iCopyingOut = false;
    iSaveTimer = NULL;
    moveToThread(qApp->thread());
}

BooksBook::~BooksBook()
{
    HDEBUG(qPrintable(iPath));
    HASSERT(!iRef.load());
    if (iCoverTask) iCoverTask->release(this);
    if (iHashTask) iHashTask->release(this);
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

QString BooksBook::path() const
{
    return iPath;
}

bool BooksBook::accessible() const
{
    return !iCopyingOut;
}

bool BooksBook::setFontSizeAdjust(int aFontSizeAdjust)
{
    if (aFontSizeAdjust > BooksSettings::FontSizeSteps) {
        aFontSizeAdjust = BooksSettings::FontSizeSteps;
    } else if (aFontSizeAdjust < -BooksSettings::FontSizeSteps) {
        aFontSizeAdjust = -BooksSettings::FontSizeSteps;
    }
    if (iFontSizeAdjust != aFontSizeAdjust) {
        iFontSizeAdjust = aFontSizeAdjust;
        requestSave();
        Q_EMIT fontSizeAdjustChanged();
        return true;
    } else {
        return false;
    }
}

void BooksBook::setPageStack(BooksPos::List aStack, int aStackPos)
{
    if (aStackPos < 0) {
        aStackPos = 0;
    } else if (aStackPos >= aStack.count()) {
        aStackPos = aStack.count() - 1;
    }
    bool changed = false;
    if (iPageStack != aStack) {
        iPageStack = aStack;
        changed = true;
    }
    if (iPageStackPos != aStackPos) {
        iPageStackPos = aStackPos;
        changed = true;
    }
    if (changed) {
        requestSave();
    }
}

void BooksBook::requestSave()
{
    // We only need save timer if we have the state file
    if (!iSaveTimer && iStorage.isValid()) {
        iSaveTimer = new BooksSaveTimer(this);
        connect(iSaveTimer, SIGNAL(save()), SLOT(saveState()));
    }
    if (iSaveTimer) iSaveTimer->requestSave();
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
        iCoverTask = new LoadCoverTask(iStateDir, iBook, iFormatPlugin,
            cachedImagePath());
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
        iCoverTask = new GuessCoverTask(iStateDir, iBook, cachedImagePath());
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

void BooksBook::onHashTaskDone()
{
    iHash = iHashTask->iHash;
    HDEBUG(QString(iHash.toHex()));
    iHashTask->release(this);
    iHashTask = NULL;
    Q_EMIT hashChanged();
}

QString BooksBook::cachedImagePath() const
{
    if (!iStateDir.isEmpty() && !iBook.isNull()) {
        return QDir::cleanPath(iStateDir + QDir::separator() +
            QString::fromStdString(iBook->file().name(false)) +
            BOOK_COVER_SUFFIX "jpg");
    }
    return QString();
}

void BooksBook::saveState()
{
    if (!iStateFilePath.isEmpty()) {
        QVariantMap state;
        HarbourJson::load(iStateFilePath, state);
        state.insert(BOOK_STATE_POSITION, iPageStack.toVariantList());
        state.insert(BOOK_STATE_FONT_SIZE_ADJUST, iFontSizeAdjust);
#ifdef BOOK_STATE_PAGE_STACK_INDEX
        state.insert(BOOK_STATE_PAGE_STACK_INDEX, iPageStackPos);
#endif
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
        QDirIterator it(iStateDir);
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

BooksBook* BooksBook::newBook(const BooksStorage& aStorage, QString aRelPath,
    QString aFileName)
{
    shared_ptr<Book> ref = BooksUtil::bookFromFile(
        QFileInfo(QDir(aStorage.fullPath(aRelPath)), aFileName).
        absoluteFilePath());
    if (!ref.isNull()) {
        return new BooksBook(aStorage, aRelPath, ref);
    } else {
        return NULL;
    }
}

// NOTE: below methods are invoked on the worker thread
bool BooksBook::makeLink(QString aDestPath)
{
    QByteArray oldp(iPath.toLocal8Bit());
    QByteArray newp(aDestPath.toLocal8Bit());
    if (!oldp.isEmpty()) {
        if (!newp.isEmpty()) {
            int err = link(oldp.data(), newp.data());
            if (!err) {
                HDEBUG("linked" << newp << "->" << oldp);
                return true;
            } else {
                HDEBUG(newp << "->" << oldp << "error:" << strerror(errno));
            }
        } else {
            HDEBUG("failed to convert" << newp << "to locale encoding");
        }
    } else {
        HDEBUG("failed to convert" << oldp << "to locale encoding");
    }
    return false;
}

BooksItem* BooksBook::copyTo(const BooksStorage& aStorage, QString aRelPath,
    CopyOperation* aOperation)
{
    QDir destDir(aStorage.fullPath(aRelPath));
    destDir.mkpath(destDir.path());
    const QString absDestPath(QFileInfo(QDir(aStorage.fullPath(aRelPath)),
        iFileName).absoluteFilePath());
    if (!isCanceled(aOperation) && makeLink(absDestPath)) {
        return newBook(aStorage, aRelPath, iFileName);
    } else if (isCanceled(aOperation)) {
        return NULL;
    } else {
        BooksBook* copy = NULL;
        QFile src(iPath);
        const qint64 total = src.size();
        qint64 copied = 0;
        if (src.open(QIODevice::ReadOnly)) {
            QFile dest(absDestPath);
            if (dest.open(QIODevice::WriteOnly)) {
                QDateTime lastTime;
                const qint64 bufsiz = 0x1000;
                char* buf = new char[bufsiz];
                int progress = 0;
                qint64 len;
                while (!isCanceled(aOperation) &&
                       (len = src.read(buf, bufsiz)) > 0 &&
                       !isCanceled(aOperation) &&
                       dest.write(buf, len) == len) {
                    copied += len;
                    if (aOperation) {
                        int newProg = (int)(copied*PROGRESS_PRECISION/total);
                        if (progress != newProg) {
                            // Don't fire signals too often
                            QDateTime now(QDateTime::currentDateTimeUtc());
                            if (!lastTime.isValid() ||
                                lastTime.msecsTo(now) >= MIN_PROGRESS_DELAY) {
                                lastTime = now;
                                progress = newProg;
                                aOperation->copyProgressChanged(progress);
                            }
                        }
                    }
                }
                delete [] buf;
                dest.close();
                aOperation->copyProgressChanged(PROGRESS_PRECISION);
                if (copied == total) {
                    dest.setPermissions(BOOKS_FILE_PERMISSIONS);
                    HDEBUG(total << "bytes copied from"<< qPrintable(iPath) <<
                        "to" << qPrintable(absDestPath));
                    copy = newBook(aStorage, aRelPath, iFileName);

                    // Copy cover image too
                    if (copy && !iCoverImage.isNull()) {
                        QString cover(copy->cachedImagePath());
                        if (!cover.isEmpty() && iCoverImage.save(cover)) {
                            HDEBUG("copied cover to" << qPrintable(cover));
                        }
                    }
                } else {
                    if (isCanceled(aOperation)) {
                        HDEBUG("copy" << qPrintable(iPath) <<  "to" <<
                            qPrintable(absDestPath) << "cancelled");
                    } else {
                        HWARN(copied << "out of" << total <<
                            "bytes copied from" << qPrintable(iPath) <<
                            "to" << qPrintable(absDestPath));
                    }
                    dest.remove();
                }
            } else {
                HWARN("failed to open" << qPrintable(absDestPath));
            }
            src.close();
        } else {
            HWARN("failed to open" << qPrintable(iPath));
        }
        return copy;
    }
}
