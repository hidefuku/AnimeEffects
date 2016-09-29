include(../common.pri)

TARGET      = AnimeEffects
TEMPLATE    = app
DESTDIR     = ..

CONFIG      += static
INCLUDES    += $$PWD

OBJECTS_DIR = .obj
MOC_DIR     = .moc
RCC_DIR     = .rcc

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
    TimeLineWidget.cpp \
    ToolWidget.cpp \
    TargetWidget.cpp \
    prop/prop_ObjectPanel.cpp \
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
    tool/tool_ModePanel.cpp \
    tool/tool_FFDPanel.cpp \
    DriverHolder.cpp \
    TimeLineInnerWidget.cpp \
    tool/tool_BonePanel.cpp \
    tool/tool_ItemTable.cpp \
    tool/tool_Items.cpp \
    ResourceDialog.cpp \
    res/res_ResourceTree.cpp \
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
    GUIResources.cpp

HEADERS += \
    MainDisplayMode.h \
    MainDisplayWidget.h \
    MainWindow.h \
    ObjectTreeWidget.h \
    PlayBackWidget.h \
    PropertyWidget.h \
    TimeLineWidget.h \
    ToolWidget.h \
    TargetWidget.h \
    prop/prop_ObjectPanel.h \
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
    tool/tool_ModePanel.h \
    tool/tool_FFDPanel.h \
    DriverHolder.h \
    TimeLineInnerWidget.h \
    tool/tool_BonePanel.h \
    tool/tool_ItemTable.h \
    tool/tool_Items.h \
    ResourceDialog.h \
    res/res_ResourceTree.h \
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
    GUIResources.h

FORMS +=

