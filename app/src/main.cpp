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

#include "BooksDefs.h"
#include "BooksPos.h"
#include "BooksShelf.h"
#include "BooksBook.h"
#include "BooksBookModel.h"
#include "BooksCoverModel.h"
#include "BooksConfig.h"
#include "BooksImageProvider.h"
#include "BooksImportModel.h"
#include "BooksPathModel.h"
#include "BooksPageStack.h"
#include "BooksStorageModel.h"
#include "BooksPageWidget.h"
#include "BooksListWatcher.h"
#include "BooksCoverWidget.h"
#include "BooksTaskQueue.h"
#include "BooksHints.h"

#include "HarbourDisplayBlanking.h"
#include "HarbourDebug.h"
#include "HarbourMediaPlugin.h"
#include "HarbourPolicyPlugin.h"
#include "HarbourTheme.h"

#include "ZLibrary.h"
#include "ZLLanguageUtil.h"

#include <sailfishapp.h>

#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>

// If the current task is stuck for too long after being canceled it's
// probably stuck forever. Let "too long" be 10 seconds.
#if HARBOUR_DEBUG
   // But let it get stuck forever in debug build
#  define TASK_QUEUE_TIMEOUT (-1)
#else
   // There's no reason to wait forever in release build though.
#  define TASK_QUEUE_TIMEOUT (10000)
#endif

#ifdef OPENREPOS
#  include "BooksDBus.h"
#endif

Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    qRegisterMetaType<BooksPos>();
    BOOKS_QML_REGISTER(BooksShelf, "Shelf");
    BOOKS_QML_REGISTER(BooksBook, "Book");
    BOOKS_QML_REGISTER(BooksBookModel, "BookModel");
    BOOKS_QML_REGISTER(BooksCoverModel, "CoverModel");
    BOOKS_QML_REGISTER(BooksImportModel, "BooksImportModel");
    BOOKS_QML_REGISTER(BooksPathModel, "BooksPathModel");
    BOOKS_QML_REGISTER(BooksPageStack, "BooksPageStack");
    BOOKS_QML_REGISTER(BooksStorageModel, "BookStorage");
    BOOKS_QML_REGISTER(BooksPageWidget, "PageWidget");
    BOOKS_QML_REGISTER(BooksListWatcher, "ListWatcher");
    BOOKS_QML_REGISTER(BooksCoverWidget, "BookCover");
    BOOKS_QML_REGISTER(BooksSettings, "BooksSettings");
    BOOKS_QML_REGISTER(HarbourDisplayBlanking, "DisplayBlanking");
    BOOKS_QML_REGISTER_SINGLETON(HarbourTheme, "HarbourTheme");
    BOOKS_QML_REGISTER_SINGLETON(BooksHints, "BooksHints");

    QLocale locale;
    QTranslator* translator = new QTranslator(app);
#ifdef OPENREPOS
    QString transDir("/usr/share/translations");
#else
    QString transDir = SailfishApp::pathTo("translations").toLocalFile();
#endif
    QString transFile(BOOKS_APP_NAME);
    if (translator->load(locale, transFile, "-", transDir) ||
        translator->load(transFile, transDir)) {
        app->installTranslator(translator);
    } else {
        HDEBUG("Failed to load translator for" << locale);
        delete translator;
    }

    BooksConfigManager configManager;
    if (ZLibrary::init(argc, argv)) {
        if (ZLLanguageUtil::isRTLLanguage(ZLibrary::Language())) {
            qApp->setLayoutDirection(Qt::RightToLeft);
        }

        const QString qml(QString::fromStdString(ZLibrary::BaseDirectory +
            BOOKS_QML_FILE));
        HDEBUG("qml file" << qPrintable(qml));

        QQuickView* view = SailfishApp::createView();
        QQmlContext* root = view->rootContext();
        QQmlEngine* engine = root->engine();
        QSharedPointer<BooksSettings> settings = BooksSettings::sharedInstance();
        HarbourPolicyPlugin::registerTypes(engine, BOOKS_QML_PLUGIN,
            BOOKS_QML_PLUGIN_V1, BOOKS_QML_PLUGIN_V2);
        HarbourMediaPlugin::registerTypes(engine, BOOKS_QML_PLUGIN,
            BOOKS_QML_PLUGIN_V1, BOOKS_QML_PLUGIN_V2);
        engine->addImageProvider(BooksImageProvider::PROVIDER_ID,
            new BooksImageProvider(root));

        root->setContextProperty("PointsPerInch", booksPPI);
        root->setContextProperty("MaximumHintCount", 1);
        root->setContextProperty("BooksSettingsMenu",
            QVariant::fromValue(BOOKS_SETTINGS_MENU));
        root->setContextProperty("Settings", settings.data());

#ifdef BOOKS_DBUS_INTERFACE
        BooksDBus* dbusHandler = BooksDBus::create(app);
        if (dbusHandler) {
            view->connect(dbusHandler, SIGNAL(activate()), SLOT(raise()));
            settings->connect(dbusHandler, SIGNAL(openBook(QString)),
                SLOT(setCurrentBookPath(QString)));
        }
#endif

        if (argc > 1) {
            const QString file(QString::fromLocal8Bit(argv[1]));
            if (QFile::exists(file)) {
                settings->setCurrentBookPath(file);
            } else {
                HWARN(qPrintable(file) << "doesn't exist");
            }
        }

        view->setTitle(qtTrId("harbour-books-app-name"));
        view->setSource(QUrl::fromLocalFile(qml));
        view->show();

        ZLibrary::run(NULL);
        BooksTaskQueue::waitForDone(TASK_QUEUE_TIMEOUT);
        ZLibrary::shutdown();
    }

    delete app;
    return 0;
}
