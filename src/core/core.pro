include(../common.pri)

TARGET      = core
TEMPLATE    = lib
DESTDIR     = .

CONFIG      += staticlib
INCLUDES    += $$PWD

OBJECTS_DIR = .obj
MOC_DIR     = .moc
RCC_DIR     = .rcc

msvc:LIBS            += ../util/util.lib ../thr/thr.lib ../cmnd/cmnd.lib ../gl/gl.lib ../img/img.lib
msvc:PRE_TARGETDEPS  += ../util/util.lib ../thr/thr.lib ../cmnd/cmnd.lib ../gl/gl.lib ../img/img.lib

mingw:LIBS            += \
    -L"$$OUT_PWD/../img/"  -limg \
    -L"$$OUT_PWD/../gl/"   -lgl \
    -L"$$OUT_PWD/../cmnd/" -lcmnd \
    -L"$$OUT_PWD/../thr/"  -lthr \
    -L"$$OUT_PWD/../util/" -lutil

mingw:PRE_TARGETDEPS  += \
    ../img/libimg.a \
    ../gl/libgl.a \
    ../cmnd/libcmnd.a \
    ../util/libutil.a

gcc:LIBS            += \
    -L"$$OUT_PWD/../img/"  -limg \
    -L"$$OUT_PWD/../gl/"   -lgl \
    -L"$$OUT_PWD/../cmnd/" -lcmnd \
    -L"$$OUT_PWD/../thr/"  -lthr \
    -L"$$OUT_PWD/../util/" -lutil

gcc:PRE_TARGETDEPS  += \
    ../img/libimg.a \
    ../gl/libgl.a \
    ../cmnd/libcmnd.a \
    ../util/libutil.a

INCLUDEPATH += ..
DEPENDPATH  += ..

SOURCES += \
    AbstractCursor.cpp \
    CameraInfo.cpp \
    GridMesh.cpp \
    LayerNode.cpp \
    ObjectNodeUtil.cpp \
    ObjectTree.cpp \
    Project.cpp \
    TimeLine.cpp \
    FFDKey.cpp \
    HeightMap.cpp \
    Deserializer.cpp \
    Serializer.cpp \
    BoneKey.cpp \
    Bone2.cpp \
    PoseKey.cpp \
    PosePalette.cpp \
    ObjectTreeEvent.cpp \
    ObjectTreeNotifier.cpp \
    TimeKey.cpp \
    BoneShape.cpp \
    TimeKeyGatherer.cpp \
    BoneInfluenceMap.cpp \
    TimeKeyBlender.cpp \
    TimeKeyExpans.cpp \
    MeshTransformer.cpp \
    ResourceHolder.cpp \
    OpaKey.cpp \
    MeshKey.cpp \
    LayerMesh.cpp \
    ResourceEvent.cpp \
    FFDKeyUpdater.cpp \
    BoneKeyUpdater.cpp \
    ClippingFrame.cpp \
    ShaderHolder.cpp \
    TimeCacheLock.cpp \
    TimeCacheAccessor.cpp \
    TimeLineEvent.cpp \
    MeshKeyUtil.cpp \
    ProjectEvent.cpp \
    MeshTransformerResource.cpp \
    ImageKey.cpp \
    ImageKeyUpdater.cpp \
    ResourceUpdatingWorkspace.cpp \
    BoneExpans.cpp \
    FolderNode.cpp \
    DestinationTexturizer.cpp \
    MoveKey.cpp \
    RotateKey.cpp \
    ScaleKey.cpp \
    SRTExpans.cpp

HEADERS += \
    AbstractCursor.h \
    CameraInfo.h \
    TimeKeyType.h \
    GridMesh.h \
    LayerNode.h \
    ObjectNode.h \
    ObjectNodeUtil.h \
    ObjectTree.h \
    PenInfo.h \
    Project.h \
    Renderer.h \
    RenderInfo.h \
    TimeInfo.h \
    TimeKey.h \
    TimeLine.h \
    Animator.h \
    TimeLineEvent.h \
    TimeKeyPos.h \
    Constant.h \
    FFDKey.h \
    ObjectType.h \
    HeightMap.h \
    Serializer.h \
    Deserializer.h \
    BoneKey.h \
    Bone2.h \
    PoseKey.h \
    PosePalette.h \
    ObjectTreeEvent.h \
    ObjectTreeNotifier.h \
    BoneShape.h \
    TimeKeyGatherer.h \
    BoneInfluenceMap.h \
    TimeKeyBlender.h \
    TimeKeyExpans.h \
    SRTExpans.h \
    MeshTransformer.h \
    ResourceHolder.h \
    OpaKey.h \
    MeshKey.h \
    LayerMesh.h \
    ResourceEvent.h \
    FFDKeyUpdater.h \
    BoneKeyUpdater.h \
    ClippingFrame.h \
    ShaderHolder.h \
    TimeCacheLock.h \
    TimeCacheAccessor.h \
    Frame.h \
    MeshKeyUtil.h \
    ProjectEvent.h \
    MeshTransformerResource.h \
    ImageKey.h \
    ImageKeyUpdater.h \
    ResourceUpdatingWorkspace.h \
    BoneExpans.h \
    FolderNode.h \
    DestinationTexturizer.h \
    MoveKey.h \
    RotateKey.h \
    ScaleKey.h
