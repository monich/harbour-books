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

#ifndef BOOKS_DEFS_H
#define BOOKS_DEFS_H

#include <QString>

#define BOOKS_APP_NAME          "harbour-books"
#define BOOKS_DATA_ROOT         "usr/share/" BOOKS_APP_NAME
#define BOOKS_QML_DIR           BOOKS_DATA_ROOT "/qml"
#define BOOKS_ICONS_DIR         BOOKS_DATA_ROOT "/icons"
#define BOOKS_DATA_DIR          BOOKS_DATA_ROOT "/data"
#define BOOKS_QML_FILE          BOOKS_QML_DIR "/BooksMain.qml"

#define BOOKS_ROOT_SHELF_DIR    "Books"

#define BOOKS_QML_PLUGIN        "harbour.books"
#define BOOKS_QML_PLUGIN_V1     1
#define BOOKS_QML_PLUGIN_V2     0
#define BOOKS_QML_REGISTER(klass,name) \
    qmlRegisterType<klass>(BOOKS_QML_PLUGIN, BOOKS_QML_PLUGIN_V1, \
    BOOKS_QML_PLUGIN_V2, name)

#define BOOKS_STATE_FILE_SUFFIX ".state"

#if defined(__i386__)
#  define BOOKS_PPI (330)   // Tablet 1536x2048
#elif defined(__arm__)
#  define BOOKS_PPI (245)   // Jolla1 540x960
#else
#  error Unexpected architechture
#endif

#endif // BOOKS_DEFS_H
