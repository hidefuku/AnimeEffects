QT      += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG  += qt c++11
CONFIG  -= debug_and_release
CONFIG  -= debug_and_release_target

INCLUDES += $$PWD

HEADERS += \
    $$PWD/XCReport.h \
    $$PWD/XCType.h \
    $$PWD/XC.h \
    $$PWD/XCAssert.h
