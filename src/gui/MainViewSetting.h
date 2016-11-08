#ifndef GUI_MAINVIEWSETTING
#define GUI_MAINVIEWSETTING

namespace gui {

class MainViewSetting
{
public:
    MainViewSetting()
        : showLayerMesh()
        , cutImagesByTheFrame()
        , rotateViewACW()
        , resetRotateView()
        , rotateViewCW()
    {
    }

    bool showLayerMesh;
    bool cutImagesByTheFrame;
    bool rotateViewACW;
    bool resetRotateView;
    bool rotateViewCW;
};

} // namespace gui

#endif // GUI_MAINVIEWSETTING

