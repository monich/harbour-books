TEMPLATE = lib
TARGET = bookssettings

# Directories
TARGETPATH = $$[QT_INSTALL_QML]/openrepos/books/settings
APP_SRC_DIR = ../src
HARBOUR_LIB_DIR = ../../harbour-lib
HARBOUR_LIB_INCLUDE_DIR = $$HARBOUR_LIB_DIR/include
HARBOUR_LIB_SRC_DIR = $$HARBOUR_LIB_DIR/src

QT += qml
CONFIG += qt plugin link_pkgconfig hide_symbols
PKGCONFIG += mlite5

DEFINES += OPENREPOS # It's for openrepos build only
QMAKE_CXXFLAGS += -Wno-unused-parameter
INCLUDEPATH += $${APP_SRC_DIR} $${HARBOUR_LIB_INCLUDE_DIR}

CONFIG(debug, debug|release) {
    DEFINES += HARBOUR_DEBUG=1
}

target.path = $$TARGETPATH
INSTALLS += target

import.files = qmldir
import.path = $$TARGETPATH
INSTALLS += import

settings_json.files = openrepos-books.json
settings_json.path = /usr/share/jolla-settings/entries/
INSTALLS += settings_json

SOURCES += \
  BooksSettingsPlugin.cpp \
  $${APP_SRC_DIR}/BooksColorScheme.cpp \
  $${APP_SRC_DIR}/BooksColorSchemeModel.cpp \
  $${APP_SRC_DIR}/BooksSettingsBase.cpp

HEADERS += \
  $${APP_SRC_DIR}/BooksColorScheme.h \
  $${APP_SRC_DIR}/BooksColorSchemeModel.h \
  $${APP_SRC_DIR}/BooksSettingsBase.h

OTHER_FILES += \
  settings/*.json

# harbour-lib

HEADERS += \
  $${HARBOUR_LIB_INCLUDE_DIR}/HarbourColorEditorModel.h \
  $${HARBOUR_LIB_INCLUDE_DIR}/HarbourDebug.h \
  $${HARBOUR_LIB_INCLUDE_DIR}/HarbourUtil.h

SOURCES += \
  $${HARBOUR_LIB_SRC_DIR}/HarbourColorEditorModel.cpp \
  $${HARBOUR_LIB_SRC_DIR}/HarbourUtil.cpp
