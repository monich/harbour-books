/*
 * Copyright (C) 2015-2022 Jolla Ltd.
 * Copyright (C) 2015-2022 Slava Monich <slava.monich@jolla.com>
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

#include "BooksSettingsBase.h"
#include "BooksDefs.h"

#include "HarbourDebug.h"
#include "HarbourUtil.h"

#include <MGConfItem>

#define DCONF_PATH                      BOOKS_DCONF_ROOT
#define DCONF_PATH_(x)                  BOOKS_DCONF_ROOT x
#define KEY_NIGHT_MODE                  DCONF_PATH_("invertColors")
#define KEY_CUSTOM_COLOR_SCHEME         DCONF_PATH_("customColorScheme")
#define KEY_USE_CUSTOM_COLOR_SCHEME     DCONF_PATH_("useCustomColorScheme")
#define KEY_AVAILABLE_COLORS            DCONF_PATH_("availableColors")

#define DEFAULT_NIGHT_MODE              false
#define DEFAULT_USE_CUSTOM_COLOR_SCHEME false

// ==========================================================================
// BooksSettingsBase::Private
// ==========================================================================

class BooksSettingsBase::Private : public QObject
{
    Q_OBJECT
public:
    // Matches Silica::Theme::ColorScheme
    enum ColorScheme {
        LightOnDark,
        DarkOnLight
    };

    static const char* gDefaultColors[];

    Private(BooksSettingsBase* aParent);
    ~Private();

    BooksSettingsBase* parentObject() const;
    static QStringList defaultColors();
    QStringList availableColors() const;
    ColorScheme silicaColorScheme() const;
    bool darkOnLight() const;
    BooksColorScheme nightModeScheme(const BooksColorScheme);
    QString customColorSchemeSpec() const;
    bool useCustomColorScheme() const;
    void setTheme(QObject*);
    void updateColorScheme();
    void updateDarkOnLight();
    void updateDefaultHighlightBackgroundColor();
    bool setCustomColorScheme(const BooksColorScheme);
    void emitPendingSignals();
    bool nightMode() const;

public Q_SLOTS:
    void onUseCustomColorSchemeChanged();
    void onThemeHighlightBackgroundColorChanged();
    void onThemeColorSchemeChanged();
    void onNightModeChanged();
    void onCustomColorSchemeChanged();
    void onAvailableColorsChanged();

public:
    MGConfItem* iNightModeConf;
    MGConfItem* iCustomColorSchemeConf;
    MGConfItem* iUseCustomColorSchemeConf;
    MGConfItem* iAvailableColorsConf;
    const QStringList iDefaultColors;
    QStringList iAvailableColors;
    BooksColorScheme iDefaultColorScheme;
    BooksColorScheme iDefaultNightModeColorScheme;
    BooksColorScheme iCustomColorScheme;
    BooksColorScheme iCustomNightModeColorScheme;
    BooksColorScheme iColorScheme;
    QObject* iTheme;
    bool iHaveSilicaColorScheme;
    bool iDarkOnLight;
    bool iColorSchemeChangePending;
    bool iDarkOnLightChangePending;
};

const char* BooksSettingsBase::Private::gDefaultColors[] = {
    "#000000", "#2160b4", "#3c8bff",
    "#007f7f", "#01823f", "#4fb548",
    "#c82246", "#f13c27", "#fedc00",
    "#f78628", "#7d499b", "#937782",
    "#545454", "#a8a8a8", "#ffffff"
};

BooksSettingsBase::Private::Private(
    BooksSettingsBase* aParent) :
    QObject(aParent),
    iNightModeConf(new MGConfItem(KEY_NIGHT_MODE, this)),
    iCustomColorSchemeConf(new MGConfItem(KEY_CUSTOM_COLOR_SCHEME, this)),
    iUseCustomColorSchemeConf(new MGConfItem(KEY_USE_CUSTOM_COLOR_SCHEME, this)),
    iAvailableColorsConf(new MGConfItem(KEY_AVAILABLE_COLORS, this)),
    iDefaultColors(defaultColors()),
    iTheme(Q_NULLPTR),
    iHaveSilicaColorScheme(false),
    iDarkOnLight(false),
    iColorSchemeChangePending(false),
    iDarkOnLightChangePending(false)
{
    iDefaultNightModeColorScheme = nightModeScheme(iDefaultColorScheme);
    iCustomColorScheme = BooksColorScheme(customColorSchemeSpec());
    iCustomNightModeColorScheme = nightModeScheme(iCustomColorScheme);
    connect(iNightModeConf, SIGNAL(valueChanged()), SLOT(onNightModeChanged()));
    connect(iCustomColorSchemeConf, SIGNAL(valueChanged()), SLOT(onCustomColorSchemeChanged()));
    connect(iUseCustomColorSchemeConf, SIGNAL(valueChanged()), SLOT(onUseCustomColorSchemeChanged()));
    connect(iUseCustomColorSchemeConf, SIGNAL(valueChanged()), SLOT(onUseCustomColorSchemeChanged()));
    connect(iAvailableColorsConf, SIGNAL(valueChanged()), SLOT(onAvailableColorsChanged()));
    iAvailableColors = availableColors();
    HDEBUG("Custom color scheme" << customColorSchemeSpec() << "=>" << iCustomColorScheme.toString());
}

BooksSettingsBase::Private::~Private()
{
    if (iTheme) disconnect(iTheme);
}

BooksSettingsBase*
BooksSettingsBase::Private::parentObject() const
{
    return qobject_cast<BooksSettingsBase*>(parent());
}

QStringList
BooksSettingsBase::Private::defaultColors()
{
    QStringList colors;
    const uint n = sizeof(gDefaultColors)/sizeof(gDefaultColors[0]);
    colors.reserve(n);
    for (uint i = 0; i < n; i++) {
        colors.append(QLatin1String(gDefaultColors[i]));
    }
    return colors;
}

bool
BooksSettingsBase::Private::nightMode() const
{
    return iNightModeConf->value(DEFAULT_NIGHT_MODE).toBool();
}

QString
BooksSettingsBase::Private::customColorSchemeSpec() const
{
    return iCustomColorSchemeConf->value().toString();
}

bool
BooksSettingsBase::Private::useCustomColorScheme() const
{
    return iUseCustomColorSchemeConf->value(DEFAULT_USE_CUSTOM_COLOR_SCHEME).toBool();
}

void
BooksSettingsBase::Private::setTheme(
    QObject* aTheme)
{
    if (iTheme != aTheme) {
        if (iTheme) disconnect(iTheme);
        iTheme = aTheme;
        if (iTheme) {
            connect(iTheme,
                SIGNAL(highlightBackgroundColorChanged()),
                SLOT(onThemeHighlightBackgroundColorChanged()));
            iHaveSilicaColorScheme = connect(iTheme,
                SIGNAL(colorSchemeChanged()),
                SLOT(onThemeColorSchemeChanged()));
        } else {
            iHaveSilicaColorScheme = false;
        }
        onThemeColorSchemeChanged();
        Q_EMIT parentObject()->themeChanged();
    }
}

BooksSettingsBase::Private::ColorScheme
BooksSettingsBase::Private::silicaColorScheme() const
{
    if (iHaveSilicaColorScheme) {
        bool ok = false;
        int value = iTheme->property("colorScheme").toInt(&ok);
        if (ok) {
            return (ColorScheme)value;
        }
    }
    return LightOnDark;
}

bool
BooksSettingsBase::Private::darkOnLight() const
{
    return silicaColorScheme() == DarkOnLight;
}

void
BooksSettingsBase::Private::updateColorScheme()
{
    BooksColorScheme scheme(useCustomColorScheme() ?
        (nightMode() ? iCustomNightModeColorScheme : iCustomColorScheme) :
        (nightMode() ? iDefaultNightModeColorScheme : iDefaultColorScheme));
    if (iColorScheme != scheme) {
        iColorScheme = scheme;
        iColorSchemeChangePending = true;
    }
}

void
BooksSettingsBase::Private::updateDarkOnLight()
{
    const bool currentValue = darkOnLight();
    if (iDarkOnLight != currentValue) {
        iDarkOnLight = currentValue;
        iDarkOnLightChangePending = true;
        HDEBUG("darkOnLight" << (iDarkOnLight ? "yes" : "no"));
    }
}

void
BooksSettingsBase::Private::updateDefaultHighlightBackgroundColor()
{
    if (iTheme) {
        const QColor highlightBackgroundColor(iTheme->
            property("highlightBackgroundColor").value<QColor>());
        HDEBUG(highlightBackgroundColor << silicaColorScheme());
        iDefaultColorScheme = iDefaultColorScheme.
            withSelectionBackground(((silicaColorScheme() == LightOnDark) ?
                highlightBackgroundColor.lighter() : highlightBackgroundColor).rgb());
    } else {
        iDefaultColorScheme = BooksColorScheme();
    }
    iDefaultNightModeColorScheme = nightModeScheme(iDefaultColorScheme);
    updateColorScheme();
}

BooksColorScheme
BooksSettingsBase::Private::nightModeScheme(
    const BooksColorScheme aColors)
{
    return aColors.invertedWithSelectionBackground(QColor(aColors.
        selectionBackground()).darker().rgb());
}

void
BooksSettingsBase::Private::onThemeColorSchemeChanged()
{
    updateDefaultHighlightBackgroundColor();
    updateDarkOnLight();
    emitPendingSignals();
}

void
BooksSettingsBase::Private::onThemeHighlightBackgroundColorChanged()
{
    updateDefaultHighlightBackgroundColor();
    emitPendingSignals();
}

void
BooksSettingsBase::Private::onUseCustomColorSchemeChanged()
{
    updateColorScheme();
    Q_EMIT parentObject()->useCustomColorSchemeChanged();
    emitPendingSignals();
}

void
BooksSettingsBase::Private::onCustomColorSchemeChanged()
{
    const BooksColorScheme scheme(customColorSchemeSpec());
    if (iCustomColorScheme != scheme) {
        iCustomColorScheme = scheme;
        iCustomNightModeColorScheme = nightModeScheme(iCustomColorScheme);
        updateColorScheme();
        emitPendingSignals();
    }
}

void
BooksSettingsBase::Private::emitPendingSignals()
{
    if (iColorSchemeChangePending || iDarkOnLightChangePending) {
        const bool colorSchemeChanged = iColorSchemeChangePending;
        const bool darkOnLightChanged = iDarkOnLightChangePending;
        BooksSettingsBase* settings = parentObject();

        iColorSchemeChangePending = false;
        iDarkOnLightChangePending = false;
        if (colorSchemeChanged) {
            Q_EMIT settings->colorSchemeChanged();
        }
        if (darkOnLightChanged) {
            Q_EMIT settings->darkOnLightChanged();
        }
    }
}

bool
BooksSettingsBase::Private::setCustomColorScheme(
    const BooksColorScheme aColors)
{
    if (iCustomColorScheme != aColors) {
        iCustomColorScheme = aColors;
        iCustomNightModeColorScheme = nightModeScheme(aColors);
        iCustomColorSchemeConf->set(aColors.toString());
        updateColorScheme();
        emitPendingSignals();
        return true;
    }
    return false;
}

void
BooksSettingsBase::Private::onNightModeChanged()
{
    BooksSettingsBase* settings = parentObject();
    updateColorScheme();
    emitPendingSignals();
    Q_EMIT settings->nightModeChanged();
}

QStringList
BooksSettingsBase::Private::availableColors() const
{
    return iAvailableColorsConf->value(iDefaultColors).toStringList();
}

void
BooksSettingsBase::Private::onAvailableColorsChanged()
{
    const QStringList newColors(availableColors());
    if (iAvailableColors != newColors) {
        iAvailableColors = newColors;
        Q_EMIT parentObject()->availableColorsChanged();
    }
}

// ==========================================================================
// BooksSettingsBase
// ==========================================================================

BooksSettingsBase::BooksSettingsBase(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

BooksSettingsBase::~BooksSettingsBase()
{
    delete iPrivate;
}

QObject*
BooksSettingsBase::theme() const
{
    return iPrivate->iTheme;
}

void
BooksSettingsBase::setTheme(
    QObject* aTheme)
{
    iPrivate->setTheme(aTheme);
}

bool
BooksSettingsBase::darkOnLight() const
{
    return iPrivate->iDarkOnLight;
}

bool
BooksSettingsBase::nightMode() const
{
    return iPrivate->nightMode();
}

void
BooksSettingsBase::setNightMode(
    bool aValue)
{
    HDEBUG(aValue);
    iPrivate->iNightModeConf->set(aValue);
}

QColor
BooksSettingsBase::pageBackgroundColor() const
{
    return QColor(iPrivate->iColorScheme.background());
}

QColor
BooksSettingsBase::invertedPageBackgroundColor() const
{
    return QColor(HarbourUtil::invertedRgb(iPrivate->iColorScheme.background()));
}

QColor
BooksSettingsBase::primaryPageToolColor() const
{
    const QRgb rgb = iPrivate->iColorScheme.foreground();
    return QColor(qRed(rgb), qGreen(rgb), qBlue(rgb), 128);
}

QColor
BooksSettingsBase::highlightPageToolColor() const
{
    const QRgb rgb = iPrivate->iColorScheme.foreground();
    return QColor(qRed(rgb), qGreen(rgb), qBlue(rgb), 192);
}

BooksColorScheme
BooksSettingsBase::colorScheme() const
{
    return iPrivate->iColorScheme;
}

BooksColorScheme
BooksSettingsBase::customColorScheme() const
{
    return iPrivate->iCustomColorScheme;
}

BooksColorScheme
BooksSettingsBase::customNightModeColorScheme() const
{
    return iPrivate->iCustomNightModeColorScheme;
}

void
BooksSettingsBase::setCustomColorScheme(
    const BooksColorScheme aColors)
{
    if (iPrivate->setCustomColorScheme(aColors)) {
        Q_EMIT customColorSchemeChanged();
    }
}

bool
BooksSettingsBase::useCustomColorScheme() const
{
    return iPrivate->useCustomColorScheme();
}

void
BooksSettingsBase::setUseCustomColorScheme(
    bool aValue)
{
    HDEBUG(aValue);
    iPrivate->iUseCustomColorSchemeConf->set(aValue);
}

const QStringList
BooksSettingsBase::defaultColors() const
{
    return iPrivate->iDefaultColors;
}

QStringList
BooksSettingsBase::availableColors() const
{
    return iPrivate->iAvailableColors;
}

void
BooksSettingsBase::setAvailableColors(
    const QStringList aColors)
{
    iPrivate->iAvailableColorsConf->set(aColors);
    if (iPrivate->iAvailableColors != aColors) {
        Q_EMIT availableColorsChanged();
    }
}

#include "BooksSettingsBase.moc"
