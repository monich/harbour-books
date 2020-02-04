NAME = books

openrepos {
    PREFIX = openrepos
    DEFINES += OPENREPOS
} else {
    PREFIX = harbour
}

TARGET = $${PREFIX}-$${NAME}
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5 glib-2.0

app_settings {
    # This path is hardcoded in jolla-settings
    TRANSLATIONS_PATH = /usr/share/translations
} else {
    TRANSLATIONS_PATH = /usr/share/$${TARGET}/translations
}

!include(../common.pri)

CONFIG(debug, debug|release) {
  DEFINES += HARBOUR_DEBUG=1
}

# Directories
FBREADER_DIR = $$_PRO_FILE_PWD_/../fbreader
FRIBIDI_DIR = $$_PRO_FILE_PWD_/../fribidi
LINEBREAK_DIR = $$_PRO_FILE_PWD_/../linebreak
HARBOUR_LIB_DIR = $$_PRO_FILE_PWD_/../harbour-lib

HARBOUR_INCLUDE_DIR = $$HARBOUR_LIB_DIR/include
HARBOUR_SRC_DIR = $$HARBOUR_LIB_DIR/src
HARBOUR_LIB_QML = $$HARBOUR_LIB_DIR/qml

# Libraries
FBREADER_LIB = $$OUT_PWD/../fbreader/libfbreader.a
FRIBIDI_LIB = $$OUT_PWD/../fribidi/libfribidi.a
LINEBREAK_LIB = $$OUT_PWD/../linebreak/liblinebreak.a

PRE_TARGETDEPS += \
  $$FBREADER_LIB \
  $$FRIBIDI_LIB \
  $$LINEBREAK_LIB
LIBS += \
  $$FBREADER_LIB \
  $$FRIBIDI_LIB \
  $$LINEBREAK_LIB \
  -lbz2 -lz -ldl

