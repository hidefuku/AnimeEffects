#-------------------------------------------------
#
# Project created by QtCreator 2014-10-30T19:23:19
#
#-------------------------------------------------

TARGET = AnimeEffects

TEMPLATE    = subdirs
SUBDIRS     = util thr cmnd gl img core ctrl gui

CONFIG += ordered

TRANSLATIONS = ../data/locale/translation_ja.ts

# copy directory
win32 {
copydata.commands = $(COPY_DIR) $$shell_path($$PWD/../data) $$shell_path($$OUT_PWD/data)
copytools.commands = $(COPY_DIR) $$shell_path($$PWD/../tools) $$shell_path($$OUT_PWD/tools)
first.depends = $(first) copydata copytools
export(first.depends)
export(copydata.commands)
export(copytools.commands)
QMAKE_EXTRA_TARGETS += first copydata copytools
}
unix:!macx {
copydata.commands = rsync -ru $$shell_path($$PWD/../data) $$shell_path($$OUT_PWD)
copytools.commands = rsync -ru $$shell_path($$PWD/../tools) $$shell_path($$OUT_PWD)
first.depends = $(first) copydata copytools
export(first.depends)
export(copydata.commands)
export(copytools.commands)
QMAKE_EXTRA_TARGETS += first copydata copytools
}
macx {
copydata.commands = rsync -ru $$shell_path($$PWD/../data) $$shell_path($$OUT_PWD/AnimeEffects.app)
copytools.commands = rsync -ru $$shell_path($$PWD/../tools) $$shell_path($$OUT_PWD/AnimeEffects.app)
first.depends = $(first) copydata copytools
export(first.depends)
export(copydata.commands)
export(copytools.commands)
QMAKE_EXTRA_TARGETS += first copydata copytools
}

# Installs
unix{
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    target.path = $$PREFIX/bin

    shortcutfiles.files = dist/AnimeEffects.desktop
    shortcutfiles.path = $$PREFIX/share/applications/
    iconfiles.files = dist/AnimeEffects.png
    iconfiles.path = $$PREFIX/share/icons/hicolor/256x256/

    INSTALLS += target
    INSTALLS += shortcutfiles
    INSTALLS += iconfiles
}
