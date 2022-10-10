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

#include "BooksColorSchemeModel.h"
#include "BooksSettingsBase.h"

#include "HarbourColorEditorModel.h"
#include "HarbourDebug.h"

#include <QtQml>

#define SETTINGS_QML_PLUGIN "openrepos.books.settings"

#define SETTINGS_QML_REGISTER_(klass,name) \
    qmlRegisterType<klass>(SETTINGS_QML_PLUGIN, 1, 0, name)
#define SETTINGS_QML_REGISTER(klass) \
    SETTINGS_QML_REGISTER_(klass,#klass)

class BooksSettingsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SETTINGS_QML_PLUGIN)

public:
    void initializeEngine(QQmlEngine*, const char*) Q_DECL_OVERRIDE;
    void registerTypes(const char*) Q_DECL_OVERRIDE;
};

void
BooksSettingsPlugin::initializeEngine(
    QQmlEngine*,
    const char* aUri)
{
    HDEBUG(aUri);
}

void
BooksSettingsPlugin::registerTypes(
    const char* aUri)
{
    HDEBUG(aUri);
    HASSERT(QLatin1String(aUri) == QLatin1String(SETTINGS_QML_PLUGIN));
    SETTINGS_QML_REGISTER_(BooksSettingsBase, "BooksSettings");
    SETTINGS_QML_REGISTER(BooksColorSchemeModel);
    SETTINGS_QML_REGISTER(HarbourColorEditorModel);
}

#include "BooksSettingsPlugin.moc"
