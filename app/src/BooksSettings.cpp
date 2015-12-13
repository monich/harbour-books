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

#include "BooksSettings.h"
#include "BooksTextStyle.h"
#include "BooksBook.h"
#include "BooksDefs.h"
#include "BooksUtil.h"

#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_PATH              BOOKS_DCONF_ROOT
#define KEY_FONT_SIZE           "fontSize"
#define KEY_PAGE_DETAILS        "pageDetails"
#define KEY_CURRENT_BOOK        "currentBook"
#define KEY_CURRENT_FOLDER      "currentFolder"
#define KEY_INVERT_COLORS       "invertColors"
#define DEFAULT_FONT_SIZE       0
#define DEFAULT_PAGE_DETAILS    0
#define DEFAULT_CURRENT_BOOK    QString()
#define DEFAULT_CURRENT_FOLDER  QString()
#define DEFAULT_INVERT_COLORS   false

#define PAGETOOL_COLOR                      QColor(128,128,128) // any bg
#define NORMAL_PAGETOOL_HIGHLIGHT_COLOR     QColor(64,64,64)    // on white
#define INVERTED_PAGETOOL_HIGHLIGHT_COLOR   QColor(192,192,192) // on black

// ==========================================================================
// BooksSettings::TextStyle
// ==========================================================================

class BooksSettings::TextStyle : public ZLTextStyle
{
public:
    TextStyle(int aFontSizeModifier) :
        iDefaultStyle(BooksTextStyle::defaults()),
        iFontSize(iDefaultStyle->fontSize() + 2*aFontSizeModifier)
        { HDEBUG(iFontSize); }

    bool isDecorated() const;

    const std::vector<std::string> &fontFamilies() const;

    int fontSize() const;
    bool bold() const;
    bool italic() const;

    const std::string &colorStyle() const;

    short spaceBefore(const ZLTextStyleEntry::Metrics& aMetrics) const;
    short spaceAfter(const ZLTextStyleEntry::Metrics& aMetrics) const;
    short lineStartIndent(const ZLTextStyleEntry::Metrics& aMetrics, bool aRtl) const;
    short lineEndIndent(const ZLTextStyleEntry::Metrics& aMetrics, bool aRtl) const;
    short firstLineIndentDelta(const ZLTextStyleEntry::Metrics& aMetrics) const;
    int verticalShift() const;

    ZLTextAlignmentType alignment() const;

    double lineSpace() const;
    bool allowHyphenations() const;

private:
    shared_ptr<ZLTextStyle> iDefaultStyle;
    int iFontSize;
};

const std::string&
BooksSettings::TextStyle::colorStyle() const
{
    return iDefaultStyle->colorStyle();
}

bool
BooksSettings::TextStyle::isDecorated() const
{
    return iDefaultStyle->isDecorated();
}

const std::vector<std::string>&
BooksSettings::TextStyle::fontFamilies() const
{
    return iDefaultStyle->fontFamilies();
}

int
BooksSettings::TextStyle::fontSize() const
{
    return iFontSize;
}

bool
BooksSettings::TextStyle::bold() const
{
    return iDefaultStyle->bold();
}

bool
BooksSettings::TextStyle::italic() const
{
    return iDefaultStyle->italic();
}

short
BooksSettings::TextStyle::spaceBefore(
    const ZLTextStyleEntry::Metrics& aMetrics) const
{
    return iDefaultStyle->spaceBefore(aMetrics);
}

short
BooksSettings::TextStyle::spaceAfter(
    const ZLTextStyleEntry::Metrics& aMetrics) const
{
    return iDefaultStyle->spaceAfter(aMetrics);
}

short
BooksSettings::TextStyle::lineStartIndent(
    const ZLTextStyleEntry::Metrics& aMetrics,
    bool aRtl) const
{
    return iDefaultStyle->lineStartIndent(aMetrics, aRtl);
}

