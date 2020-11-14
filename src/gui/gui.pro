include(../common.pri)

TARGET      = AnimeEffects
TEMPLATE    = app
DESTDIR     = ..

CONFIG      += static
INCLUDES    += $$PWD

OBJECTS_DIR = .obj
MOC_DIR     = .moc
RCC_DIR     = .rcc

win32:RC_ICONS = ../AnimeEffects.ico
macx:ICON = ../AnimeEffects.icns

msvc:LIBS            += ../util/util.lib ../thr/thr.lib ../cmnd/cmnd.lib ../gl/gl.lib ../img/img.lib ../core/core.lib ../ctrl/ctrl.lib
msvc:PRE_TARGETDEPS  += ../util/util.lib ../thr/thr.lib ../cmnd/cmnd.lib ../gl/gl.lib ../img/img.lib ../core/core.lib ../ctrl/ctrl.lib

mingw:LIBS            += \
    -L"$$OUT_PWD/../ctrl/" -lctrl \
    -L"$$OUT_PWD/../core/" -lcore \
    -L"$$OUT_PWD/../img/"  -limg \
    -L"$$OUT_PWD/../gl/"   -lgl \
    -L"$$OUT_PWD/../cmnd/" -lcmnd \
    -L"$$OUT_PWD/../thr/"  -lthr \
    -L"$$OUT_PWD/../util/" -lutil

mingw:PRE_TARGETDEPS  += \
    ../ctrl/libctrl.a \
    ../core/libcore.a \
    ../img/libimg.a \
    ../gl/libgl.a \
    ../cmnd/libcmnd.a \
    ../util/libutil.a

gcc:LIBS            += \
    -L"$$OUT_PWD/../ctrl/" -lctrl \
    -L"$$OUT_PWD/../core/" -lcore \
    -L"$$OUT_PWD/../img/"  -limg \
    -L"$$OUT_PWD/../gl/"   -lgl \
    -L"$$OUT_PWD/../cmnd/" -lcmnd \
    -L"$$OUT_PWD/../thr/"  -lthr \
    -L"$$OUT_PWD/../util/" -lutil

gcc:PRE_TARGETDEPS  += \
    ../ctrl/libctrl.a \
    ../core/libcore.a \
    ../img/libimg.a \
    ../gl/libgl.a \
    ../cmnd/libcmnd.a \
    ../util/libutil.a

INCLUDEPATH += ..
DEPENDPATH  += ..

SOURCES += \
    Main.cpp \
    MainDisplayWidget.cpp \
    MainWindow.cpp \
    ObjectTreeWidget.cpp \
    PlayBackWidget.cpp \
    PropertyWidget.cpp \
    TimeLineEditorWidget.cpp \
    TimeLineInfoWidget.cpp \
    TimeLineWidget.cpp \
    ToolWidget.cpp \
    TargetWidget.cpp \
    prop/prop_Panel.cpp \
    prop/prop_KeyGroup.cpp \
    prop/prop_Items.cpp \
    prop/prop_Backboard.cpp \
    obj/obj_MoveItem.cpp \
    obj/obj_InsertItem.cpp \
    obj/obj_Item.cpp \
    obj/obj_RemoveItem.cpp \
    obj/obj_Notifiers.cpp \
    obj/obj_Util.cpp \
    theme/Theme.cpp \
    theme/TimeLine.cpp \
    tool/tool_ModePanel.cpp \
    tool/tool_FFDPanel.cpp \
    DriverHolder.cpp \
    tool/tool_BonePanel.cpp \
    tool/tool_ItemTable.cpp \
    tool/tool_Items.cpp \
    ResourceDialog.cpp \
    res/res_Item.cpp \
    ExportDialog.cpp \
    prop/prop_AttrGroup.cpp \
    prop/prop_ProjectPanel.cpp \
    NewProjectDialog.cpp \
    tool/tool_MeshPanel.cpp \
    ViaPoint.cpp \
    res/res_Notifier.cpp \
    MainMenuBar.cpp \
    tool/tool_SRTPanel.cpp \
    res/res_ResourceUpdater.cpp \
    ProjectHook.cpp \
    ProjectTabBar.cpp \
    EasyDialog.cpp \
    menu/menu_ProgressReporter.cpp \
    tool/tool_ViewPanel.cpp \
    tool/tool_FlowLayout.cpp \
    GUIResources.cpp \
    prop/prop_KeyKnocker.cpp \
    prop/prop_KeyAccessor.cpp \
    KeyBindingDialog.cpp \
    KeyCommandMap.cpp \
    KeyCommandInvoker.cpp \
    CanvasMover.cpp \
    prop/prop_ConstantPanel.cpp \
    prop/prop_DefaultKeyPanel.cpp \
    prop/prop_CurrentKeyPanel.cpp \
    MSVCMemoryLeakDebugger.cpp \
    MSVCBackTracer.cpp \
    LocaleDecider.cpp \
    GeneralSettingDialog.cpp \
    ResourceTreeWidget.cpp \
    tool/tool_PosePanel.cpp \
    MouseSettingDialog.cpp \
    MouseSetting.cpp

HEADERS += \
    MainDisplayMode.h \
    MainDisplayWidget.h \
    MainWindow.h \
    ObjectTreeWidget.h \
    PlayBackWidget.h \
    PropertyWidget.h \
    TimeLineEditorWidget.h \
    TimeLineInfoWidget.h \
    TimeLineWidget.h \
    ToolWidget.h \
    TargetWidget.h \
    prop/prop_Panel.h \
    prop/prop_KeyGroup.h \
    prop/prop_Items.h \
    prop/prop_ItemBase.h \
    prop/prop_Backboard.h \
    obj/obj_MoveItem.h \
    obj/obj_InsertItem.h \
    obj/obj_Item.h \
    obj/obj_RemoveItem.h \
    obj/obj_Notifiers.h \
    obj/obj_Util.h \
    theme/Theme.h \
    theme/TimeLine.h \
    tool/tool_ModePanel.h \
    tool/tool_FFDPanel.h \
    DriverHolder.h \
    tool/tool_BonePanel.h \
    tool/tool_ItemTable.h \
    tool/tool_Items.h \
    ResourceDialog.h \
    res/res_Item.h \
    ExportDialog.h \
    prop/prop_AttrGroup.h \
    prop/prop_ProjectPanel.h \
    NewProjectDialog.h \
    tool/tool_MeshPanel.h \
    ViaPoint.h \
    res/res_Notifier.h \
    res/res_ImageSetter.h \
    MainMenuBar.h \
    tool/tool_SRTPanel.h \
    res/res_ResourceUpdater.h \
    ProjectHook.h \
    ProjectTabBar.h \
    EasyDialog.h \
    menu/menu_ProgressReporter.h \
    tool/tool_ViewPanel.h \
    tool/tool_FlowLayout.h \
    MainViewSetting.h \
    MainDisplayStyle.h \
    GUIResources.h \
    prop/prop_KeyKnocker.h \
    prop/prop_KeyAccessor.h \
    KeyBindingDialog.h \
    KeyCommandMap.h \
    KeyCommandInvoker.h \
    CanvasMover.h \
    prop/prop_ConstantPanel.h \
    prop/prop_DefaultKeyPanel.h \
    prop/prop_CurrentKeyPanel.h \
    MSVCMemoryLeakDebugger.h \
    MSVCBackTracer.h \
    LocaleDecider.h \
    GeneralSettingDialog.h \
    ResourceTreeWidget.h \
    LocaleParam.h \
    tool/tool_PosePanel.h \
    MouseSettingDialog.h \
    MouseSetting.h

FORMS +=