OTHER_FILES += \
  icons/harbour-books.svg \
  harbour-books.desktop \
  qml/*.qml \
  qml/images/* \
  settings/*.qml \
  settings/*.json \
  data/default/* \
  data/zlibrary/core/encodings/* \
  data/zlibrary/core/resources/*

TARGET_DATA_DIR = /usr/share/$$TARGET
TARGET_DEFAULT_DATA_DIR = $$TARGET_DATA_DIR/data
TARGET_ZLIBRARY_DATA_DIR = $$TARGET_DEFAULT_DATA_DIR
TARGET_ICON_ROOT = /usr/share/icons/hicolor

core_data.files = \
  data/zlibrary/core/*.gz \
  data/zlibrary/core/*.zip \
  data/zlibrary/core/encodings \
  data/zlibrary/core/resources
core_data.path = $$TARGET_ZLIBRARY_DATA_DIR
INSTALLS += core_data

text_data.files = data/zlibrary/text/*.zip
text_data.path = $$TARGET_ZLIBRARY_DATA_DIR
INSTALLS += text_data

defaults.files = data/default/*
defaults.path = $$TARGET_ZLIBRARY_DATA_DIR
INSTALLS += defaults

formats.files = data/formats/*
formats.path = $$TARGET_DEFAULT_DATA_DIR/formats
INSTALLS += formats

samples.files = data/samples/*
samples.path = $$TARGET_DEFAULT_DATA_DIR/samples
INSTALLS += samples

INCLUDEPATH += \
  src \
  $$HARBOUR_INCLUDE_DIR \
  $$FBREADER_DIR/fbreader/fbreader/src \
  $$FBREADER_DIR/fbreader/zlibrary/text/include \
  $$FBREADER_DIR/fbreader/zlibrary/core/include \
  $$FBREADER_DIR/fbreader/zlibrary/core/src/view \
  $$FBREADER_DIR/fbreader/zlibrary/core/src/dialogs \
  $$FBREADER_DIR/fbreader/zlibrary/core/src/application \
  $$FBREADER_DIR/fbreader/zlibrary/core/src/options \
  $$FBREADER_DIR/fbreader/zlibrary/core/src/unix \
  $$FBREADER_DIR/fbreader/zlibrary/ui/src/qt4

SOURCES += \
  src/BooksBook.cpp \
  src/BooksBookModel.cpp \
  src/BooksConfig.cpp \
  src/BooksCoverModel.cpp \
  src/BooksCoverWidget.cpp \
  src/BooksDialogManager.cpp \
  src/BooksFeedback.cpp \
  src/BooksHints.cpp \
  src/BooksImageProvider.cpp \
  src/BooksImportModel.cpp \
  src/BooksListWatcher.cpp \
  src/BooksLoadingProperty.cpp \
  src/BooksMediaPlugin.cpp \
  src/BooksPageStack.cpp \
  src/BooksPageWidget.cpp \
  src/BooksPaintContext.cpp \
  src/BooksPathModel.cpp \
  src/BooksPolicyPlugin.cpp \
  src/BooksPos.cpp \
  src/BooksSaveTimer.cpp \
  src/BooksSettings.cpp \
  src/BooksShelf.cpp \
  src/BooksStorage.cpp \
  src/BooksStorageModel.cpp \
  src/BooksTextStyle.cpp \
  src/BooksTaskQueue.cpp \
  src/BooksTextView.cpp \
  src/BooksUtil.cpp \
  src/main.cpp \
  src/ZLApplication.cpp \
  src/ZLibrary.cpp

# Some libraries are not allowed in harbour
openrepos {
  LIBS += -lexpat -lmagic -ludev
} else {
  SOURCES += \
    stubs/libexpat.c \
    stubs/libmagic.c \
    stubs/libudev.c
}

HEADERS += \
  src/BooksBook.h \
  src/BooksBookModel.h \
  src/BooksConfig.h \
  src/BooksCoverModel.h \
  src/BooksCoverWidget.h \
  src/BooksDefs.h \
  src/BooksDialogManager.h \
  src/BooksFeedback.h \
  src/BooksHints.h \
  src/BooksImageProvider.h \
  src/BooksImportModel.h \
  src/BooksItem.h \
  src/BooksListWatcher.h \
  src/BooksLoadingProperty.h \
  src/BooksMediaPlugin.h \
  src/BooksPageStack.h \
  src/BooksPageWidget.h \
  src/BooksPaintContext.h \
  src/BooksPathModel.h \
  src/BooksPolicyPlugin.h \
  src/BooksPos.h \
  src/BooksSaveTimer.h \
  src/BooksSettings.h \
  src/BooksShelf.h \
  src/BooksStorage.h \
  src/BooksStorageModel.h \
  src/BooksTaskQueue.h \
  src/BooksTextView.h \
  src/BooksTextStyle.h \
  src/BooksTypes.h \
  src/BooksUtil.h

# harbour-lib

HEADERS += \
  $$HARBOUR_INCLUDE_DIR/HarbourDisplayBlanking.h \
  $$HARBOUR_INCLUDE_DIR/HarbourImageProvider.h \
  $$HARBOUR_INCLUDE_DIR/HarbourJson.h \
  $$HARBOUR_INCLUDE_DIR/HarbourPluginLoader.h \
  $$HARBOUR_INCLUDE_DIR/HarbourTask.h \
  $$HARBOUR_INCLUDE_DIR/HarbourTheme.h

HEADERS += \
  $$HARBOUR_SRC_DIR/HarbourMce.h

SOURCES += \
  $$HARBOUR_SRC_DIR/HarbourDisplayBlanking.cpp \
  $$HARBOUR_SRC_DIR/HarbourImageProvider.cpp \
  $$HARBOUR_SRC_DIR/HarbourJson.cpp \
  $$HARBOUR_SRC_DIR/HarbourMce.cpp \
  $$HARBOUR_SRC_DIR/HarbourPluginLoader.cpp \
  $$HARBOUR_SRC_DIR/HarbourTask.cpp \
  $$HARBOUR_SRC_DIR/HarbourTheme.cpp

HARBOUR_QML_COMPONENTS = \
    $$HARBOUR_LIB_QML/HarbourFitLabel.qml \
    $$HARBOUR_LIB_QML/HarbourHorizontalSwipeHint.qml

OTHER_FILES += $${HARBOUR_QML_COMPONENTS}

qml_components.files = $${HARBOUR_QML_COMPONENTS}
qml_components.path = /usr/share/$${TARGET}/qml/harbour
INSTALLS += qml_components

# Icons
ICON_SIZES = 86 108 128 256
for(s, ICON_SIZES) {
    icon_target = icon$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    equals(PREFIX, "openrepos") {
        $${icon_target}.extra = cp $${icon_dir}/harbour-$${NAME}.png $$eval($${icon_target}.files)
        $${icon_target}.CONFIG += no_check_exist
    }
    INSTALLS += $${icon_target}
}

# Settings
app_settings {
    settings_json.files = settings/$${TARGET}.json
    settings_json.path = /usr/share/jolla-settings/entries/
    equals(PREFIX, "openrepos") {
        settings_json.extra = sed s/harbour/openrepos/g settings/harbour-$${NAME}.json > $$eval(settings_json.files)
        settings_json.CONFIG += no_check_exist
    }
    INSTALLS += settings_json
}

settings_qml.files = settings/*.qml
settings_qml.path = /usr/share/$${TARGET}/settings/
INSTALLS += settings_qml

# Desktop file
equals(PREFIX, "openrepos") {
    desktop.extra = sed s/harbour/openrepos/g harbour-$${NAME}.desktop > $${TARGET}.desktop
    desktop.CONFIG += no_check_exist
}

# Translations
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml \
  $${_PRO_FILE_PWD_}/settings

defineTest(addTrFile) {
    rel = translations/harbour-$${1}
    OTHER_FILES += $${rel}.ts

    in = $${_PRO_FILE_PWD_}/$$rel
    out = $${OUT_PWD}/translations/$${PREFIX}-$$1

    s = $$replace(1,-,_)
    lupdate_target = lupdate_$$s
    lrelease_target = lrelease_$$s

    $${lupdate_target}.commands = lupdate -noobsolete $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${lrelease_target}.target = $${out}.qm
    $${lrelease_target}.depends = $${lupdate_target}
    $${lrelease_target}.commands = lrelease -idbased \"$${out}.ts\"

    QMAKE_EXTRA_TARGETS += $${lrelease_target} $${lupdate_target}
    PRE_TARGETDEPS += $${out}.qm
    qm.files += $${out}.qm
    export($${lupdate_target}.commands)
    export($${lrelease_target}.target)
    export($${lrelease_target}.depends)
    export($${lrelease_target}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(PRE_TARGETDEPS)
    export(OTHER_FILES)
    export(qm.files)
}

LANGUAGES = de fi hu nl pl pt_BR ru sv es zh_CN

addTrFile($${NAME})
for(l, LANGUAGES) {
    addTrFile($${NAME}-$$l)
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm
