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

#include "BooksConfig.h"

#include "HarbourDebug.h"

// ==========================================================================
// BooksConfig::Private
// ==========================================================================

class BooksConfig::Private
{
public:
};

// ==========================================================================
// BooksConfig
// ==========================================================================

BooksConfig::BooksConfig() : iPrivate(new Private)
{
}

BooksConfig::~BooksConfig()
{
    delete iPrivate;
}

void
BooksConfig::listOptionNames(
    const std::string& aGroup,
    std::vector<std::string>& names)
{
    HDEBUG(aGroup.c_str());
}

void
BooksConfig::listOptionGroups(
    std::vector<std::string>& aGroups)
{
    HDEBUG("sorry...");
}

void
BooksConfig::removeGroup(
    const std::string& aName)
{
    HDEBUG(aName.c_str());
}

const std::string&
BooksConfig::getDefaultValue(
    const std::string& aGroup,
    const std::string& aName,
    const std::string& aDefault) const
{
    HDEBUG(aGroup.c_str() << aName.c_str() << "(" << aDefault.c_str() << ")");
    return aDefault;
}

const std::string&
BooksConfig::getValue(
    const std::string& aGroup,
    const std::string& aName,
    const std::string& aDefault) const
{
    HDEBUG(aGroup.c_str() << aName.c_str() << "(" << aDefault.c_str() << ")");
    return aDefault;
}

void
BooksConfig::setValue(
    const std::string& aGroup,
    const std::string& aName,
    const std::string& aValue,
    const std::string& aCategory)
{
    HDEBUG(aGroup.c_str() << aName.c_str() << aValue.c_str() << aCategory.c_str());
}

void
BooksConfig::unsetValue(
    const std::string& aGroup,
    const std::string& aName)
{
    HDEBUG(aGroup.c_str() << aName.c_str());
}

bool BooksConfig::isAutoSavingSupported() const
{
    HDEBUG("NO");
    return false;
}

void BooksConfig::startAutoSave(int aSeconds)
{
    HDEBUG(aSeconds);
}

// ==========================================================================
// BooksConfigManager
// ==========================================================================

BooksConfigManager::BooksConfigManager()
{
    HASSERT(!ourInstance);
    HASSERT(!ourIsInitialised);
    ourInstance = this;
}

BooksConfigManager::~BooksConfigManager()
{
    HASSERT(ourInstance == this);
    if (ourInstance == this) {
        ourInstance = NULL;
        ourIsInitialised = false;
    }
}

ZLConfig* BooksConfigManager::createConfig() const
{
    ZLConfig* config = new BooksConfig();
    ourIsInitialised = true;
    return config;
}
