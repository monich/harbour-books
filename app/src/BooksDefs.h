/*
 * Copyright (C) 2015-2021 Jolla Ltd.
 * Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>
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

#ifndef BOOKS_DEFS_H
#define BOOKS_DEFS_H

#include <QString>

#ifdef OPENREPOS
#  define BOOKS_DBUS_INTERFACE  "openrepos.books"
#  define BOOKS_APP_NAME        "openrepos-books"
#  define BOOKS_SETTINGS_MENU   false
#else
#  define BOOKS_APP_NAME        "harbour-books"
#  define BOOKS_SETTINGS_MENU    true
#endif

#define BOOKS_DBUS_SERVICE      BOOKS_DBUS_INTERFACE
#define BOOKS_DCONF_ROOT        "/apps/" BOOKS_APP_NAME "/"
#define BOOKS_DATA_ROOT         "usr/share/" BOOKS_APP_NAME
#define BOOKS_QML_DIR           BOOKS_DATA_ROOT "/qml"
#define BOOKS_ICONS_DIR         BOOKS_DATA_ROOT "/icons"
#define BOOKS_DATA_DIR          BOOKS_DATA_ROOT "/data"
#define BOOKS_QML_FILE          BOOKS_QML_DIR "/BooksMain.qml"

#define BOOKS_INTERNAL_ROOT     "Documents/Books"

#define BOOKS_QML_PLUGIN        "harbour.books"
#define BOOKS_QML_PLUGIN_V1     1
#define BOOKS_QML_PLUGIN_V2     0
#define BOOKS_QML_REGISTER(klass,name) \
    qmlRegisterType<klass>(BOOKS_QML_PLUGIN, BOOKS_QML_PLUGIN_V1, \
    BOOKS_QML_PLUGIN_V2, name)
#define BOOKS_QML_REGISTER_SINGLETON(klass,name) \
    qmlRegisterSingletonType<klass>(BOOKS_QML_PLUGIN, BOOKS_QML_PLUGIN_V1, \
    BOOKS_QML_PLUGIN_V2, name, klass::createSingleton)

#define BOOKS_STATE_FILE_SUFFIX ".state"
#define BOOKS_MARKS_FILE_SUFFIX ".marks"

extern int booksPPI;
#define BOOKS_PPI booksPPI

#endif // BOOKS_DEFS_H
