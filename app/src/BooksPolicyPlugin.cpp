/*
 * Copyright (C) 2017 Jolla Ltd.
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

#include "BooksPolicyPlugin.h"

// Workaround for org.nemomobile.policy (or Nemo.Policy) not being
// allowed in harbour apps

BooksPolicyPlugin* BooksPolicyPlugin::gInstance = Q_NULLPTR;

const char BooksPolicyPlugin::RESOURCE_QML_TYPE[] = "Resource";
const char BooksPolicyPlugin::PERMISSIONS_QML_TYPE[] = "Permissions";

BooksPolicyPlugin::BooksPolicyPlugin(
    QQmlEngine* aEngine) :
    BooksPluginLoader(aEngine, "org.nemomobile.policy", 1, 0)
{
}

void
BooksPolicyPlugin::registerTypes(
    const char* aModule,
    int aMajor,
    int aMinor)
{
    reRegisterType(RESOURCE_QML_TYPE, aModule, aMajor, aMinor);
    reRegisterType(PERMISSIONS_QML_TYPE, aModule, aMajor, aMinor);
}

void
BooksPolicyPlugin::registerTypes(
    QQmlEngine* aEngine,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    if (!gInstance) {
        gInstance = new BooksPolicyPlugin(aEngine);
    }
    gInstance->registerTypes(aModule, aMajor, aMinor);
}
