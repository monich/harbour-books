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

#include "BooksStorage.h"
#include "BooksDefs.h"

#include "HarbourDebug.h"

#include "ZLibrary.h"

#include <QHash>
#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QSocketNotifier>

#include <unistd.h>
#include <libudev.h>

#define INTERNAL_STATE_DIR  "internal"
#define REMOVABLE_STATE_DIR "removable"

// ==========================================================================
// BooksStorage::Data
// ==========================================================================

class BooksStorage::Private: public QObject
{
    Q_OBJECT

public:
    Private(QString aDevice, QDir aBooksDir, bool aInternal);

    bool isRemoved() const;
    bool equal(const Private& aData) const;

    static QString mountPoint(QString aPath);
    static bool isMountPoint(QString aPath);

Q_SIGNALS:
    void removed();

public:
    QAtomicInt iRef;
    QString iDevice;
    QDir iBooksDir;
    QDir iConfigDir;
    bool iInternal;
    bool iPresent;
};

BooksStorage::Private::Private(QString aDevice, QDir aBooksDir, bool aInternal) :
    iRef(1), iDevice(aDevice), iBooksDir(aBooksDir), iInternal(aInternal),
    iPresent(true)
{
    QString cfgDir;
    cfgDir = QString::fromStdString(ZLibrary::ApplicationWritableDirectory());
    if (!cfgDir.endsWith('/')) cfgDir += '/';
    if (aInternal) {
        cfgDir += INTERNAL_STATE_DIR;
        QString subDir(aDevice);
        if (subDir.startsWith("/dev")) subDir.remove(0,4);
        if (!subDir.startsWith('/')) cfgDir += '/';
        cfgDir += subDir;
    } else {
        cfgDir += REMOVABLE_STATE_DIR "/";
        QString label = QDir(Private::mountPoint(aBooksDir.path())).dirName();
        if (label.isEmpty()) label = "sdcard";
        cfgDir += label;
    }
    iConfigDir.setPath(cfgDir);
}

bool BooksStorage::Private::equal(const BooksStorage::Private& aData) const
{
    return iInternal == iInternal &&
           iPresent == iPresent &&
           iDevice == aData.iDevice &&
           iBooksDir == aData.iBooksDir;
}

bool BooksStorage::Private::isMountPoint(QString aPath)
{
    std::string path = aPath.toStdString();
    std::string parent = path + "/..";
    struct stat stPath, stParent;
    return stat(path.c_str(), &stPath) == 0 &&
        stat(parent.c_str(), &stParent) == 0 &&
        stPath.st_dev != stParent.st_dev;
}

QString BooksStorage::Private::mountPoint(QString aPath)
{
    QFileInfo info(aPath);
    QDir dir = info.isDir() ? QDir(aPath) : info.dir();
    dir.makeAbsolute();
    while (!isMountPoint(dir.path()) && !dir.isRoot()) {
        info.setFile(dir.path());
        dir = info.dir();
    }
    return dir.path();
}

// ==========================================================================
// BooksStorage
// ==========================================================================

BooksStorage::BooksStorage() :
    QObject(NULL),
    iPrivate(NULL),
    iPassThrough(false)
{
}

BooksStorage::BooksStorage(const BooksStorage& aStorage) :
    QObject(NULL),
    iPrivate(aStorage.iPrivate),
    iPassThrough(false)
{
    if (iPrivate) iPrivate->iRef.ref();
}

BooksStorage::BooksStorage(QString aDevice, QDir aBooksDir, bool aInternal) :
    QObject(NULL),
    iPrivate(new Private(aDevice, aBooksDir, aInternal)),
    iPassThrough(false)
{
    HDEBUG("config dir" << qPrintable(configDir().path()));
}

BooksStorage::~BooksStorage()
{
    if (iPrivate && !iPrivate->iRef.deref()) delete iPrivate;
}

