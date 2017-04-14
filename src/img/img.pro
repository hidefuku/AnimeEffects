include(../common.pri)

TARGET      = img
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
    PSDReader.cpp \
    PSDUtil.cpp \
    PSDWriter.cpp \
    Buffer.cpp \
    ResourceNode.cpp \
    Util.cpp \
    BlendMode.cpp \
    GridMeshCreator.cpp \
    ResourceData.cpp \
    ResourceHandle.cpp \
    BlendModeName.cpp

HEADERS += \
    Buffer.h \
    PSDFormat.h \
    PSDReader.h \
    PSDUtil.h \
    PSDWriter.h \
    Format.h \
    PixelPos.h \
    Quad.h \
    ResourceNode.h \
    ResourceHandle.h \
    Util.h \
    ColorRGBA.h \
    BlendMode.h \
    GridMeshCreator.h \
    ResourceData.h \
    BlendModeName.h