short
BooksSettings::TextStyle::lineEndIndent(
    const ZLTextStyleEntry::Metrics& aMetrics,
    bool aRtl) const
{
    return iDefaultStyle->lineEndIndent(aMetrics, aRtl);
}

short
BooksSettings::TextStyle::firstLineIndentDelta(
    const ZLTextStyleEntry::Metrics& aMetrics) const
{
    return iDefaultStyle->firstLineIndentDelta(aMetrics);
}

int
BooksSettings::TextStyle::verticalShift() const
{
    return iDefaultStyle->verticalShift();
}

ZLTextAlignmentType
BooksSettings::TextStyle::alignment() const
{
    return iDefaultStyle->alignment();
}

double
BooksSettings::TextStyle::lineSpace() const
{
    return iDefaultStyle->lineSpace();
}

bool
BooksSettings::TextStyle::allowHyphenations() const
{
    return iDefaultStyle->allowHyphenations();
}

// ==========================================================================
// BooksSettings
// ==========================================================================

BooksSettings::BooksSettings(QObject* aParent) :
    QObject(aParent),
    iFontSizeConf(new MGConfItem(DCONF_PATH KEY_FONT_SIZE, this)),
    iPageDetailsConf(new MGConfItem(DCONF_PATH KEY_PAGE_DETAILS, this)),
    iInvertColorsConf(new MGConfItem(DCONF_PATH KEY_INVERT_COLORS, this)),
    iCurrentFolderConf(new MGConfItem(DCONF_PATH KEY_CURRENT_FOLDER, this)),
    iCurrentBookPathConf(new MGConfItem(DCONF_PATH KEY_CURRENT_BOOK, this)),
    iCurrentBook(NULL),
    iFontSize(currentFontSize())
{
    updateCurrentBook();
    updateCurrentStorage();
    connect(iFontSizeConf, SIGNAL(valueChanged()), SLOT(onFontSizeValueChanged()));
    connect(iPageDetailsConf, SIGNAL(valueChanged()), SIGNAL(pageDetailsChanged()));
    connect(iInvertColorsConf, SIGNAL(valueChanged()), SIGNAL(invertColorsChanged()));
    connect(iCurrentFolderConf, SIGNAL(valueChanged()), SLOT(onCurrentFolderChanged()));
    connect(iCurrentBookPathConf, SIGNAL(valueChanged()), SLOT(onCurrentBookPathChanged()));
}

bool
BooksSettings::increaseFontSize()
{
    if (iFontSize < MaxFontSize) {
        setFontSize(iFontSize+1);
        return true;
    } else {
        return false;
    }
}

bool
BooksSettings::decreaseFontSize()
{
    if (iFontSize > MinFontSize) {
        setFontSize(iFontSize-1);
        return true;
    } else {
        return false;
    }
}

int
BooksSettings::currentFontSize() const
{
    const int fontSize = iFontSizeConf->value(DEFAULT_FONT_SIZE).toInt();
    if (fontSize < MinFontSize) {
        return MinFontSize;
    } else if (fontSize > MaxFontSize) {
        return MaxFontSize;
    } else {
        return fontSize;
    }
}

int
BooksSettings::fontSize(
    int aFontSizeAdjust) const
{
    const int fontSize = iFontSize + aFontSizeAdjust;
    if (fontSize < MinFontSize) {
        return MinFontSize;
    } else if (fontSize > MaxFontSize) {
        return MaxFontSize;
    } else {
        return fontSize;
    }
}

void
BooksSettings::setFontSize(
    int aValue)
{
    HDEBUG(aValue);
    iFontSizeConf->set(aValue);
}

void
BooksSettings::onFontSizeValueChanged()
{
    const int newSize = currentFontSize();
    HDEBUG(newSize);
    if (iFontSize != newSize) {
        iFontSize = newSize;
        for (int i=0; i<=FontSizeSteps; i++) {
            iTextStyle[i].reset();
        }
        Q_EMIT fontSizeChanged();
        Q_EMIT textStyleChanged();
    }
}

