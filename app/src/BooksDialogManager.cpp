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

#include "BooksDialogManager.h"

#include "HarbourDebug.h"

#include "ZLDialog.h"
#include "ZLOptionsDialog.h"
#include "ZLProgressDialog.h"
#include "image/ZLQtImageManager.h"

#include <QGuiApplication>
#include <QClipboard>

void BooksDialogManager::createApplicationWindow(ZLApplication* aApp) const
{
    HDEBUG("THIS IS NOT SUPPOSED TO HAPPEN!");
}

shared_ptr<ZLDialog>
BooksDialogManager::createDialog(
    const ZLResourceKey& aKey) const
{
    HDEBUG(aKey.Name.c_str());
    return NULL;
}

shared_ptr<ZLOptionsDialog>
BooksDialogManager::createOptionsDialog(
    const ZLResourceKey& aKey,
    shared_ptr<ZLRunnable> aApplyAction,
    bool aApplyButton) const
{
    HDEBUG(aKey.Name.c_str());
    return NULL;
}

shared_ptr<ZLOpenFileDialog>
BooksDialogManager::createOpenFileDialog(
    const ZLResourceKey& aKey,
    const std::string& aDirPath,
    const std::string& aFilePath,
    const ZLOpenFileDialog::Filter& aFilter) const
{
    HDEBUG(aKey.Name.c_str());
    return NULL;
}

void
BooksDialogManager::informationBox(
    const std::string& title,
    const std::string& message) const
{
    HDEBUG(QString::fromStdString(title) << message.c_str());
}

void
BooksDialogManager::errorBox(
    const ZLResourceKey& key,
    const std::string& message) const
{
    HDEBUG(QString::fromStdString(key.Name) << message.c_str());
}

int
BooksDialogManager::questionBox(
    const ZLResourceKey& key,
    const std::string& message,
    const ZLResourceKey& button0,
    const ZLResourceKey& button1,
    const ZLResourceKey& button2) const
{
    HDEBUG(QString::fromStdString(key.Name) << message.c_str());
    return -1;
}

shared_ptr<ZLProgressDialog>
BooksDialogManager::createProgressDialog(
    const ZLResourceKey& aKey) const
{
    HDEBUG(aKey.Name.c_str());
    return NULL;
}

bool
BooksDialogManager::isClipboardSupported(
    ClipboardType aType) const
{
    return true;
}

void
BooksDialogManager::setClipboardText(
    const std::string& text,
    ClipboardType type) const
{
    if (!text.empty()) {
        QGuiApplication::clipboard()->setText(QString::fromStdString(text),
            (type == CLIPBOARD_MAIN) ? QClipboard::Clipboard : QClipboard::Selection);
    }
}

void
BooksDialogManager::setClipboardImage(
    const ZLImageData &imageData,
    ClipboardType type) const
{
    QGuiApplication::clipboard()->setImage(
        *static_cast<const ZLQtImageData&>(imageData).image(),
        (type == CLIPBOARD_MAIN) ? QClipboard::Clipboard : QClipboard::Selection);
}
