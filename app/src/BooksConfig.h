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

#ifndef BOOKS_CONFIG_H
#define BOOKS_CONFIG_H

#include "BooksTypes.h"
#include <ZLConfig.h>

class BooksConfigManager : public ZLConfigManager {
public:
    BooksConfigManager();
    ~BooksConfigManager();
    ZLConfig* createConfig() const;
};

class BooksConfig : public ZLConfig
{
public:
    BooksConfig();
    ~BooksConfig();

    // ZLConfig
    void listOptionNames(const std::string& aGroup, std::vector<std::string>& names);
    void listOptionGroups(std::vector<std::string>& aGroups);
    void removeGroup(const std::string& aName);

    const std::string& getDefaultValue(const std::string& aGroup, const std::string& aName, const std::string& aDefault) const;
    const std::string& getValue(const std::string& aGroup, const std::string& aName, const std::string& aDefault) const;
    void setValue(const std::string& aGroup, const std::string& aName, const std::string& aValue, const std::string& aCategory);
    void unsetValue(const std::string& aGroup, const std::string& aName);

    bool isAutoSavingSupported() const;
    void startAutoSave(int aSeconds);

private:
    void updateRenderType();

private:
    class Private;
    Private* iPrivate;
};

#endif // BOOKS_CONFIG_H