shared_ptr<ZLTextStyle>
BooksSettings::textStyle(
    int aFontSizeAdjust) const
{
    const int size = fontSize(aFontSizeAdjust);
    const int i = size - MinFontSize;
    shared_ptr<ZLTextStyle> style = iTextStyle[i];
    if (style.isNull()) {
        style = new TextStyle(size);
        iTextStyle[i] = style;
    }
    return style;
}

int
BooksSettings::pageDetails() const
{
    return iPageDetailsConf->value(DEFAULT_PAGE_DETAILS).toInt();
}

void
BooksSettings::setPageDetails(
    int aValue)
{
    HDEBUG(aValue);
    iPageDetailsConf->set(aValue);
}

bool
BooksSettings::invertColors() const
{
    return iInvertColorsConf->value(DEFAULT_INVERT_COLORS).toBool();
}

void
BooksSettings::setInvertColors(
    bool aValue)
{
    HDEBUG(aValue);
    iInvertColorsConf->set(aValue);
}

QString
BooksSettings::currentFolder() const
{
    return iCurrentFolderConf->value(DEFAULT_CURRENT_FOLDER).toString();
}

void
BooksSettings::setCurrentFolder(
    QString aValue)
{
    HDEBUG(aValue);
    iCurrentFolderConf->set(aValue);
}

void
BooksSettings::onCurrentFolderChanged()
{
    if (updateCurrentStorage()) {
        Q_EMIT currentStorageChanged();
    }
    Q_EMIT currentFolderChanged();
}

bool
BooksSettings::updateCurrentStorage()
{
    BooksStorageManager* mgr = BooksStorageManager::instance();
    BooksStorage storage = mgr->storageForPath(currentFolder());
    if (storage.isValid() && storage.device() != iCurrentStorageDevice) {
        iCurrentStorageDevice = storage.device();
        return true;
    }
    return false;
}

QObject*
BooksSettings::currentBook() const
{
    return iCurrentBook;
}

void
BooksSettings::setCurrentBook(
    QObject* aBook)
{
    BooksBook* book = qobject_cast<BooksBook*>(aBook);
    if (iCurrentBook != book) {
        if (iCurrentBook) iCurrentBook->release();
        if (book) {
            HDEBUG(book->path());
            (iCurrentBook = book)->retain();
            iCurrentBookPathConf->set(book->path());
        } else {
            iCurrentBook = NULL;
            iCurrentBookPathConf->set(QString());
        }
        Q_EMIT currentBookChanged();
    }
}

bool
BooksSettings::updateCurrentBook()
{
    QString path = iCurrentBookPathConf->value(DEFAULT_CURRENT_BOOK).toString();
    if (path.isEmpty()) {
        if (iCurrentBook) {
            iCurrentBook->release();
            iCurrentBook = NULL;
            return true;
        }
    } else if (!iCurrentBook || iCurrentBook->path() != path) {
        shared_ptr<Book> book = BooksUtil::bookFromFile(path);
        if (!book.isNull()) {
            QString rel;
            QFileInfo info(path);
            BooksStorageManager* mgr = BooksStorageManager::instance();
            BooksStorage storage = mgr->storageForPath(info.path(), &rel);
            if (storage.isValid()) {
                if (iCurrentBook) iCurrentBook->release();
                iCurrentBook = new BooksBook(storage, rel, book);
                iCurrentBook->requestCoverImage();
                return true;
            }
        }
        if (iCurrentBook) {
            iCurrentBook->release();
            iCurrentBook = NULL;
            return true;
        }
    }
    return false;
}

void
BooksSettings::onCurrentBookPathChanged()
{
    if (updateCurrentBook()) {
        Q_EMIT currentBookChanged();
    }
}

QColor
BooksSettings::primaryPageToolColor() const
{
    return PAGETOOL_COLOR;
}

QColor
BooksSettings::highlightPageToolColor() const
{
    return invertColors() ?
        INVERTED_PAGETOOL_HIGHLIGHT_COLOR :
        NORMAL_PAGETOOL_HIGHLIGHT_COLOR;
}