void BooksStorage::connectNotify(const QMetaMethod& aSignal)
{
    if (iPrivate && !iPassThrough) {
        iPassThrough = true;
        connect(iPrivate, SIGNAL(removed()), SIGNAL(removed()));
    }

    // Prototype of this method has changed in Qt5. Make sure that
    // compilation breaks on Qt with older version of Qt:
    QObject::connectNotify(aSignal);
}

QString BooksStorage::device() const
{
    return iPrivate ? iPrivate->iDevice : QString();
}

QDir BooksStorage::booksDir() const
{
    return iPrivate ? iPrivate->iBooksDir : QDir();
}

QDir BooksStorage::configDir() const
{
    return iPrivate ? iPrivate->iConfigDir : QDir();
}

bool BooksStorage::isInternal() const
{
    return iPrivate && iPrivate->iInternal;
}

bool BooksStorage::isPresent() const
{
    return iPrivate && iPrivate->iPresent;
}

QString BooksStorage::fullPath(QString aRelativePath) const
{
    if (iPrivate) {
        QString path(booksDir().path());
        if (!aRelativePath.isEmpty()) {
            if (!path.endsWith('/')) path += '/';
            path += aRelativePath;
        }
        return QDir::cleanPath(path);
    }
    return QString();
}

bool BooksStorage::equal(const BooksStorage& aStorage) const
{
    if (iPrivate == aStorage.iPrivate) {
        return true;
    } else if (!iPrivate || !aStorage.iPrivate) {
        return false;
    } else {
        return iPrivate->equal(*aStorage.iPrivate);
    }
}

BooksStorage& BooksStorage::operator = (const BooksStorage& aStorage)
{
    if (iPrivate != aStorage.iPrivate) {
        if (iPrivate && !iPrivate->iRef.deref()) delete iPrivate;
        iPrivate = aStorage.iPrivate;
        if (iPrivate) iPrivate->iRef.ref();
    }
    return *this;
}

// ==========================================================================
// BooksStorageManager::Private
// ==========================================================================

#define STORAGE_MOUNTS_FILE     "/proc/mounts"

#define STORAGE_SUBSYSTEM       "block"
#define STORAGE_DISK            "disk"
#define STORAGE_PARTITION       "partition"
#define STORAGE_MOUNT_PREFIX    "/media/"

#define STORAGE_ACTION_ADD      "add"
#define STORAGE_ACTION_REMOVE   "remove"

#define STORAGE_SCAN_INTERVAL   100
#define STORAGE_SCAN_TIMEOUT    5000

class BooksStorageManager::Private {
public:
    static BooksStorageManager* gInstance;

    Private();
    ~Private();

    int findDevice(QString aDevice) const;
    int findPath(QString aPath, QString* aRelPath) const;

public:
    QList<BooksStorage> iStorageList;
    struct udev* iUdev;
    struct udev_monitor* iMonitor;
    int iDescriptor;
    QSocketNotifier* iNotifier;
    QTimer* iScanMountsTimer;
    QDateTime iScanDeadline;
};

BooksStorageManager* BooksStorageManager::Private::gInstance = NULL;

