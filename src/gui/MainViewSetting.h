#ifndef GUI_MAINVIEWSETTING
#define GUI_MAINVIEWSETTING

namespace gui {

class MainViewSetting
{
public:
    MainViewSetting()
        : showLayerMesh()
        , cutImagesByTheFrame()
        , rotateViewDeg()
    {
    }

    bool showLayerMesh;
    bool cutImagesByTheFrame;
    int rotateViewDeg;
};

} // namespace gui

#endif // GUI_MAINVIEWSETTING

