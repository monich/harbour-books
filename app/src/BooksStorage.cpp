/*
 * Copyright (C) 2015-2021 Jolla Ltd.
 * Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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
#include "BooksSettings.h"
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include <libudev.h>

#define TMP_STATE_DIR       "tmp"
#define INTERNAL_STATE_DIR  "internal"
#define REMOVABLE_STATE_DIR "removable"

// ==========================================================================
// BooksStorage::Data
// ==========================================================================

class BooksStorage::Private: public QObject
{
    Q_OBJECT

public:
    Private(QString aDevice, QString aMountPoint, QString aBooksDir,
        Type aType);

    bool isRemoved() const;
    bool equal(const Private& aData) const;

    static QString fullPath(QDir aDir, QString aRelativePath);
    static QString mountPoint(QString aPath);
    static bool isMountPoint(QString aPath);

Q_SIGNALS:
    void removed();

public:
    QAtomicInt iRef;
    QString iDevice;
    QString iMountPoint;
    QDir iBooksDir;
    QDir iConfigDir;
    Type iType;
    bool iPresent;
};

BooksStorage::Private::Private(
    QString aDevice,
    QString aMountPoint,
    QString aBooksDir,
    Type aType) :
    iRef(1),
    iDevice(aDevice),
    iMountPoint(aMountPoint),
    iBooksDir(aBooksDir),
    iType(aType),
    iPresent(true)
{
    QString cfgDir;
    cfgDir = QString::fromStdString(ZLibrary::ApplicationWritableDirectory());
    if (!cfgDir.endsWith('/')) cfgDir += '/';
    QString subDir;
    switch (aType) {
    case InternalStorage:
        cfgDir += INTERNAL_STATE_DIR;
        subDir = aDevice;
        if (subDir.startsWith("/dev")) subDir.remove(0,4);
        if (!subDir.startsWith('/')) cfgDir += '/';
        cfgDir += subDir;
        break;
    case RemovableStorage:
        cfgDir += REMOVABLE_STATE_DIR "/";
        subDir = QDir(Private::mountPoint(aBooksDir)).dirName();
        if (subDir.isEmpty()) subDir = "sdcard";
        cfgDir += subDir;
        break;
    case TmpStorage:
        cfgDir += TMP_STATE_DIR "/";
        break;
    }
    iConfigDir.setPath(cfgDir);
}

bool BooksStorage::Private::equal(const BooksStorage::Private& aData) const
{
    return iType == aData.iType &&
           iPresent == aData.iPresent &&
           iMountPoint == aData.iMountPoint &&
           iDevice == aData.iDevice &&
           iBooksDir == aData.iBooksDir &&
           iConfigDir == aData.iConfigDir;
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

QString BooksStorage::Private::fullPath(QDir aDir, QString aRelativePath)
{
    QString path(aDir.path());
    if (!aRelativePath.isEmpty()) {
        if (!path.endsWith('/')) path += '/';
        path += aRelativePath;
    }
    return QDir::cleanPath(path);
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

BooksStorage::BooksStorage(QString aDevice, QString aMount, QString aBooksDir,
    Type aType) : QObject(NULL),
    iPrivate(new Private(aDevice, aMount, aBooksDir, aType)),
    iPassThrough(false)
{
    HDEBUG("config dir" << qPrintable(configDir().path()));
}

BooksStorage::~BooksStorage()
{
    if (iPrivate && !iPrivate->iRef.deref()) delete iPrivate;
}

BooksStorage BooksStorage::tmpStorage()
{
    return BooksStorage("tmpfs", "/tmp", "/", BooksStorage::TmpStorage);
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
    return iPrivate && iPrivate->iType == InternalStorage;
}

bool BooksStorage::isPresent() const
{
    return iPrivate && iPrivate->iPresent;
}

QString BooksStorage::fullPath(QString aRelativePath) const
{
    if (iPrivate) {
        return Private::fullPath(iPrivate->iBooksDir, aRelativePath);
    }
    return QString();
}

QString BooksStorage::fullConfigPath(QString aRelativePath) const
{
    if (iPrivate) {
        return Private::fullPath(iPrivate->iConfigDir, aRelativePath);
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

void BooksStorage::set(const BooksStorage& aStorage)
{
    if (iPrivate != aStorage.iPrivate) {
        if (iPrivate && !iPrivate->iRef.deref()) delete iPrivate;
        iPrivate = aStorage.iPrivate;
        if (iPrivate) iPrivate->iRef.ref();
    }
}

// ==========================================================================
// BooksStorageManager::Private
// ==========================================================================

#define STORAGE_MOUNTS_FILE     "/proc/mounts"

#define STORAGE_SUBSYSTEM       "block"
#define STORAGE_DISK            "disk"
#define STORAGE_PARTITION       "partition"

#define STORAGE_ACTION_ADD      "add"
#define STORAGE_ACTION_REMOVE   "remove"

#define STORAGE_SCAN_INTERVAL   100
#define STORAGE_SCAN_TIMEOUT    5000

class BooksStorageManager::Private : public QObject {
    Q_OBJECT
public:
    static BooksStorageManager* gInstance;

    struct MountEntry {
        QString dev;
        QString path;

        bool parse(QString aLine);
    };

    Private(BooksStorageManager* aParent);
    ~Private();

    BooksStorage internalStorage() const;
    int findDevice(QString aDevice) const;
    int findPath(QString aPath, QString* aRelPath) const;
    bool isMediaMount(const MountEntry* aEntry);
    bool scanMounts();

public Q_SLOTS:
    void onDeviceEvent(int);
    void onRemovableRootChanged();
    void onScanMounts();

public:
    BooksStorageManager* iParent;
    QSharedPointer<BooksSettings> iSettings;
    QList<BooksStorage> iStorageList;
    struct udev* iUdev;
    struct udev_monitor* iMonitor;
    int iDescriptor;
    QSocketNotifier* iNotifier;
    QTimer* iScanMountsTimer;
    QDateTime iScanDeadline;
    QString iRunMediaPrefix;
};

BooksStorageManager* BooksStorageManager::Private::gInstance = NULL;

bool BooksStorageManager::Private::MountEntry::parse(QString aLine)
{
    QStringList entries = aLine.split(' ', QString::SkipEmptyParts);
    if (entries.count() > 2) {
        dev = entries.at(0);
        path = entries.at(1);
        // If the path contains spaces, those get replaced with \040
        static const QString ENCODED_SPACE("\\040");
        static const QString SPACE(" ");
        path.replace(ENCODED_SPACE, SPACE);
        return true;
    } else {
        dev = path = QString();
        return false;
    }
}

BooksStorageManager::Private::Private(BooksStorageManager* aParent) :
    QObject(aParent),
    iParent(aParent),
    iSettings(BooksSettings::sharedInstance()),
    iUdev(udev_new()),
    iMonitor(NULL),
    iDescriptor(-1),
    iNotifier(NULL),
    iScanMountsTimer(NULL),
    iRunMediaPrefix("/run/media/")
{
    QString homeDevice;
    QString homeBooks = QDir::homePath();
    QString homeMount(BooksStorage::Private::mountPoint(homeBooks));
    if (!homeBooks.endsWith('/')) homeBooks += '/';
    homeBooks += QLatin1String(BOOKS_INTERNAL_ROOT);

    const struct passwd* pw = getpwuid(geteuid());
    if (pw) {
        iRunMediaPrefix += QLatin1String(pw->pw_name);
        iRunMediaPrefix += QChar('/');
    }

    HDEBUG("home mount" << qPrintable(homeMount));
    HDEBUG("home books path" << qPrintable(homeBooks));
    HDEBUG("media mount prefix" << qPrintable(iRunMediaPrefix));

    QFile mounts(STORAGE_MOUNTS_FILE);
    if (mounts.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // For some reason QTextStream can't read /proc/mounts line by line
        QByteArray contents = mounts.readAll();
        QTextStream in(&contents);
        MountEntry entry;
        while (!in.atEnd()) {
            if (entry.parse(in.readLine())) {
                if (entry.path == homeMount) {
                    homeDevice = entry.dev;
                    HDEBUG("internal" << homeDevice);
                } else if (isMediaMount(&entry)) {
                    QString path(entry.path);
                    if (!path.endsWith('/')) path += '/';
                    path += iSettings->removableRoot();
                    BooksStorage bs(entry.dev, entry.path, path,
                        BooksStorage::RemovableStorage);
                    HDEBUG("removable" << entry.dev << bs.booksDir().path());
                    iStorageList.append(bs);
                } else {
                    HDEBUG(entry.dev << entry.path);
                }
            }
        }
        mounts.close();
    }

    iStorageList.insert(0, BooksStorage(homeDevice, homeMount, homeBooks,
        BooksStorage::InternalStorage));

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
                connect(iNotifier, SIGNAL(activated(int)),
                    SLOT(onDeviceEvent(int)));
            }
        }
    }

    connect(iSettings.data(), SIGNAL(removableRootChanged()),
        SLOT(onRemovableRootChanged()));
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

BooksStorage BooksStorageManager::Private::internalStorage() const
{
    const int n = iStorageList.count();
    for (int i = 0; i < n; i++) {
        BooksStorage storage(iStorageList.at(i));
        if (storage.isInternal()) {
            return storage;
        }
    }
    return BooksStorage();
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

bool BooksStorageManager::Private::scanMounts()
{
    bool newStorageFound = false;
    QList<BooksStorage> newMounts;
    QFile mounts(STORAGE_MOUNTS_FILE);
    if (mounts.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // For some reason QTextStream can't read /proc/mounts line by line
        QByteArray contents = mounts.readAll();
        QTextStream in(&contents);
        MountEntry entry;
        while (!in.atEnd()) {
            if (entry.parse(in.readLine()) &&
                isMediaMount(&entry) &&
                findDevice(entry.dev) < 0) {
                QString path(entry.path);
                if (!path.endsWith('/')) path += '/';
                path += iSettings->removableRoot();
                HDEBUG("new removable device" << entry.dev << path);
                BooksStorage storage(entry.dev, entry.path, path,
                    BooksStorage::RemovableStorage);
                iStorageList.append(storage);
                Q_EMIT iParent->storageAdded(storage);
                newStorageFound = true;
            }
        }
        mounts.close();
    }
    return newStorageFound;
}

bool BooksStorageManager::Private::isMediaMount(const MountEntry* aEntry)
{
    return aEntry->path.startsWith(QStringLiteral("/media/")) ||
        aEntry->path.startsWith(iRunMediaPrefix);
}

void BooksStorageManager::Private::onScanMounts()
{
    if (scanMounts()) {
        iScanMountsTimer->stop();
    } else {
        QDateTime now = QDateTime::currentDateTime();
        if (now > iScanDeadline) {
            HDEBUG("timeout waiting for new mount to appear");
            iScanMountsTimer->stop();
        } else {
            HDEBUG("no new mounts found");
        }
    }
}

void BooksStorageManager::Private::onDeviceEvent(int)
{
    struct udev_device* dev = udev_monitor_receive_device(iMonitor);
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
                    if (!iScanMountsTimer) {
                        QTimer* timer = new QTimer(this);
                        timer->setSingleShot(false);
                        timer->setInterval(STORAGE_SCAN_INTERVAL);
                        connect(timer, SIGNAL(timeout()), SLOT(onScanMounts()));
                        iScanMountsTimer = timer;
                    }
                    iScanMountsTimer->start();
                    iScanDeadline = QDateTime::currentDateTime().
                        addMSecs(STORAGE_SCAN_TIMEOUT);
                }
            } else if (!(strcmp(action, STORAGE_ACTION_REMOVE))) {
                int pos = findDevice(devnode);
                if (pos >= 0) {
                    HDEBUG("removable device is gone");
                    BooksStorage storage = iStorageList.takeAt(pos);
                    storage.iPrivate->iPresent = false;
                    Q_EMIT storage.iPrivate->removed();
                    Q_EMIT iParent->storageRemoved(storage);
                }
            }
        }
        udev_device_unref(dev);
    } else {
        HWARN("no device!");
    }
}

void BooksStorageManager::Private::onRemovableRootChanged()
{
    int i;
    HDEBUG(iSettings->removableRoot());
    QList<BooksStorage> replaced; // old-new pairs
    for (i=iStorageList.count()-1; i>=0; i--) {
        BooksStorage storage = iStorageList.at(i);
        if (!storage.isInternal()) {
            QString path(storage.iPrivate->iMountPoint);
            if (!path.endsWith('/')) path += '/';
            path += iSettings->removableRoot();
            BooksStorage updated(storage.iPrivate->iDevice,
                storage.iPrivate->iMountPoint, path,
                BooksStorage::RemovableStorage);
            if (!storage.equal(updated)) {
                replaced.append(storage);
                replaced.append(updated);
                iStorageList.replace(i, updated);
            } else {
                HWARN(storage.root() << "didn't change");
            }
        }
    }
    for (i=0; (i+1)<replaced.count(); i+=2) {
        Q_EMIT iParent->storageReplaced(replaced.at(i), replaced.at(i+1));
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
    iPrivate(new Private(this))
{
}

BooksStorageManager::~BooksStorageManager()
{
    if (Private::gInstance == this) {
        Private::gInstance = NULL;
    }
}

int BooksStorageManager::count() const
{
    return iPrivate->iStorageList.count();
}

QList<BooksStorage> BooksStorageManager::storageList() const
{
    return iPrivate->iStorageList;
}

BooksStorage BooksStorageManager::internalStorage() const
{
    return iPrivate->internalStorage();
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

#include "BooksStorage.moc"
