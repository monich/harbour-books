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

#include "BooksUtil.h"
#include "HarbourDebug.h"

#include "ZLDir.h"
#include "formats/FormatPlugin.h"

#include <queue>

shared_ptr<Book> BooksUtil::bookFromFile(std::string aPath)
{
    shared_ptr<Book> book;
    const ZLFile file(aPath);
    PluginCollection& plugins = PluginCollection::Instance();
    shared_ptr<FormatPlugin> plugin = plugins.plugin(file, false);
    if (!plugin.isNull()) {
        std::string error = plugin->tryOpen(file);
        if (error.empty()) {
            book = Book::loadFromFile(file);
        } else {
            HWARN(error.c_str());
        }
    } else if (file.isArchive()) {
        std::queue<std::string> archiveNames;
        std::vector<std::string> items;
        archiveNames.push(aPath);
        while (!archiveNames.empty() && book.isNull()) {
            shared_ptr<ZLDir> dir = ZLFile(archiveNames.front()).directory();
            archiveNames.pop();
            if (!dir.isNull()) {
                dir->collectFiles(items, true);
                for (std::vector<std::string>::const_iterator it = items.begin();
                     book.isNull() && it != items.end(); ++it) {
                    const std::string itemName = dir->itemPath(*it);
                    ZLFile subFile(itemName);
                    if (subFile.isArchive()) {
                        archiveNames.push(itemName);
                    } else {
                        book = Book::loadFromFile(subFile);
                    }
                }
                items.clear();
            }
        }
    }
    return book;
}

bool BooksUtil::isValidFileName(QString aName)
{
    return !aName.isEmpty() &&
        aName != QStringLiteral(".") &&
        aName != QStringLiteral("..") &&
        !aName.contains('/');
}
