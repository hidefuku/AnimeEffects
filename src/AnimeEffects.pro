#-------------------------------------------------
#
# Project created by QtCreator 2014-10-30T19:23:19
#
#-------------------------------------------------

TEMPLATE    = subdirs
SUBDIRS     = util thr cmnd gl img core ctrl gui

CONFIG += ordered

# copy directory
win32 {
copydata.commands = $(COPY_DIR) $$shell_path($$PWD/data) $$shell_path($$OUT_PWD/data)
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
}
unix {
copydata.commands = rsync -ru $$shell_path($$PWD/data) $$shell_path($$OUT_PWD)
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
}
