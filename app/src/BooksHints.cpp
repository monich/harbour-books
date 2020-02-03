/*
 * Copyright (C) 2015-2020 Jolla Ltd.
 * Copyright (C) 2015-2020 Slava Monich <slava.monich@jolla.com>
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

#include "BooksHints.h"
#include "BooksDefs.h"
#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_PATH                  BOOKS_DCONF_ROOT "hints/"
#define KEY_STORAGE_LEFT_SWIPE      "storageLeftSwipt"
#define DEFAULT_STORAGE_LEFT_SWIPE  0

BooksHints::BooksHints(QObject* aParent) :
    QObject(aParent),
    iStorageLeftSwipe(new MGConfItem(DCONF_PATH KEY_STORAGE_LEFT_SWIPE, this))
{
    connect(iStorageLeftSwipe, SIGNAL(valueChanged()), SIGNAL(storageLeftSwipeChanged()));
}

int
BooksHints::storageLeftSwipe() const
{
    return iStorageLeftSwipe->value(DEFAULT_STORAGE_LEFT_SWIPE).toInt();
}

void
BooksHints::setStorageLeftSwipe(
    int aValue)
{
    HDEBUG(aValue);
    iStorageLeftSwipe->set(aValue);
}

// Callback for qmlRegisterSingletonType<BooksHints>
QObject* BooksHints::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new BooksHints;
}
