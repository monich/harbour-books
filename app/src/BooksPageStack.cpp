/*
 * Copyright (C) 2016-2018 Jolla Ltd.
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

#include "BooksPageStack.h"

#include "HarbourDebug.h"

#define SignalModelChanged          (1 << 0)
#define SignalCountChanged          (1 << 1)
#define SignalCurrentIndexChanged   (1 << 2)
#define SignalCurrentPageChanged    (1 << 3)

// ==========================================================================
// BooksPageStack::Entry
// ==========================================================================

class BooksPageStack::Entry {
public:
    BooksPos iPos;
    int iPage;

public:
    Entry() : iPos(0, 0, 0), iPage(0) {}
    Entry(const BooksPos& aPos, int aPage) : iPos(aPos), iPage(aPage) {}
    Entry(const Entry& aEntry) : iPos(aEntry.iPos), iPage(aEntry.iPage) {}

    const Entry& operator = (const Entry& Entry)
    {
        iPage = Entry.iPage;
        iPos = Entry.iPos;
        return *this;
    }

    bool operator == (const Entry& aEntry) const
    {
        return iPage == aEntry.iPage && iPos == aEntry.iPos;
    }

    bool operator != (const Entry& aEntry) const
    {
        return iPage != aEntry.iPage || iPos != aEntry.iPos;
    }
};

// ==========================================================================
// BooksPageStack::Private
// ==========================================================================

class BooksPageStack::Private : public QObject {
    Q_OBJECT
public:
    QList<Entry> iEntries;
    int iCurrentIndex;
    int iQueuedSignals;

    enum {
        MAX_DEPTH = 10
    };

private:
    BooksPos::List iPageMarks;

public:
    Private(BooksPageStack* aModel);

    BooksPos::Stack getStack() const;
    bool atEnd() const;
    bool isValidIndex(int aIndex) const;
    void setCurrentIndex(int aIndex);
    int currentPage() const;
    int pageAt(int aIndex) const;
    void setCurrentPage(int aPage);
    void setPageAt(int aIndex, int aPage);
    void setStack(BooksPos::List aStack, int aCurrentPos);
    void setPageMarks(BooksPos::List aPageMarks);
    void push(BooksPos);
    void push(int aPage);
    void pop();
    void clear();
    void emitQueuedSignals();

private Q_SLOTS:
    void onModelChanged();

private:
    BooksPageStack* parentModel() const;
    void push(BooksPos aPos, int aPage);
    BooksPos getPosAt(int aIndex) const;
    int findPage(BooksPos aPos) const;
    int makeIndexValid(int aIndex) const;
    void pageChanged(int aIndex);
    void checkCurrentIndex(int aLastIndex);
    void checkCurrentPage(int aLastCurrentPage);
    void checkCount(int aLastCount);
    int validateCurrentIndex();
    void queueSignals(int aSignals);
};

BooksPageStack::Private::Private(BooksPageStack* aModel) :
    QObject(aModel),
    iCurrentIndex(0),
    iQueuedSignals(0)
{
    iEntries.append(Entry());
    connect(aModel,
        SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
        SLOT(onModelChanged()));
    connect(aModel,
        SIGNAL(rowsInserted(QModelIndex,int,int)),
        SLOT(onModelChanged()));
    connect(aModel,
        SIGNAL(rowsRemoved(QModelIndex,int,int)),
        SLOT(onModelChanged()));
    connect(aModel,
        SIGNAL(modelReset()),
        SLOT(onModelChanged()));
}

inline BooksPageStack* BooksPageStack::Private::parentModel() const
{
    return qobject_cast<BooksPageStack*>(parent());
}

void BooksPageStack::Private::emitQueuedSignals()
{
    static const struct SignalInfo {
        int signal;
        void (BooksPageStack::*fn)();
    } signalInfo [] = {
        { SignalModelChanged, &BooksPageStack::changed },
        { SignalCountChanged, &BooksPageStack::countChanged },
        { SignalCurrentIndexChanged, &BooksPageStack::currentIndexChanged },
        { SignalCurrentPageChanged, &BooksPageStack::currentPageChanged}
    };
    const uint n = sizeof(signalInfo)/sizeof(signalInfo[0]);
    if (iQueuedSignals) {
        BooksPageStack* model = parentModel();
        for (uint i=0; i<n && iQueuedSignals; i++) {
            if (iQueuedSignals & signalInfo[i].signal) {
                iQueuedSignals &= ~signalInfo[i].signal;
                Q_EMIT (model->*(signalInfo[i].fn))();
            }
        }
    }
}

void BooksPageStack::Private::onModelChanged()
{
    queueSignals(SignalModelChanged);
}

inline void BooksPageStack::Private::queueSignals(int aSignals)
{
    iQueuedSignals |= aSignals;
}

int BooksPageStack::Private::makeIndexValid(int aIndex) const
{
    if (aIndex < 0) {
        return 0;
    } else if (aIndex >= iEntries.count()) {
        return iEntries.count() - 1;
    }
    return aIndex;
}

inline bool BooksPageStack::Private::isValidIndex(int aIndex) const
{
    return aIndex >= 0 && aIndex < iEntries.count();
}

inline bool BooksPageStack::Private::atEnd() const
{
    return (iCurrentIndex + 1) >= iEntries.count();
}

inline void BooksPageStack::Private::checkCurrentIndex(int aLastIndex)
{
    if (validateCurrentIndex() != aLastIndex) {
        queueSignals(SignalCurrentIndexChanged);
    }
}

inline void BooksPageStack::Private::checkCurrentPage(int aLastCurrentPage)
{
    if (currentPage() != aLastCurrentPage) {
        queueSignals(SignalCurrentPageChanged);
    }
}

inline void BooksPageStack::Private::checkCount(int aLastCount)
{
    if (iEntries.count() != aLastCount) {
        queueSignals(SignalCountChanged);
    }
}

int BooksPageStack::Private::validateCurrentIndex()
{
    const int validIndex = makeIndexValid(iCurrentIndex);
    if (iCurrentIndex != validIndex) {
        iCurrentIndex = validIndex;
        queueSignals(SignalCurrentIndexChanged);
    }
    return iCurrentIndex;
}

inline int BooksPageStack::Private::currentPage() const
{
    return iEntries.at(iCurrentIndex).iPage;
}

int BooksPageStack::Private::pageAt(int aIndex) const
{
    if (isValidIndex(aIndex)) {
        return iEntries.at(aIndex).iPage;
    }
    return 0;
}

void BooksPageStack::Private::setCurrentIndex(int aIndex)
{
    const int newIndex = makeIndexValid(aIndex);
    if (iCurrentIndex != newIndex) {
        const int oldCurrentPage = currentPage();
        iCurrentIndex = newIndex;
        HDEBUG(iCurrentIndex);
        checkCurrentPage(oldCurrentPage);
        queueSignals(SignalCurrentIndexChanged);
    }
}

void BooksPageStack::Private::setPageAt(int aIndex, int aPage)
{
    Entry entry = iEntries.at(aIndex);
    if (entry.iPage != aPage) {
        entry.iPage = aPage;
        const int np = iPageMarks.count();
        if (np > 0) {
            entry.iPos = (aPage < 0) ? iPageMarks.at(0) :
                (aPage >= np) ? iPageMarks.at(np-1) :
                iPageMarks.at(aPage);
        } else {
            entry.iPos.set(0, 0, 0);
        }
        iEntries[aIndex] = entry;
        pageChanged(aIndex);
    }
}

inline void BooksPageStack::Private::setCurrentPage(int aPage)
{
    setPageAt(iCurrentIndex, aPage);
}

void BooksPageStack::Private::pageChanged(int aIndex)
{
    QVector<int> roles;
    roles.append(PageRole);
    BooksPageStack* model = parentModel();
    QModelIndex modelIndex(model->createIndex(aIndex, 0));
    Q_EMIT model->dataChanged(modelIndex, modelIndex, roles);
}

void BooksPageStack::Private::setStack(BooksPos::List aStack, int aStackPos)
{
    if (aStack.isEmpty()) {
        aStack = BooksPos::List();
        aStack.append(BooksPos(0, 0, 0));
    }

    // First entry (always exists)
    BooksPos pos = aStack.at(0);
    Entry lastEntry(pos, findPage(pos));
    if (iEntries.at(0) != lastEntry) {
        iEntries[0] = lastEntry;
        pageChanged(0);
    } else {
        iEntries[0] = lastEntry;
    }

    // Update other entries
    int entryIndex = 1, stackIndex = 1;
    const int oldEntryCount = iEntries.count();
    while (entryIndex < oldEntryCount && stackIndex < aStack.count()) {
        pos = aStack.at(stackIndex++);
        Entry entry(pos, findPage(pos));
        if (iEntries.at(entryIndex) != entry) {
            iEntries[entryIndex] = entry;
            pageChanged(entryIndex);
        } else {
            iEntries[entryIndex] = entry;
        }
        lastEntry = entry;
        entryIndex++;
    }

    if (entryIndex < oldEntryCount) {
        // We have run out of stack entries, remove remainig rows
        BooksPageStack* model = parentModel();
        model->beginRemoveRows(QModelIndex(), entryIndex, oldEntryCount-1);
        while (iEntries.count() > entryIndex) {
            iEntries.removeLast();
        }
        model->endRemoveRows();
        queueSignals(SignalCountChanged);
    } else {
        // Add new entries if necessary
        while (stackIndex < aStack.count()) {
            pos = aStack.at(stackIndex++);
            Entry entry(pos, findPage(pos));
            if (entry != lastEntry) {
                iEntries.append(entry);
                lastEntry = entry;
            }
        }
        const int n = iEntries.count();
        if (n > oldEntryCount) {
            // We have added some entries, update the model
            BooksPageStack* model = parentModel();
            model->beginInsertRows(QModelIndex(), oldEntryCount, n-1);
            model->endInsertRows();
            queueSignals(SignalCountChanged);
        }
    }

    setCurrentIndex(aStackPos);
    validateCurrentIndex();
}

void BooksPageStack::Private::setPageMarks(BooksPos::List aPageMarks)
{
    if (iPageMarks != aPageMarks) {
        iPageMarks = aPageMarks;
        const int prevCurrentPage = currentPage();
        HDEBUG(iPageMarks);
        const int n = iEntries.count();
        for (int i=0; i<n; i++) {
            Entry entry = iEntries.at(i);
            const int page = findPage(entry.iPos);
            if (entry.iPage != page) {
                entry.iPage = page;
                iEntries[i] = entry;
                pageChanged(i);
            }
        }
        checkCurrentPage(prevCurrentPage);
    }
}

int BooksPageStack::Private::findPage(BooksPos aPos) const
{
    BooksPos::ConstIterator it = qBinaryFind(iPageMarks, aPos);
    if (it == iPageMarks.end()) {
        it = qLowerBound(iPageMarks, aPos);
        if (it == iPageMarks.end()) {
            return iPageMarks.count() - 1;
        } else if (it == iPageMarks.begin()) {
            return 0;
        } else {
            return (it - iPageMarks.begin()) - 1;
        }
    } else {
        return it - iPageMarks.begin();
    }
}

BooksPos BooksPageStack::Private::getPosAt(int aIndex) const
{
    if (iPageMarks.isEmpty()) {
        return BooksPos(0, 0, 0);
    } else if (aIndex < 0) {
        return iPageMarks.first();
    } else if (aIndex >= iPageMarks.count()) {
        return iPageMarks.last();
    } else {
        return iPageMarks.at(aIndex);
    }
}

void BooksPageStack::Private::push(BooksPos aPos, int aPage)
{
    Entry last = iEntries.last();
    const int n = iEntries.count();
    // The caller makes sure that the operation makes sense
    HASSERT(!atEnd() || last.iPos != aPos || last.iPage != aPage);
    if (n >= iCurrentIndex+2) {
        // Push on top of the current position.
        if (n > iCurrentIndex+2) {
            // Drop unnecessary entries
            BooksPageStack* model = parentModel();
            model->beginRemoveRows(QModelIndex(), iCurrentIndex+2, n-1);
            while (iEntries.count() > iCurrentIndex+2) {
                iEntries.removeLast();
            }
            model->endRemoveRows();
            queueSignals(SignalCountChanged);
        }
        // And replace the next one
        setPageAt(iCurrentIndex+1, aPage);
        setCurrentIndex(iCurrentIndex+1);
    } else if (n >= MAX_DEPTH) {
        // We have reached the depth limit, push the entire stack down
        const int oldCurrentPage = currentPage();
        for (int i=1; i<n; i++) {
            iEntries[i-1] = iEntries[i];
            pageChanged(i-1);
        }
        iEntries[n-1] = Entry(aPos, aPage);
        pageChanged(n-1);
        checkCurrentPage(oldCurrentPage);
    } else {
        // Just push the new one
        const int i = iCurrentIndex+1;
        BooksPageStack* model = parentModel();
        model->beginInsertRows(QModelIndex(), i, i);
        iEntries.append(Entry(aPos, aPage));
        model->endInsertRows();
        queueSignals(SignalCountChanged);
        setCurrentIndex(i);
    }
}

void BooksPageStack::Private::push(BooksPos aPos)
{
    if (!atEnd() || iEntries.last().iPos != aPos) {
        push(aPos, findPage(aPos));
    }
}

void BooksPageStack::Private::push(int aPage)
{
    if (!atEnd() || iEntries.last().iPage != aPage) {
        push(getPosAt(aPage), aPage);
    }
}

void BooksPageStack::Private::pop()
{
    const int n = iEntries.count();
    if (n > 1) {
        BooksPageStack* model = parentModel();
        model->beginRemoveRows(QModelIndex(), n-1, n-1);
        iEntries.removeLast();
        validateCurrentIndex();
        model->endRemoveRows();
        queueSignals(SignalCountChanged);
    }
}

void BooksPageStack::Private::clear()
{
    const int n = iEntries.count();
    if (n > 1) {
        BooksPageStack* model = parentModel();
        Entry currentEntry = iEntries.at(iCurrentIndex);
        model->beginRemoveRows(QModelIndex(), 1, n-1);
        while (iEntries.count() > 1) {
            iEntries.removeLast();
        }
        validateCurrentIndex();
        model->endRemoveRows();
        if (iEntries.at(0) != currentEntry) {
            iEntries[0] = currentEntry;
            pageChanged(0);
        }
        queueSignals(SignalCountChanged);
    }
}

BooksPos::Stack BooksPageStack::Private::getStack() const
{
    const int n = iEntries.count();
    BooksPos::Stack stack;
    stack.iList.reserve(n);
    for (int i=0; i<n; i++) {
        stack.iList.append(iEntries.at(i).iPos);
    }
    stack.iPos = iCurrentIndex;
    return stack;
}

// ==========================================================================
// BooksPageStack
// ==========================================================================

BooksPageStack::BooksPageStack(QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private(this))
{
#if QT_VERSION < 0x050000
    setRoleNames(roleNames());
#endif
}

int BooksPageStack::count() const
{
    return iPrivate->iEntries.count();
}

int BooksPageStack::currentIndex() const
{
    return iPrivate->iCurrentIndex;
}

int BooksPageStack::currentPage() const
{
    return iPrivate->currentPage();
}

void BooksPageStack::setCurrentIndex(int aIndex)
{
    iPrivate->setCurrentIndex(aIndex);
    iPrivate->emitQueuedSignals();
}

void BooksPageStack::setCurrentPage(int aPage)
{
    iPrivate->setCurrentPage(aPage);
    iPrivate->emitQueuedSignals();
}

void BooksPageStack::back()
{
    iPrivate->setCurrentIndex(iPrivate->iCurrentIndex - 1);
    iPrivate->emitQueuedSignals();
}

void BooksPageStack::forward()
{
    iPrivate->setCurrentIndex(iPrivate->iCurrentIndex + 1);
    iPrivate->emitQueuedSignals();
}

BooksPos::Stack BooksPageStack::getStack() const
{
    return iPrivate->getStack();
}

void BooksPageStack::setStack(BooksPos::List aStack, int aCurrentPos)
{
    iPrivate->setStack(aStack, aCurrentPos);
    iPrivate->emitQueuedSignals();
}

void BooksPageStack::setPageMarks(BooksPos::List aPageMarks)
{
    iPrivate->setPageMarks(aPageMarks);
    iPrivate->emitQueuedSignals();
}

int BooksPageStack::pageAt(int aIndex)
{
    return iPrivate->pageAt(aIndex);
}

void BooksPageStack::pushPage(int aPage)
{
    HDEBUG(aPage);
    iPrivate->push(aPage);
    iPrivate->emitQueuedSignals();
}

void BooksPageStack::pushPosition(BooksPos aPos)
{
    HDEBUG("" << aPos);
    iPrivate->push(aPos);
    iPrivate->emitQueuedSignals();
}

void BooksPageStack::pop()
{
    HDEBUG("");
    iPrivate->pop();
    iPrivate->emitQueuedSignals();
}

void BooksPageStack::clear()
{
    HDEBUG("");
    iPrivate->clear();
    iPrivate->emitQueuedSignals();
}

QHash<int,QByteArray> BooksPageStack::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(PageRole, "page");
    return roles;
}

int BooksPageStack::rowCount(const QModelIndex&) const
{
    return iPrivate->iEntries.count();
}

QVariant BooksPageStack::data(const QModelIndex& aIndex, int aRole) const
{
    const int row = aIndex.row();
    if (iPrivate->isValidIndex(row) && aRole == PageRole) {
        return iPrivate->iEntries.at(row).iPage;
    }
    return QVariant();
}

bool BooksPageStack::setData(const QModelIndex& aIndex, const QVariant& aValue,
    int aRole)
{
    const int row = aIndex.row();
    if (iPrivate->isValidIndex(row) && aRole == PageRole) {
        bool ok = false;
        const int page = aValue.toInt(&ok);
        if (page >= 0 && ok) {
            iPrivate->setPageAt(row, page);
            iPrivate->emitQueuedSignals();
            return true;
        }
    }
    return false;
}

#include "BooksPageStack.moc"
