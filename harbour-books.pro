TEMPLATE = subdirs
CONFIG += ordered app_settings
SUBDIRS = fribidi linebreak fbreader app

app_settings {
  SUBDIRS += settings
  settings.file = app/settings/settings.pro
}

OTHER_FILES += \
    README.md \
    rpm/*.spec
