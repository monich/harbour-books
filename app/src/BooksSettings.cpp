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
#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_PATH              BOOKS_DCONF_ROOT
#define KEY_FONT_SIZE           "fontSize"
#define KEY_PAGE_DETAILS        "pageDetails"
#define KEY_CURRENT_BOOK        "currentBook"
#define KEY_INVERT_COLORS       "invertColors"
#define DEFAULT_FONT_SIZE       0
#define DEFAULT_PAGE_DETAILS    0
#define DEFAULT_CURRENT_BOOK    QString()
#define DEFAULT_INVERT_COLORS   false

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

    const std::string &fontFamily() const;

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

const std::string&
BooksSettings::TextStyle::fontFamily() const
{
    return iDefaultStyle->fontFamily();
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
    iFontSize(new MGConfItem(DCONF_PATH KEY_FONT_SIZE, this)),
    iPageDetails(new MGConfItem(DCONF_PATH KEY_PAGE_DETAILS, this)),
    iInvertColors(new MGConfItem(DCONF_PATH KEY_INVERT_COLORS, this)),
    iCurrentBookPath(new MGConfItem(DCONF_PATH KEY_CURRENT_BOOK, this)),
    iCurrentBook(NULL)
{
    iTextStyle = new TextStyle(fontSize());
    updateCurrentBook();
    connect(iFontSize, SIGNAL(valueChanged()), SLOT(onFontSizeValueChanged()));
    connect(iPageDetails, SIGNAL(valueChanged()), SIGNAL(pageDetailsChanged()));
    connect(iInvertColors, SIGNAL(valueChanged()), SIGNAL(invertColorsChanged()));
    connect(iCurrentBookPath, SIGNAL(valueChanged()), SLOT(onCurrentBookPathChanged()));
}

int
BooksSettings::fontSize() const
{
    return iFontSize->value(DEFAULT_FONT_SIZE).toInt();
}

void
BooksSettings::setFontSize(
    int aValue)
{
    HDEBUG(aValue);
    iFontSize->set(aValue);
}

void
BooksSettings::onFontSizeValueChanged()
{
    const int newSize = fontSize();
    HDEBUG(newSize);
    iTextStyle = new TextStyle(newSize);
    Q_EMIT fontSizeChanged();
    Q_EMIT textStyleChanged();
}

int
BooksSettings::pageDetails() const
{
    return iPageDetails->value(DEFAULT_PAGE_DETAILS).toInt();
}

void
BooksSettings::setPageDetails(
    int aValue)
{
    HDEBUG(aValue);
    iPageDetails->set(aValue);
}

bool
BooksSettings::invertColors() const
{
    return iInvertColors->value(DEFAULT_INVERT_COLORS).toBool();
}

void
BooksSettings::setInvertColors(
    bool aValue)
{
    HDEBUG(aValue);
    iInvertColors->set(aValue);
}

QObject*
BooksSettings::currentBook() const
{
    return iCurrentBook;
}

void
BooksSettings::setCurrentBook(QObject* aBook)
{
    BooksBook* book = qobject_cast<BooksBook*>(aBook);
    if (iCurrentBook != book) {
        if (iCurrentBook) iCurrentBook->release();
        if (book) {
            HDEBUG(book->path());
            (iCurrentBook = book)->retain();
            iCurrentBookPath->set(book->path());
        } else {
            iCurrentBook = NULL;
            iCurrentBookPath->set(QString());
        }
        Q_EMIT currentBookChanged();
    }
}

bool
BooksSettings::updateCurrentBook()
{
    QString path = iCurrentBookPath->value(DEFAULT_CURRENT_BOOK).toString();
    if (path.isEmpty()) {
        if (iCurrentBook) {
            iCurrentBook->release();
            iCurrentBook = NULL;
            return true;
        }
    } else if (!iCurrentBook || iCurrentBook->path() != path) {
        ZLFile file(path.toStdString());
        shared_ptr<Book> book = Book::loadFromFile(file);
        if (!book.isNull()) {
            QFileInfo info(path);
            BooksStorageManager* mgr = BooksStorageManager::instance();
            BooksStorage storage = mgr->storageForPath(info.path());
            if (storage.isValid()) {
                if (iCurrentBook) iCurrentBook->release();
                iCurrentBook = new BooksBook(storage, book);
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
