TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = fribidi linebreak fbreader app

OTHER_FILES += \
    README.md \
    rpm/*.spec
