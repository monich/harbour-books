TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = fribidi linebreak fbreader harbour-lib app

OTHER_FILES += \
    rpm/harbour-books.changes \
    rpm/harbour-books.spec
