include(../common.pri)

TARGET      = cmnd
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
    Stack.cpp \
    Scalable.cpp

HEADERS += \
    Base.h \
    BasicCommands.h \
    Listener.h \
    ScopedMacro.h \
    ScopedUndoSuspender.h \
    SleepableObject.h \
    Stack.h \
    SignalNotifier.h \
    Scalable.h \
    UndoneDeleter.h \
    DoneDeleter.h \
    Stable.h \
    Vector.h
