/*
 * Copyright (C) 2015-2017 Jolla Ltd.
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

#ifndef BOOKS_TASK_H
#define BOOKS_TASK_H

#include <QObject>
#include <QRunnable>

class BooksTaskQueue;

class BooksTask : public QObject, public QRunnable
{
    Q_OBJECT
    friend class BooksTaskQueue;

protected:
    BooksTask();

public:
    virtual ~BooksTask();

    bool isStarted() const;
    void release(QObject* aHandler);

protected:
    bool isCanceled() const;

    virtual void run();
    virtual void performTask() = 0;

Q_SIGNALS:
    void runFinished();
    void done();

private Q_SLOTS:
    void onAboutToQuit();
    void onRunFinished();

private:
    bool iAboutToQuit;
    bool iSubmitted;
    bool iStarted;
    bool iReleased;
    bool iDone;
};

inline bool BooksTask::isStarted() const
    { return iStarted; }
inline bool BooksTask::isCanceled() const
    { return iReleased || iAboutToQuit; }

#endif // BOOKS_TASK_H
