/*
 * Copyright (C) 2015-2017 Jolla Ltd.
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

#ifndef BOOKS_DIALOG_MANAGER_H
#define BOOKS_DIALOG_MANAGER_H

#include <ZLDialogManager.h>

#include <QObject>
#include <QString>
#include <QImage>

class BooksDialogManager: public QObject, ZLDialogManager {
    Q_OBJECT

public:
    static void createInstance();

private:
    BooksDialogManager(QObject* aParent);
    ~BooksDialogManager();

Q_SIGNALS:
    void copyTextToClipboard(QString aText) const;
    void copyImageToClipboard(QImage aImage) const;

private Q_SLOTS:
    void onCopyTextToClipboard(QString aText);
    void onCopyImageToClipboard(QImage aImage);

public:
    void createApplicationWindow(ZLApplication *application) const;

    shared_ptr<ZLDialog> createDialog(const ZLResourceKey &key) const;
    shared_ptr<ZLOptionsDialog> createOptionsDialog(const ZLResourceKey &key, shared_ptr<ZLRunnable> applyAction, bool showApplyButton) const;
    shared_ptr<ZLOpenFileDialog> createOpenFileDialog(const ZLResourceKey &key, const std::string &directoryPath, const std::string &filePath, const ZLOpenFileDialog::Filter &filter) const;
    void informationBox(const std::string &title, const std::string &message) const;
    void errorBox(const ZLResourceKey &key, const std::string &message) const;
    int questionBox(const ZLResourceKey &key, const std::string &message, const ZLResourceKey &button0, const ZLResourceKey &button1, const ZLResourceKey &button2) const;
    shared_ptr<ZLProgressDialog> createProgressDialog(const ZLResourceKey &key) const;

    bool isClipboardSupported(ClipboardType type) const;
    void setClipboardText(const std::string &text, ClipboardType type) const;
    void setClipboardImage(const ZLImageData &imageData, ClipboardType type) const;
};

#endif /* BOOKS_DIALOG_MANAGER_H */
