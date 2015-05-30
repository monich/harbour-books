TEMPLATE = lib
TARGET = fribidi
CONFIG += staticlib link_pkgconfig
PKGCONFIG += glib-2.0

!include(../common.pri)

SRC_DIR = fribidi

INCLUDEPATH += gen $$SRC_DIR/lib $$SRC_DIR/charset

DEFINES += USE_SIMPLE_MALLOC=1

SOURCES += \
  $$SRC_DIR/lib/fribidi.c \
  $$SRC_DIR/lib/fribidi-arabic.c \
  $$SRC_DIR/lib/fribidi-bidi.c \
  $$SRC_DIR/lib/fribidi-bidi-types.c \
  $$SRC_DIR/lib/fribidi-deprecated.c \
  $$SRC_DIR/lib/fribidi-joining.c \
  $$SRC_DIR/lib/fribidi-joining-types.c \
  $$SRC_DIR/lib/fribidi-mem.c \
  $$SRC_DIR/lib/fribidi-mirroring.c \
  $$SRC_DIR/lib/fribidi-run.c \
  $$SRC_DIR/lib/fribidi-shape.c

HEADERS += \
  $$SRC_DIR/lib/bidi-types.h \
  $$SRC_DIR/lib/common.h \
  $$SRC_DIR/lib/debug.h \
  $$SRC_DIR/lib/fribidi-arabic.h \
  $$SRC_DIR/lib/fribidi-begindecls.h \
  $$SRC_DIR/lib/fribidi-bidi.h \
  $$SRC_DIR/lib/fribidi-bidi-types.h \
  $$SRC_DIR/lib/fribidi-bidi-types-list.h \
  $$SRC_DIR/lib/fribidi-common.h \
  $$SRC_DIR/lib/fribidi-deprecated.h \
  $$SRC_DIR/lib/fribidi-enddecls.h \
  $$SRC_DIR/lib/fribidi-flags.h \
  $$SRC_DIR/lib/fribidi.h \
  $$SRC_DIR/lib/fribidi-joining.h \
  $$SRC_DIR/lib/fribidi-joining-types.h \
  $$SRC_DIR/lib/fribidi-joining-types-list.h \
  $$SRC_DIR/lib/fribidi-mirroring.h \
  $$SRC_DIR/lib/fribidi-shape.h \
  $$SRC_DIR/lib/fribidi-types.h \
  $$SRC_DIR/lib/fribidi-unicode.h \
  $$SRC_DIR/lib/joining-types.h \
  $$SRC_DIR/lib/mem.h \
  $$SRC_DIR/lib/run.h

HEADERS += \
  gen/fribidi-config.h \
  gen/fribidi-unicode-version.h
