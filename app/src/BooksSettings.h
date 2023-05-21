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

#ifndef BOOKS_SETTINGS_H
#define BOOKS_SETTINGS_H

#include "shared_ptr.h"
#include "ZLTextStyle.h"

#include "BooksSettingsBase.h"

#include <QSharedPointer>

class BooksSettings : public BooksSettingsBase
{
    Q_OBJECT
    Q_ENUMS(FontSize)
    Q_ENUMS(Orientation)
    Q_ENUMS(Action)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(qreal brightness READ brightness NOTIFY brightnessChanged)
    Q_PROPERTY(qreal nightModeBrightness READ nightModeBrightness WRITE setNightModeBrightness NOTIFY nightModeBrightnessChanged)
    Q_PROPERTY(int pageDetails READ pageDetails WRITE setPageDetails NOTIFY pageDetailsChanged)
    Q_PROPERTY(bool pageDetailsFixed READ pageDetailsFixed WRITE setPageDetailsFixed NOTIFY pageDetailsFixedChanged)
    Q_PROPERTY(bool turnPageByTap READ turnPageByTap WRITE setTurnPageByTap NOTIFY turnPageByTapChanged)
    Q_PROPERTY(bool sampleBookCopied READ sampleBookCopied NOTIFY sampleBookCopiedChanged)
    Q_PROPERTY(bool bookPullDownMenu READ bookPullDownMenu WRITE setBookPullDownMenu NOTIFY bookPullDownMenuChanged)
    Q_PROPERTY(bool keepDisplayOn READ keepDisplayOn WRITE setKeepDisplayOn NOTIFY keepDisplayOnChanged)
    Q_PROPERTY(int lowBatteryLevel READ lowBatteryLevel WRITE setLowBatteryLevel NOTIFY lowBatteryLevelChanged)
    Q_PROPERTY(int volumeUpAction READ volumeUpAction WRITE setVolumeUpAction NOTIFY volumeUpActionChanged)
    Q_PROPERTY(int volumeDownAction READ volumeDownAction WRITE setVolumeDownAction NOTIFY volumeDownActionChanged)
    Q_PROPERTY(QObject* currentBook READ currentBook WRITE setCurrentBook NOTIFY currentBookChanged)
    Q_PROPERTY(QString currentFolder READ currentFolder WRITE setCurrentFolder NOTIFY currentFolderChanged)
    Q_PROPERTY(QString currentStorage READ currentStorage NOTIFY currentStorageChanged)
    Q_PROPERTY(QString relativePath READ relativePath NOTIFY relativePathChanged)
    Q_PROPERTY(QString removableRoot READ removableRoot NOTIFY removableRootChanged)
    Q_PROPERTY(int orientation READ orientation NOTIFY orientationChanged)
    class TextStyle;

public:
    enum FontSize {
        MinFontSize = -5,
        DefaultFontSize = 0,
        MaxFontSize = 15,
        FontSizeSteps = MaxFontSize - MinFontSize
    };

    enum Orientation {
        OrientationAny,
        OrientationPortrait,
        OrientationLandscape
    };

    enum Action {
        ActionNone,
        ActionPreviousPage,
        ActionNextPage
    };

    // Use sharedInstance() to instantiate this class
    explicit BooksSettings(QObject* aParent = Q_NULLPTR);
    static QSharedPointer<BooksSettings> sharedInstance();

    Q_INVOKABLE bool increaseFontSize();
    Q_INVOKABLE bool decreaseFontSize();

    shared_ptr<ZLTextStyle> textStyle(int) const;

    int fontSize() const;
    void setFontSize(int aValue);

    qreal brightness() const;
    qreal nightModeBrightness() const;
    void setNightModeBrightness(qreal);

    int pageDetails() const;
    void setPageDetails(int);

    bool pageDetailsFixed() const;
    void setPageDetailsFixed(bool);

    bool turnPageByTap() const;
    void setTurnPageByTap(bool);

    bool bookPullDownMenu() const;
    void setBookPullDownMenu(bool);

    bool sampleBookCopied() const;
    void setSampleBookCopied();

    bool keepDisplayOn() const;
    void setKeepDisplayOn(bool);

    int lowBatteryLevel() const;
    void setLowBatteryLevel(int);

    Action volumeUpAction() const;
    void setVolumeUpAction(int);

    Action volumeDownAction() const;
    void setVolumeDownAction(int);

    QObject* currentBook() const;
    void setCurrentBook(QObject*);

    QString relativePath() const;
    QString removableRoot() const;
    QString currentFolder() const;
    void setCurrentFolder(const QString);

    QString currentStorage() const;

    Orientation orientation() const;

public Q_SLOTS:
    void setCurrentBookPath(QString);

Q_SIGNALS:
    void fontSizeChanged();
    void nightModeBrightnessChanged();
    void brightnessChanged();
    void textStyleChanged();
    void pageDetailsChanged();
    void pageDetailsFixedChanged();
    void turnPageByTapChanged();
    void sampleBookCopiedChanged();
    void bookPullDownMenuChanged();
    void keepDisplayOnChanged();
    void lowBatteryLevelChanged();
    void volumeUpActionChanged();
    void volumeDownActionChanged();
    void currentBookChanged();
    void currentFolderChanged();
    void currentStorageChanged();
    void relativePathChanged();
    void removableRootChanged();
    void orientationChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // BOOKS_SETTINGS_H
