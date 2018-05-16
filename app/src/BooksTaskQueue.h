/*
 * Copyright (C) 2015-2018 Jolla Ltd.
 * Copyright (C) 2015-2018 Slava Monich <slava.monich@jolla.com>
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

#ifndef BOOKS_TASK_QUEUE_H
#define BOOKS_TASK_QUEUE_H

#include "shared_ptr.h"

#include <QThreadPool>

class BooksTask;

class BooksTaskQueue
{
    friend class shared_ptr_storage<BooksTaskQueue>;

public:
    static shared_ptr<BooksTaskQueue> defaultQueue();
    static shared_ptr<BooksTaskQueue> scaleQueue();
    static shared_ptr<BooksTaskQueue> hashQueue();
    static void waitForDone(int aMsecs = -1);

    QThreadPool* pool() const;

private:
    BooksTaskQueue(int aMaxThreadCount);
    ~BooksTaskQueue();

private:
    class Private;
    friend class Private;
    QThreadPool* iPool;
};

inline QThreadPool* BooksTaskQueue::pool() const
    { return iPool; }

#endif // BOOKS_TASK_QUEUE_H
