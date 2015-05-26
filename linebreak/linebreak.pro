TEMPLATE = lib
TARGET = linebreak
CONFIG += staticlib

!include(../common.pri)

SRC_DIR = linebreak

SOURCES += \
  $$SRC_DIR/linebreak.c \
  $$SRC_DIR/linebreakdata.c \
  $$SRC_DIR/linebreakdef.c \
  $$SRC_DIR/wordbreak.c

HEADERS += \
  $$SRC_DIR/linebreakdef.h \
  $$SRC_DIR/linebreak.h \
  $$SRC_DIR/wordbreakdef.h \
  $$SRC_DIR/wordbreak.h
