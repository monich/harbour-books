CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0

WARNINGS = -Wall -Wno-unused-parameter -Wno-deprecated-declarations -Wno-missing-field-initializers
QMAKE_CXXFLAGS += $$WARNINGS -Wno-psabi
QMAKE_CFLAGS += $$WARNINGS

CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS_DEBUG *= -O0
  QMAKE_CFLAGS_DEBUG *= -O0
  DEFINES += HARBOUR_DEBUG=1
}

# Directories
FBREADER_DIR = $$_PRO_FILE_PWD_/../fbreader/fbreader

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
  -lbz2 -lz -lexpat -lmagic

INCLUDEPATH += \
  $$FBREADER_DIR/zlibrary/core/include \
  $$FBREADER_DIR/zlibrary/text/include \
  $$FBREADER_DIR/zlibrary/ui/src/qt4 \
  $$FBREADER_DIR/fbreader/src/formats/css
