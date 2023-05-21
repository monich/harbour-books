/*
 * Copyright (C) 2015-2023 Slava Monich <slava@monich.com>
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
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING
 * IN ANY WAY OUT OF THE USE OR INABILITY TO USE THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "BooksSettings.h"
#include "BooksPaintContext.h"
#include "BooksTextStyle.h"
#include "BooksTextView.h"
#include "BooksBook.h"
#include "BooksDefs.h"
#include "BooksUtil.h"

#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_PATH                  BOOKS_DCONF_ROOT
#define DCONF_PATH_(x)              BOOKS_DCONF_ROOT x
#define KEY_FONT_SIZE               DCONF_PATH_("fontSize")
#define KEY_PAGE_DETAILS            DCONF_PATH_("pageDetails")
#define KEY_NIGHT_MODE_BRIGHTNESS   DCONF_PATH_("nightModeBrightness")
#define KEY_PAGE_DETAILS_FIXED      DCONF_PATH_("pageDetailsFixed")
#define KEY_TURN_PAGE_BY_TAP        DCONF_PATH_("turnPageByTap")
#define KEY_SAMPLE_BOOK_COPIED      DCONF_PATH_("sampleBookCopied")
#define KEY_CURRENT_BOOK            DCONF_PATH_("currentBook")
#define KEY_CURRENT_FOLDER          DCONF_PATH_("currentFolder")
#define KEY_REMOVABLE_ROOT          DCONF_PATH_("removableRoot")
#define KEY_BOOK_PULL_DOWN_MENU     DCONF_PATH_("bookPullDownMenu")
#define KEY_KEEP_DISPLAY_ON         DCONF_PATH_("keepDisplayOn")
#define KEY_LOW_BATTERY_LEVEL       DCONF_PATH_("lowBatteryLevel")
#define KEY_VOLUME_UP_ACTION        DCONF_PATH_("volumeUpAction")
#define KEY_VOLUME_DOWN_ACTION      DCONF_PATH_("volumeDownAction")
#define KEY_ORIENTATION             DCONF_PATH_("orientation")

#define DEFAULT_FONT_SIZE           0
#define DEFAULT_NIGHT_BRIGHTNESS    1.0
#define DEFAULT_PAGE_DETAILS        0
#define DEFAULT_PAGE_DETAILS_FIXED  false
#define DEFAULT_TURN_PAGE_BY_TAP    false
#define DEFAULT_SAMPLE_BOOK_COPIED  false
#define DEFAULT_CURRENT_BOOK        QString()
#define DEFAULT_CURRENT_FOLDER      QString()
#define DEFAULT_REMOVABLE_ROOT      "Books"
#define DEFAULT_BOOK_PULL_DOWN_MENU true
#define DEFAULT_KEEP_DISPLAY_ON     false
#define DEFAULT_LOW_BATTERY_LEVEL   20 // percent
#define DEFAULT_VOLUME_UP_ACTION    BooksSettings::ActionNextPage
#define DEFAULT_VOLUME_DOWN_ACTION  BooksSettings::ActionPreviousPage
#define DEFAULT_ORIENTATION         BooksSettings::OrientationAny

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

    bool isDecorated() const Q_DECL_OVERRIDE;

    const std::vector<std::string> &fontFamilies() const Q_DECL_OVERRIDE;

    int fontSize() const Q_DECL_OVERRIDE;
    bool bold() const Q_DECL_OVERRIDE;
    bool italic() const Q_DECL_OVERRIDE;

    const std::string &colorStyle() const Q_DECL_OVERRIDE;

    short spaceBefore(const ZLTextStyleEntry::Metrics&) const Q_DECL_OVERRIDE;
    short spaceAfter(const ZLTextStyleEntry::Metrics&) const Q_DECL_OVERRIDE;
    short lineStartIndent(const ZLTextStyleEntry::Metrics&, bool) const Q_DECL_OVERRIDE;
    short lineEndIndent(const ZLTextStyleEntry::Metrics&, bool) const Q_DECL_OVERRIDE;
    short firstLineIndentDelta(const ZLTextStyleEntry::Metrics&) const Q_DECL_OVERRIDE;
    int verticalShift() const Q_DECL_OVERRIDE;

    ZLTextAlignmentType alignment() const Q_DECL_OVERRIDE;

    double lineSpace() const Q_DECL_OVERRIDE;
    bool allowHyphenations() const Q_DECL_OVERRIDE;

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
// BooksSettings::Private
// ==========================================================================

class BooksSettings::Private : public QObject
{
    Q_OBJECT
public:
    // Matches Silica::Theme::ColorScheme
    enum ColorScheme {
        LightOnDark,
        DarkOnLight
    };

    Private(BooksSettings* aParent);
    ~Private();

    BooksSettings* parentObject() const;
    bool updateCurrentBook();
    bool updateCurrentStorage();
    bool updateBrightness();
    int fontSizeValue() const;
    int fontSize(int aFontSizeAdjust) const;
    qreal brightnessValue() const;
    qreal nightModeBrightness() const;
    QString currentFolder() const;
    shared_ptr<ZLTextStyle> textStyle(int aFontSizeAdjust) const;
    void setCurrentBook(QObject*);
    static Action getAction(MGConfItem*, Action);
    static qreal normalizeBrightness(qreal);

public Q_SLOTS:
    void onNightModeChanged();
    void onNightModeBrightnessChanged();
    void onFontSizeValueChanged();
    void onCurrentBookPathChanged();
    void onCurrentFolderChanged();

public:
    static QWeakPointer<BooksSettings> sSharedInstance;
    MGConfItem* iFontSizeConf;
    MGConfItem* iNightModeBrightnessConf;
    MGConfItem* iPageDetailsConf;
    MGConfItem* iPageDetailsFixedConf;
    MGConfItem* iTurnPageByTapConf;
    MGConfItem* iSampleBookCopiedConf;
    MGConfItem* iBookPullDownMenuConf;
    MGConfItem* iKeepDisplayOnConf;
    MGConfItem* iLowBatteryLevelConf;
    MGConfItem* iVolumeUpActionConf;
    MGConfItem* iVolumeDownActionConf;
    MGConfItem* iCurrentFolderConf;
    MGConfItem* iCurrentBookPathConf;
    MGConfItem* iOrientationConf;
    MGConfItem* iRemovableRootConf;
    mutable shared_ptr<ZLTextStyle> iTextStyle[FontSizeSteps+1];
    BooksBook* iCurrentBook;
    QString iCurrentStorageDevice;
    int iFontSize;
    qreal iBrightness;
};

QWeakPointer<BooksSettings> BooksSettings::Private::sSharedInstance;

BooksSettings::Private::Private(BooksSettings* aParent) :
    QObject(aParent),
    iFontSizeConf(new MGConfItem(KEY_FONT_SIZE, this)),
    iNightModeBrightnessConf(new MGConfItem(KEY_NIGHT_MODE_BRIGHTNESS, this)),
    iPageDetailsConf(new MGConfItem(KEY_PAGE_DETAILS, this)),
    iPageDetailsFixedConf(new MGConfItem(KEY_PAGE_DETAILS_FIXED, this)),
    iTurnPageByTapConf(new MGConfItem(KEY_TURN_PAGE_BY_TAP, this)),
    iSampleBookCopiedConf(new MGConfItem(KEY_SAMPLE_BOOK_COPIED, this)),
    iBookPullDownMenuConf(new MGConfItem(KEY_BOOK_PULL_DOWN_MENU, this)),
    iKeepDisplayOnConf(new MGConfItem(KEY_KEEP_DISPLAY_ON, this)),
    iLowBatteryLevelConf(new MGConfItem(KEY_LOW_BATTERY_LEVEL, this)),
    iVolumeUpActionConf(new MGConfItem(KEY_VOLUME_UP_ACTION, this)),
    iVolumeDownActionConf(new MGConfItem(KEY_VOLUME_DOWN_ACTION, this)),
    iCurrentFolderConf(new MGConfItem(KEY_CURRENT_FOLDER, this)),
    iCurrentBookPathConf(new MGConfItem(KEY_CURRENT_BOOK, this)),
    iOrientationConf(new MGConfItem(KEY_ORIENTATION, this)),
    iRemovableRootConf(new MGConfItem(KEY_REMOVABLE_ROOT, this)),
    iCurrentBook(Q_NULLPTR)
{
    iFontSize = fontSizeValue();
    iBrightness = brightnessValue();
    connect(aParent, SIGNAL(nightModeChanged()), SLOT(onNightModeChanged()));
    connect(iFontSizeConf, SIGNAL(valueChanged()), SLOT(onFontSizeValueChanged()));
    connect(iCurrentFolderConf, SIGNAL(valueChanged()), SLOT(onCurrentFolderChanged()));
    connect(iCurrentBookPathConf, SIGNAL(valueChanged()), SLOT(onCurrentBookPathChanged()));
    connect(iNightModeBrightnessConf, SIGNAL(valueChanged()), SLOT(onNightModeBrightnessChanged()));
    connect(iPageDetailsConf, SIGNAL(valueChanged()), aParent, SIGNAL(pageDetailsChanged()));
    connect(iPageDetailsFixedConf, SIGNAL(valueChanged()), aParent, SIGNAL(pageDetailsFixedChanged()));
    connect(iTurnPageByTapConf, SIGNAL(valueChanged()), aParent, SIGNAL(turnPageByTapChanged()));
    connect(iSampleBookCopiedConf, SIGNAL(valueChanged()), aParent, SIGNAL(sampleBookCopiedChanged()));
    connect(iBookPullDownMenuConf, SIGNAL(valueChanged()), aParent, SIGNAL(bookPullDownMenuChanged()));
    connect(iKeepDisplayOnConf, SIGNAL(valueChanged()), aParent, SIGNAL(keepDisplayOnChanged()));
    connect(iLowBatteryLevelConf, SIGNAL(valueChanged()), aParent, SIGNAL(lowBatteryLevelChanged()));
    connect(iVolumeUpActionConf, SIGNAL(valueChanged()), aParent, SIGNAL(volumeUpActionChanged()));
    connect(iVolumeDownActionConf, SIGNAL(valueChanged()), aParent, SIGNAL(volumeDownActionChanged()));
    connect(iOrientationConf, SIGNAL(valueChanged()), aParent, SIGNAL(orientationChanged()));
    connect(iRemovableRootConf, SIGNAL(valueChanged()), aParent, SIGNAL(removableRootChanged()));
}

BooksSettings::Private::~Private()
{
}

inline BooksSettings*
BooksSettings::Private::parentObject() const
{
    return qobject_cast<BooksSettings*>(parent());
}

inline qreal
BooksSettings::Private::normalizeBrightness(
    qreal aBrightness)
{
    return (aBrightness < 0) ? 0 : (aBrightness > 1) ? 1 : aBrightness;
}

qreal
BooksSettings::Private::nightModeBrightness() const
{
    bool ok;
    QVariant var(iNightModeBrightnessConf->value(DEFAULT_NIGHT_BRIGHTNESS));
    const qreal value = var.toReal(&ok);
    return ok ? normalizeBrightness(value) : DEFAULT_NIGHT_BRIGHTNESS;
}

inline qreal
BooksSettings::Private::brightnessValue() const
{
    return parentObject()->nightMode() ? nightModeBrightness() : 1.0;
}

bool
BooksSettings::Private::updateBrightness()
{
    const qreal newBrightness = brightnessValue();
    if (iBrightness != newBrightness) {
        iBrightness = newBrightness;
        HDEBUG(iBrightness);
        return true;
    }
    return false;
}

void
BooksSettings::Private::onNightModeChanged()
{
    BooksSettings* settings = parentObject();
    if (updateBrightness()) {
        Q_EMIT settings->brightnessChanged();
    }
}

void
BooksSettings::Private::onNightModeBrightnessChanged()
{
    BooksSettings* settings = parentObject();
    if (updateBrightness()) {
        Q_EMIT settings->brightnessChanged();
    }
    Q_EMIT settings->nightModeBrightnessChanged();
}

int
BooksSettings::Private::fontSizeValue() const
{
    bool ok;
    const int fontSize = iFontSizeConf->value(DEFAULT_FONT_SIZE).toInt(&ok);
    if (ok) {
        if (fontSize < MinFontSize) {
            return MinFontSize;
        } else if (fontSize > MaxFontSize) {
            return MaxFontSize;
        } else {
            return fontSize;
        }
    } else {
        return iFontSize;
    }
}

int
BooksSettings::Private::fontSize(
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

shared_ptr<ZLTextStyle>
BooksSettings::Private::textStyle(
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

QString
BooksSettings::Private::currentFolder() const
{
    return iCurrentFolderConf->value(DEFAULT_CURRENT_FOLDER).toString();
}

void
BooksSettings::Private::setCurrentBook(
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
            iCurrentBook = Q_NULLPTR;
            iCurrentBookPathConf->set(QString());
        }
        Q_EMIT parentObject()->currentBookChanged();
    }
}

bool
BooksSettings::Private::updateCurrentBook()
{
    QString path = iCurrentBookPathConf->value(DEFAULT_CURRENT_BOOK).toString();
    if (path.isEmpty()) {
        if (iCurrentBook) {
            iCurrentBook->release();
            iCurrentBook = Q_NULLPTR;
            return true;
        }
    } else if (!iCurrentBook || iCurrentBook->path() != path) {
        shared_ptr<Book> book = BooksUtil::bookFromFile(path);
        if (!book.isNull()) {
            QString rel;
            QFileInfo info(path);
            BooksStorageManager* mgr = BooksStorageManager::instance();
            BooksStorage storage = mgr->storageForPath(info.path(), &rel);
            if (iCurrentBook) iCurrentBook->release();
            iCurrentBook = storage.isValid() ?
                new BooksBook(storage, rel, book) :
                new BooksBook(BooksStorage::tmpStorage(),
                    info.dir().absolutePath(), book);
            iCurrentBook->requestCoverImage();
            return true;
        }
        if (iCurrentBook) {
            iCurrentBook->release();
            iCurrentBook = Q_NULLPTR;
            return true;
        }
    }
    return false;
}

bool
BooksSettings::Private::updateCurrentStorage()
{
    BooksStorageManager* mgr = BooksStorageManager::instance();
    BooksStorage storage = mgr->storageForPath(currentFolder());
    if (storage.isValid() && storage.device() != iCurrentStorageDevice) {
        iCurrentStorageDevice = storage.device();
        return true;
    }
    return false;
}

void
BooksSettings::Private::onFontSizeValueChanged()
{
    const int newSize = fontSizeValue();
    HDEBUG(newSize);
    if (iFontSize != newSize) {
        iFontSize = newSize;
        for (int i=0; i<=FontSizeSteps; i++) {
            iTextStyle[i].reset();
        }
        BooksSettings* settings = parentObject();
        Q_EMIT settings->fontSizeChanged();
        Q_EMIT settings->textStyleChanged();
    }
}

void
BooksSettings::Private::onCurrentFolderChanged()
{
    BooksSettings* settings = parentObject();
    if (updateCurrentStorage()) {
        Q_EMIT settings->currentStorageChanged();
    }
    Q_EMIT settings->currentFolderChanged();
    Q_EMIT settings->relativePathChanged();
}

void
BooksSettings::Private::onCurrentBookPathChanged()
{
    if (updateCurrentBook()) {
        Q_EMIT parentObject()->currentBookChanged();
    }
}

BooksSettings::Action
BooksSettings::Private::getAction(
    MGConfItem* aItem,
    Action aDefault)
{
    // Need to cast int to enum right away to force "enumeration value not
    // handled in switch" warning if we miss one of the actions:
    Action value = (Action)aItem->value(aDefault).toInt();
    switch (value) {
    case ActionNone:
    case ActionPreviousPage:
    case ActionNextPage:
        return value;
    }
    return aDefault;
}

// ==========================================================================
// BooksSettings
// ==========================================================================

BooksSettings::BooksSettings(QObject* aParent) :
    BooksSettingsBase(aParent),
    iPrivate(new Private(this))
{
}

QSharedPointer<BooksSettings>
BooksSettings::sharedInstance()
{
    QSharedPointer<BooksSettings> instance = Private::sSharedInstance;
    if (instance.isNull()) {
        // QObject::deleteLater protects against trouble in case if the
        // recipient of the signal drops the last shared reference.
        instance = QSharedPointer<BooksSettings>(new BooksSettings, &QObject::deleteLater);
        Private::sSharedInstance = instance;
        // Finish initialization. These invoke BooksStorageManager::instance()
        // which in turn calls BooksSettings::sharedInstance() to call
        // removableRoot(). Now that Private::sSharedInstance is set, it
        // won't cause infinite recursion although the returned BooksSettings
        // object will be slightly under-initialized, so to speak. But that's
        // ok as long as BooksStorageManager::instance() doesn't need anything
        // from BooksSettings other than removableRoot()
        instance->iPrivate->updateCurrentBook();
        instance->iPrivate->updateCurrentStorage();
    }
    return instance;
}

bool
BooksSettings::increaseFontSize()
{
    if (iPrivate->iFontSize < MaxFontSize) {
        setFontSize(iPrivate->iFontSize+1);
        return true;
    } else {
        return false;
    }
}

bool
BooksSettings::decreaseFontSize()
{
    if (iPrivate->iFontSize > MinFontSize) {
        setFontSize(iPrivate->iFontSize-1);
        return true;
    } else {
        return false;
    }
}

void
BooksSettings::setFontSize(
    int aValue)
{
    HDEBUG(aValue);
    iPrivate->iFontSizeConf->set(aValue);
}

int
BooksSettings::fontSize() const
{
    return iPrivate->iFontSize;
}

void
BooksSettings::setNightModeBrightness(
    qreal aValue)
{
    HDEBUG(aValue);
    iPrivate->iNightModeBrightnessConf->set(Private::normalizeBrightness(aValue));
}

qreal
BooksSettings::brightness() const
{
    return iPrivate->iBrightness;
}

qreal
BooksSettings::nightModeBrightness() const
{
    return iPrivate->nightModeBrightness();
}

QString
BooksSettings::currentStorage() const
{
    return iPrivate->iCurrentStorageDevice;
}

shared_ptr<ZLTextStyle>
BooksSettings::textStyle(
    int aFontSizeAdjust) const
{
    return iPrivate->textStyle(aFontSizeAdjust);
}

int
BooksSettings::pageDetails() const
{
    return iPrivate->iPageDetailsConf->value(DEFAULT_PAGE_DETAILS).toInt();
}

void
BooksSettings::setPageDetails(
    int aValue)
{
    HDEBUG(aValue);
    iPrivate->iPageDetailsConf->set(aValue);
}

bool
BooksSettings::pageDetailsFixed() const
{
    return iPrivate->iPageDetailsFixedConf->value(DEFAULT_PAGE_DETAILS_FIXED).toBool();
}

void
BooksSettings::setPageDetailsFixed(
    bool aValue)
{
    HDEBUG(aValue);
    iPrivate->iPageDetailsFixedConf->set(aValue);
}

bool
BooksSettings::turnPageByTap() const
{
    return iPrivate->iTurnPageByTapConf->value(DEFAULT_TURN_PAGE_BY_TAP).toBool();
}

void
BooksSettings::setTurnPageByTap(
    bool aValue)
{
    HDEBUG(aValue);
    iPrivate->iTurnPageByTapConf->set(aValue);
}

bool
BooksSettings::bookPullDownMenu() const
{
    return iPrivate->iBookPullDownMenuConf->value(DEFAULT_BOOK_PULL_DOWN_MENU).toBool();
}

void
BooksSettings::setBookPullDownMenu(
    bool aValue)
{
    HDEBUG(aValue);
    iPrivate->iBookPullDownMenuConf->set(aValue);
}

bool
BooksSettings::sampleBookCopied() const
{
    return iPrivate->iSampleBookCopiedConf->value(DEFAULT_SAMPLE_BOOK_COPIED).toBool();
}

void
BooksSettings::setSampleBookCopied()
{
    HDEBUG("");
    iPrivate->iSampleBookCopiedConf->set(true);
}

bool
BooksSettings::keepDisplayOn() const
{
    return iPrivate->iKeepDisplayOnConf->value(DEFAULT_KEEP_DISPLAY_ON).toBool();
}

void
BooksSettings::setKeepDisplayOn(
    bool aValue)
{
    HDEBUG(aValue);
    iPrivate->iKeepDisplayOnConf->set(aValue);
}

int
BooksSettings::lowBatteryLevel() const
{
    return iPrivate->iLowBatteryLevelConf->value(DEFAULT_LOW_BATTERY_LEVEL).toInt();
}

void
BooksSettings::setLowBatteryLevel(
    int aValue)
{
    HDEBUG(aValue);
    iPrivate->iLowBatteryLevelConf->set(aValue);
}

BooksSettings::Action
BooksSettings::volumeUpAction() const
{
    return Private::getAction(iPrivate->iVolumeUpActionConf,
        DEFAULT_VOLUME_UP_ACTION);
}

void
BooksSettings::setVolumeUpAction(
    int aValue)
{
    HDEBUG(aValue);
    iPrivate->iVolumeUpActionConf->set(aValue);
}

BooksSettings::Action
BooksSettings::volumeDownAction() const
{
    return Private::getAction(iPrivate->iVolumeDownActionConf,
        DEFAULT_VOLUME_DOWN_ACTION);
}

void
BooksSettings::setVolumeDownAction(
    int aValue)
{
    HDEBUG(aValue);
    iPrivate->iVolumeDownActionConf->set(aValue);
}

QString
BooksSettings::removableRoot() const
{
    return iPrivate->iRemovableRootConf->value(DEFAULT_REMOVABLE_ROOT).toString();
}

QString
BooksSettings::relativePath() const
{
    QString rel;
    BooksStorageManager::instance()->storageForPath(currentFolder(), &rel);
    return rel;
}

QString
BooksSettings::currentFolder() const
{
    return iPrivate->currentFolder();
}

void
BooksSettings::setCurrentFolder(
    const QString aValue)
{
    HDEBUG(aValue);
    iPrivate->iCurrentFolderConf->set(aValue);
}

QObject*
BooksSettings::currentBook() const
{
    return iPrivate->iCurrentBook;
}

void
BooksSettings::setCurrentBook(
    QObject* aBook)
{
    iPrivate->setCurrentBook(aBook);
}

BooksSettings::Orientation
BooksSettings::orientation() const
{
    // Need to cast int to enum right away to force "enumeration value not
    // handled in switch" warning if we miss one of the Orientation:
    Orientation value = (Orientation)
        iPrivate->iOrientationConf->value(DEFAULT_ORIENTATION).toInt();
    switch (value) {
    case OrientationAny:
    case OrientationPortrait:
    case OrientationLandscape:
        return value;
    }
    return DEFAULT_ORIENTATION;
}

void
BooksSettings::setCurrentBookPath(
    QString aPath)
{
    HDEBUG(aPath);
    iPrivate->iCurrentBookPathConf->set(aPath);
}

#include "BooksSettings.moc"
