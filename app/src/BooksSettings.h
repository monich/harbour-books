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

#ifndef BOOKS_SETTINGS_H
#define BOOKS_SETTINGS_H

#include "BooksTypes.h"
#include "ZLTextStyle.h"
#include <QColor>
#include <QtQml>

class MGConfItem;

class BooksSettings : public QObject
{
    Q_OBJECT
    Q_ENUMS(FontSize)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(int pageDetails READ pageDetails WRITE setPageDetails NOTIFY pageDetailsChanged)
    Q_PROPERTY(bool invertColors READ invertColors WRITE setInvertColors NOTIFY invertColorsChanged)
    Q_PROPERTY(QObject* currentBook READ currentBook WRITE setCurrentBook NOTIFY currentBookChanged)
    Q_PROPERTY(QString currentFolder READ currentFolder WRITE setCurrentFolder NOTIFY currentFolderChanged)
    Q_PROPERTY(QString currentStorage READ currentStorage NOTIFY currentStorageChanged)
    Q_PROPERTY(QColor primaryPageToolColor READ primaryPageToolColor CONSTANT)
    Q_PROPERTY(QColor highlightPageToolColor READ highlightPageToolColor NOTIFY invertColorsChanged)
    class TextStyle;

public:
    enum FontSize {
        MinFontSize = -5,
        DefaultFontSize = 0,
        MaxFontSize = 10,
        FontSizeSteps = MaxFontSize - MinFontSize
    };

    explicit BooksSettings(QObject* aParent = NULL);

    Q_INVOKABLE bool increaseFontSize();
    Q_INVOKABLE bool decreaseFontSize();

    int fontSize() const { return iFontSize; }
    void setFontSize(int aValue);

    int pageDetails() const;
    void setPageDetails(int aValue);

    shared_ptr<ZLTextStyle> textStyle(int aFontSizeAdjust) const;

    bool invertColors() const;
    void setInvertColors(bool aValue);

    QObject* currentBook() const;
    void setCurrentBook(QObject* aBook);

    QString currentFolder() const;
    void setCurrentFolder(QString aValue);

    QString currentStorage() const { return iCurrentStorageDevice; }
    QColor primaryPageToolColor() const;
    QColor highlightPageToolColor() const;

Q_SIGNALS:
    void fontSizeChanged();
    void textStyleChanged();
    void pageDetailsChanged();
    void invertColorsChanged();
    void currentBookChanged();
    void currentFolderChanged();
    void currentStorageChanged();

private Q_SLOTS:
    void onFontSizeValueChanged();
    void onCurrentBookPathChanged();
    void onCurrentFolderChanged();

private:
    void updateRenderType();
    bool updateCurrentBook();
    bool updateCurrentStorage();
    int currentFontSize() const;
    int fontSize(int aFontSizeAdjust) const;

private:
    MGConfItem* iFontSizeConf;
    MGConfItem* iPageDetailsConf;
    MGConfItem* iInvertColorsConf;
    MGConfItem* iCurrentFolderConf;
    MGConfItem* iCurrentBookPathConf;
    mutable shared_ptr<ZLTextStyle> iTextStyle[FontSizeSteps+1];
    BooksBook* iCurrentBook;
    QString iCurrentStorageDevice;
    int iFontSize;
};

QML_DECLARE_TYPE(BooksSettings)

#endif // BOOKS_SETTINGS_H