BooksStorageManager::Private::Private() :
    iUdev(udev_new()),
    iMonitor(NULL),
    iDescriptor(-1),
    iNotifier(NULL),
    iScanMountsTimer(NULL)
{
    QString homeDevice;
    QString homeBooks = QDir::homePath();
    QString homeMount(BooksStorage::Private::mountPoint(homeBooks));
    if (!homeBooks.endsWith('/')) homeBooks += '/';
    homeBooks += QLatin1String(BOOKS_INTERNAL_ROOT);
    HDEBUG("home mount" << qPrintable(homeMount));
    HDEBUG("home books path" << qPrintable(homeBooks));

    QFile mounts(STORAGE_MOUNTS_FILE);
    if (mounts.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // For some reason QTextStream can't read /proc/mounts line by line
        QByteArray contents = mounts.readAll();
        QTextStream in(&contents);
        QString mediaPrefix(STORAGE_MOUNT_PREFIX);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList entries = line.split(' ', QString::SkipEmptyParts);
            if (entries.count() > 2) {
                QString mount(entries.at(1));
                if (mount == homeMount) {
                    homeDevice = entries.at(0);
                    HDEBUG("home device" << homeDevice);
                } else if (mount.startsWith(mediaPrefix)) {
                    QString dev = entries.at(0);
                    QString path = mount;
                    if (!path.endsWith('/')) path += '/';
                    path += QLatin1String(BOOKS_REMOVABLE_ROOT);
                    HDEBUG("removable device" << dev << path);
                    iStorageList.append(BooksStorage(dev, path, false));
                }
            }
        }
        mounts.close();
    }

    iStorageList.insert(0, BooksStorage(homeDevice, homeBooks, true));

    if (iUdev) {
        iMonitor = udev_monitor_new_from_netlink(iUdev, "udev");
        if (iMonitor) {
            udev_monitor_filter_add_match_subsystem_devtype(iMonitor,
                STORAGE_SUBSYSTEM, NULL);
            udev_monitor_enable_receiving(iMonitor);
            iDescriptor = udev_monitor_get_fd(iMonitor);
            if (iDescriptor >= 0) {
                iNotifier = new QSocketNotifier(iDescriptor,
                    QSocketNotifier::Read);
            }
        }
    }
}

int BooksStorageManager::Private::findDevice(QString aDevice) const
{
    const int n = iStorageList.count();
    for (int i=0; i<n; i++) {
        BooksStorage::Private* data = iStorageList.at(i).iPrivate;
        if (data->iDevice == aDevice) {
            return i;
        }
    }
    return -1;
}

int BooksStorageManager::Private::findPath(QString aPath, QString* aRelPath) const
{
    if (!aPath.isEmpty()) {
        const int n = iStorageList.count();
        for (int i=0; i<n; i++) {
            BooksStorage::Private* data = iStorageList.at(i).iPrivate;
            if (aPath.startsWith(data->iBooksDir.path())) {
                if (aRelPath) {
                    int i = data->iBooksDir.path().length();
                    while (aPath.length() > i && aPath.at(i) == '/') i++;
                    if (aPath.length() > i) {
                        *aRelPath = aPath.right(aPath.length() - i);
                    } else {
                        *aRelPath = QString();
                    }
                }
                return i;
            }
        }
    }
    return -1;
}

BooksStorageManager::Private::~Private()
{
    if (iUdev) {
        if (iMonitor) {
            if (iDescriptor >= 0) {
                delete iNotifier;
                close(iDescriptor);
            }
            udev_monitor_unref(iMonitor);
        }
        udev_unref(iUdev);
    }
}

// ==========================================================================
// BooksStorageManager
// ==========================================================================

BooksStorageManager* BooksStorageManager::instance()
{
    if (!Private::gInstance) {
        Private::gInstance = new BooksStorageManager;
    }
    return Private::gInstance;
}

void BooksStorageManager::deleteInstance()
{
    delete Private::gInstance;
    HASSERT(!Private::gInstance);
}

BooksStorageManager::BooksStorageManager() :
    iPrivate(new Private)
{
    if (iPrivate->iNotifier) {
        connect(iPrivate->iNotifier, SIGNAL(activated(int)),
            SLOT(onDeviceEvent(int)));
    }
}

BooksStorageManager::~BooksStorageManager()
{
    if (Private::gInstance == this) {
        Private::gInstance = NULL;
    }
    delete iPrivate;
}

int BooksStorageManager::count() const
{
    return iPrivate->iStorageList.count();
}

QList<BooksStorage> BooksStorageManager::storageList() const
{
    return iPrivate->iStorageList;
}

BooksStorage BooksStorageManager::storageForDevice(QString aDevice) const
{
    int index = iPrivate->findDevice(aDevice);
    return (index >= 0) ? iPrivate->iStorageList.at(index) : BooksStorage();
}

