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

#include "BooksTaskQueue.h"
#include "BooksTask.h"

#include "HarbourDebug.h"

static weak_ptr<BooksTaskQueue> booksTaskQueueInstance;

shared_ptr<BooksTaskQueue> BooksTaskQueue::instance()
{
    shared_ptr<BooksTaskQueue> worker;
    if (booksTaskQueueInstance.isNull()) {
        booksTaskQueueInstance = (worker = new BooksTaskQueue());
    } else {
        worker = booksTaskQueueInstance;
    }
    return worker;
}

BooksTaskQueue::BooksTaskQueue() :
    iPool(new QThreadPool)
{
    HDEBUG("created");
    iPool->setMaxThreadCount(1);
}

void BooksTaskQueue::waitForDone(int aMsecs)
{
    shared_ptr<BooksTaskQueue> worker = booksTaskQueueInstance;
    if (!worker.isNull()) {
        worker->iPool->waitForDone(aMsecs);
    }
}

BooksTaskQueue::~BooksTaskQueue()
{
    HDEBUG("deleting");
    iPool->waitForDone();
    delete iPool;
    HDEBUG("deleted");
}

void BooksTaskQueue::submit(BooksTask* aTask)
{
    HASSERT(!aTask->iStarted);
    aTask->iStarted = true;
    iPool->start(aTask);
}

void BooksTaskQueue::submit(BooksTask* aTask, QObject* aTarget,
    const char* aSlot)
{
    QObject::connect(aTask, SIGNAL(done()), aTarget, aSlot);
    submit(aTask);
}
