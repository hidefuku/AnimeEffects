QT      += core gui opengl xml
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

DEFINES += "AE_MAJOR_VERSION=1"
DEFINES += "AE_MINOR_VERSION=1"

DEFINES += "AE_PROJECT_FORMAT_MAJOR_VERSION=0"
DEFINES += "AE_PROJECT_FORMAT_MINOR_VERSION=4"

DEFINES += "AE_PROJECT_FORMAT_OLDEST_MAJOR_VERSION=0"
DEFINES += "AE_PROJECT_FORMAT_OLDEST_MINOR_VERSION=4"

# OpenGL CoreProfile Option
unix {
DEFINES += USE_GL_CORE_PROFILE
}
unix:!macx {
QMAKE_RPATHDIR += ./
}
macx {
QMAKE_RPATHDIR += ../Frameworks
}
