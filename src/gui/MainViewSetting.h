#ifndef GUI_MAINVIEWSETTING
#define GUI_MAINVIEWSETTING

namespace gui {

class MainViewSetting
{
public:
    MainViewSetting()
        : showLayerMesh()
        , cutImagesByTheFrame()
        , rotateViewRad()
    {
    }

    bool showLayerMesh;
    bool cutImagesByTheFrame;
    float rotateViewRad;
};

} // namespace gui

#endif // GUI_MAINVIEWSETTING