BooksStorage BooksStorageManager::storageForPath(QString aPath, QString* aRelPath) const
{
    int index = iPrivate->findPath(aPath, aRelPath);
    return (index >= 0) ? iPrivate->iStorageList.at(index) : BooksStorage();
}

void BooksStorageManager::onDeviceEvent(int)
{
    struct udev_device* dev = udev_monitor_receive_device(iPrivate->iMonitor);
    if (dev) {
        const char* devnode = udev_device_get_devnode(dev);
        const char* action = udev_device_get_action(dev);
        HDEBUG("got device");
        HDEBUG("   node:" << devnode);
        HDEBUG("   subsystem:" << udev_device_get_subsystem(dev));
        HDEBUG("   devtype:" << udev_device_get_devtype(dev));
        HDEBUG("   action:" << action);
        if (devnode && action) {
            if (!(strcmp(action, STORAGE_ACTION_ADD))) {
                // Mount list isn't updated yet when we receive this
                // notification. It takes hundreds of milliseconds until
                // it gets mounted and becomes accessible.
                if (!scanMounts()) {
                    HDEBUG("no new mounts found");
                    if (!iPrivate->iScanMountsTimer) {
                        QTimer* timer = new QTimer(this);
                        timer->setSingleShot(false);
                        timer->setInterval(STORAGE_SCAN_INTERVAL);
                        connect(timer, SIGNAL(timeout()), SLOT(onScanMounts()));
                        iPrivate->iScanMountsTimer = timer;
                    }
                    iPrivate->iScanMountsTimer->start();
                    iPrivate->iScanDeadline = QDateTime::currentDateTime().
                        addMSecs(STORAGE_SCAN_TIMEOUT);
                }
            } else if (!(strcmp(action, STORAGE_ACTION_REMOVE))) {
                int pos = iPrivate->findDevice(devnode);
                if (pos >= 0) {
                    HDEBUG("removable device is gone");
                    BooksStorage storage = iPrivate->iStorageList.takeAt(pos);
                    storage.iPrivate->iPresent = false;
                    Q_EMIT storage.iPrivate->removed();
                    Q_EMIT storageRemoved(storage);
                }
            }
        }
        udev_device_unref(dev);
    } else {
        HWARN("no device!");
    }
}

bool BooksStorageManager::scanMounts()
{
    bool newStorageFound = false;
    QFile mounts(STORAGE_MOUNTS_FILE);
    if (mounts.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // For some reason QTextStream can't read /proc/mounts line by line
        QByteArray contents = mounts.readAll();
        QTextStream in(&contents);
        QString mediaPrefix(STORAGE_MOUNT_PREFIX);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList entries = line.split(' ', QString::SkipEmptyParts);
            if (entries.count() > 2) {
                QString mount(entries.at(1));
                if (mount.startsWith(mediaPrefix)) {
                    QString dev = entries.at(0);
                    int index = iPrivate->findDevice(dev);
                    if (index < 0) {
                        QString path = mount;
                        if (!path.endsWith('/')) path += '/';
                        path += QLatin1String(BOOKS_REMOVABLE_ROOT);
                        HDEBUG("new removable device" << dev << path);
                        BooksStorage storage(dev, path, false);
                        iPrivate->iStorageList.append(storage);
                        Q_EMIT storageAdded(storage);
                        newStorageFound = true;
                    }
                }
            }
        }
        mounts.close();
    }
    return newStorageFound;
}

void BooksStorageManager::onScanMounts()
{
    if (scanMounts()) {
        iPrivate->iScanMountsTimer->stop();
    } else {
        QDateTime now = QDateTime::currentDateTime();
        if (now > iPrivate->iScanDeadline) {
            HDEBUG("timeout waiting for new mount to appear");
            iPrivate->iScanMountsTimer->stop();
        } else {
            HDEBUG("no new mounts found");
        }
    }
}

#include "BooksStorage.moc"
