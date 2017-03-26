include(../common.pri)

TARGET      = ctrl
TEMPLATE    = lib
DESTDIR     = .

CONFIG      += staticlib
INCLUDES    += $$PWD

OBJECTS_DIR = .obj
MOC_DIR     = .moc
RCC_DIR     = .rcc

msvc:LIBS            += ../util/util.lib ../thr/thr.lib ../cmnd/cmnd.lib ../gl/gl.lib ../img/img.lib ../core/core.lib
msvc:PRE_TARGETDEPS  += ../util/util.lib ../thr/thr.lib ../cmnd/cmnd.lib ../gl/gl.lib ../img/img.lib ../core/core.lib

mingw:LIBS            += \
    -L"$$OUT_PWD/../core/" -lcore \
    -L"$$OUT_PWD/../img/"  -limg \
    -L"$$OUT_PWD/../gl/"   -lgl \
    -L"$$OUT_PWD/../cmnd/" -lcmnd \
    -L"$$OUT_PWD/../thr/"  -lthr \
    -L"$$OUT_PWD/../util/" -lutil

mingw:PRE_TARGETDEPS  += \
    ../core/libcore.a \
    ../img/libimg.a \
    ../gl/libgl.a \
    ../cmnd/libcmnd.a \
    ../util/libutil.a

gcc:LIBS            += \
    -L"$$OUT_PWD/../core/" -lcore \
    -L"$$OUT_PWD/../img/"  -limg \
    -L"$$OUT_PWD/../gl/"   -lgl \
    -L"$$OUT_PWD/../cmnd/" -lcmnd \
    -L"$$OUT_PWD/../thr/"  -lthr \
    -L"$$OUT_PWD/../util/" -lutil

gcc:PRE_TARGETDEPS  += \
    ../core/libcore.a \
    ../img/libimg.a \
    ../gl/libgl.a \
    ../cmnd/libcmnd.a \
    ../util/libutil.a

INCLUDEPATH += ..
DEPENDPATH  += ..

SOURCES += \
    Driver.cpp \
    TimeLineEditor.cpp \
    TimeLineUtil.cpp \
    SRTEditor.cpp \
    ImageFileLoader.cpp \
    FFDEditor.cpp \
    ProjectLoader.cpp \
    ProjectSaver.cpp \
    System.cpp \
    BoneEditor.cpp \
    PoseEditor.cpp \
    bone/bone_CreateMode.cpp \
    bone/bone_Renderer.cpp \
    bone/bone_KeyOwner.cpp \
    bone/bone_MoveJointMode.cpp \
    bone/bone_Focuser.cpp \
    bone/bone_InfluenceMode.cpp \
    pose/pose_KeyOwner.cpp \
    pose/pose_TransBoneMode.cpp \
    bone/bone_Notifier.cpp \
    ffd/ffd_KeyOwner.cpp \
    ffd/ffd_MoveVertices.cpp \
    ffd/ffd_Task.cpp \
    bone/bone_GeoBuilder.cpp \
    Exporter.cpp \
    bone/bone_PaintInflMode.cpp \
    bone/bone_EraseInflMode.cpp \
    MeshEditor.cpp \
    mesh/mesh_KeyOwner.cpp \
    mesh/mesh_CreateMode.cpp \
    mesh/mesh_Focuser.cpp \
    mesh/mesh_Renderer.cpp \
    mesh/mesh_DeleteMode.cpp \
    mesh/mesh_Notifier.cpp \
    mesh/mesh_MeshAccessor.cpp \
    srt/srt_Symbol.cpp \
    srt/srt_KeyOwner.cpp \
    srt/srt_MoveMode.cpp \
    srt/srt_CentroidMode.cpp \
    srt/srt_CentroidMover.cpp \
    bone/bone_DeleteMode.cpp \
    bone/bone_DeleteBone.cpp \
    mesh/mesh_SplitMode.cpp \
    bone/bone_BindNodesMode.cpp \
    bone/bone_NodeSelector.cpp \
    ffd/ffd_TaskResource.cpp \
    DriverResources.cpp \
    Painter.cpp \
    KeyBinding.cpp \
    time/time_Focuser.cpp \
    time/time_Renderer.cpp \
    time/time_Scaler.cpp \
    TimeLineRow.cpp \
    time/time_Current.cpp \
    ffd/ffd_Target.cpp \
    ffd/ffd_DragMode.cpp \
    ffd/ffd_BrushMode.cpp \
    CmndName.cpp \
    VideoFormat.cpp \
    PoseParam.cpp \
    pose/pose_DrawBoneMode.cpp \
    pose/pose_ErasePoseMode.cpp \
    pose/pose_RotateBones.cpp \
    pose/pose_RigidBone.cpp \
    pose/pose_BoneDynamics.cpp

