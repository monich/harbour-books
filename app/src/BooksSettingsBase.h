/*
 * Copyright (C) 2022 Jolla Ltd.
 * Copyright (C) 2022 Slava Monich <slava.monich@jolla.com>
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

#ifndef BOOKS_SETTINGS_BASE_H
#define BOOKS_SETTINGS_BASE_H

#include "BooksTypes.h"
#include "BooksColorScheme.h"

#include <QObject>
#include <QColor>
#include <QStringList>

class BooksSettingsBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool darkOnLight READ darkOnLight NOTIFY darkOnLightChanged)
    Q_PROPERTY(bool nightMode READ nightMode WRITE setNightMode NOTIFY nightModeChanged)
    Q_PROPERTY(QColor pageBackgroundColor READ pageBackgroundColor NOTIFY colorSchemeChanged)
    Q_PROPERTY(QColor invertedPageBackgroundColor READ invertedPageBackgroundColor NOTIFY colorSchemeChanged)
    Q_PROPERTY(QColor primaryPageToolColor READ primaryPageToolColor NOTIFY colorSchemeChanged)
    Q_PROPERTY(QColor highlightPageToolColor READ highlightPageToolColor NOTIFY colorSchemeChanged)
    Q_PROPERTY(BooksColorScheme colorScheme READ colorScheme NOTIFY colorSchemeChanged)
    Q_PROPERTY(BooksColorScheme customColorScheme READ customColorScheme WRITE setCustomColorScheme NOTIFY customColorSchemeChanged)
    Q_PROPERTY(BooksColorScheme customNightModeColorScheme READ customNightModeColorScheme NOTIFY customColorSchemeChanged)
    Q_PROPERTY(bool useCustomColorScheme READ useCustomColorScheme WRITE setUseCustomColorScheme NOTIFY useCustomColorSchemeChanged)
    Q_PROPERTY(QStringList defaultColors READ defaultColors CONSTANT)
    Q_PROPERTY(QStringList availableColors READ availableColors WRITE setAvailableColors NOTIFY availableColorsChanged)

public:
    explicit BooksSettingsBase(QObject* aParent = Q_NULLPTR);
    ~BooksSettingsBase();

    QObject* theme() const;
    void setTheme(QObject*);
    bool darkOnLight() const;

    bool nightMode() const;
    void setNightMode(bool);

    QColor pageBackgroundColor() const;
    QColor invertedPageBackgroundColor() const;
    QColor primaryPageToolColor() const;
    QColor highlightPageToolColor() const;
    BooksColorScheme colorScheme() const;

    BooksColorScheme customColorScheme() const;
    BooksColorScheme customNightModeColorScheme() const;
    void setCustomColorScheme(const BooksColorScheme);

    bool useCustomColorScheme() const;
    void setUseCustomColorScheme(bool);

    const QStringList defaultColors() const;
    QStringList availableColors() const;
    void setAvailableColors(const QStringList);

Q_SIGNALS:
    void themeChanged();
    void darkOnLightChanged();
    void nightModeChanged();
    void colorSchemeChanged();
    void customColorSchemeChanged();
    void useCustomColorSchemeChanged();
    void availableColorsChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // BOOKS_SETTINGS_BASE_H
