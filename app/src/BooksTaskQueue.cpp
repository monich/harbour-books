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

class BooksTaskQueue::Private {
public:
    static weak_ptr<BooksTaskQueue> gDefaultQueue;
    static weak_ptr<BooksTaskQueue> gScaleQueue;

    static BooksTaskQueue* newDefaultQueue() { return new BooksTaskQueue(1); }
    static BooksTaskQueue* newScaleQueue() { return new BooksTaskQueue(2); }

    static void waitForDone(shared_ptr<BooksTaskQueue> aQueue, int aMsecs) {
        if (!aQueue.isNull()) {
            aQueue->iPool->waitForDone(aMsecs);
        }
    }

    static void waitForDone(int aMsecs) {
        waitForDone(gDefaultQueue, aMsecs);
        waitForDone(gScaleQueue, aMsecs);
    }

    static shared_ptr<BooksTaskQueue> get(weak_ptr<BooksTaskQueue>* aQueue,
        BooksTaskQueue* (*aNewFunc)())
    {
        shared_ptr<BooksTaskQueue> queue;
        if (aQueue->isNull()) {
            *aQueue = (queue = aNewFunc());
        } else {
            queue = *aQueue;
        }
        return queue;
    }
};

weak_ptr<BooksTaskQueue> BooksTaskQueue::Private::gDefaultQueue;
weak_ptr<BooksTaskQueue> BooksTaskQueue::Private::gScaleQueue;

shared_ptr<BooksTaskQueue> BooksTaskQueue::defaultQueue()
{
    return Private::get(&Private::gDefaultQueue, Private::newDefaultQueue);
}

shared_ptr<BooksTaskQueue> BooksTaskQueue::scaleQueue()
{
    return Private::get(&Private::gScaleQueue, Private::newScaleQueue);
}

void BooksTaskQueue::waitForDone(int aMsecs)
{
    Private::waitForDone(aMsecs);
}

BooksTaskQueue::BooksTaskQueue(int aMaxThreadCount) :
    iPool(new QThreadPool)
{
    HDEBUG("created");
    iPool->setMaxThreadCount(aMaxThreadCount);
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
