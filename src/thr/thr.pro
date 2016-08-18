include(../common.pri)

TARGET      = thr
TEMPLATE    = lib
DESTDIR     = .

CONFIG      += staticlib
INCLUDES    += $$PWD

OBJECTS_DIR = .obj
MOC_DIR     = .moc
RCC_DIR     = .rcc

msvc:LIBS            += ../util/util.lib
msvc:PRE_TARGETDEPS  += ../util/util.lib

mingw:LIBS            += -L"$$OUT_PWD/../util/" -lutil
mingw:PRE_TARGETDEPS  += ../util/libutil.a

gcc:LIBS            += -L"$$OUT_PWD/../util/" -lutil
gcc:PRE_TARGETDEPS  += ../util/libutil.a

INCLUDEPATH += ..
DEPENDPATH  += ..

SOURCES += \
    Worker.cpp \
    TaskQueue.cpp \
    Task.cpp \
    Paralleler.cpp

HEADERS += \
    Worker.h \
    TaskQueue.h \
    Task.h \
    Paralleler.h