HEADERS += \
    Driver.h \
    ToolType.h \
    TimeLineEditor.h \
    TimeLineUtil.h \
    SRTEditor.h \
    ImageFileLoader.h \
    ScopedModifier.h \
    FFDEditor.h \
    FFDParam.h \
    ProjectLoader.h \
    ProjectSaver.h \
    System.h \
    TimeLineRow.h \
    BoneEditor.h \
    PoseEditor.h \
    bone/bone_IMode.h \
    BoneEditMode.h \
    bone/bone_CreateMode.h \
    bone/bone_Renderer.h \
    bone/bone_KeyOwner.h \
    bone/bone_MoveJointMode.h \
    bone/bone_Focuser.h \
    bone/bone_MoveBone.h \
    BoneParam.h \
    bone/bone_InfluenceMode.h \
    bone/bone_AssignInfluence.h \
    pose/pose_KeyOwner.h \
    pose/pose_TransBoneMode.h \
    pose/pose_RotateBone.h \
    bone/bone_PushNewPoses.h \
    bone/bone_PushNewTopPoses.h \
    bone/bone_Notifier.h \
    IEditor.h \
    ffd/ffd_KeyOwner.h \
    ffd/ffd_MoveVertices.h \
    ffd/ffd_Task.h \
    bone/bone_GeoBuilder.h \
    bone/bone_Target.h \
    pose/pose_Target.h \
    Exporter.h \
    bone/bone_PaintInflMode.h \
    bone/bone_EraseInflMode.h \
    MeshEditor.h \
    mesh/mesh_KeyOwner.h \
    mesh/mesh_IMode.h \
    mesh/mesh_Target.h \
    MeshParam.h \
    mesh/mesh_CreateMode.h \
    mesh/mesh_Focuser.h \
    mesh/mesh_Renderer.h \
    mesh/mesh_DeleteMode.h \
    mesh/mesh_Notifier.h \
    mesh/mesh_MeshAccessor.h \
    mesh/mesh_VtxMover.h \
    srt/srt_Symbol.h \
    srt/srt_FocusType.h \
    srt/srt_KeyOwner.h \
    srt/srt_IMode.h \
    SRTParam.h \
    srt/srt_MoveMode.h \
    srt/srt_CentroidMode.h \
    srt/srt_CentroidMover.h \
    bone/bone_DeleteMode.h \
    bone/bone_DeleteBone.h \
    mesh/mesh_SplitMode.h \
    bone/bone_BindNodesMode.h \
    GraphicStyle.h \
    bone/bone_NodeSelector.h \
    ffd/ffd_TaskResource.h \
    DriverResources.h \
    Painter.h \
    KeyBinding.h \
    time/time_Focuser.h \
    time/time_Renderer.h \
    time/time_Scaler.h \
    time/time_Current.h \
    ffd/ffd_Target.h \
    ffd/ffd_IMode.h \
    ffd/ffd_DragMode.h \
    ffd/ffd_BrushMode.h \
    CmndName.h \
    UILogger.h \
    UILogType.h \
    UILog.h \
    VideoFormat.h \
    PoseParam.h \
    PoseEditMode.h \
    pose/pose_DrawBoneMode.h \
    pose/pose_ErasePoseMode.h \
    pose/pose_IMode.h \
    pose/pose_RotateBones.h \
    pose/pose_RigidBone.h \
    pose/pose_BoneDynamics.h
