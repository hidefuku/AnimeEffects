#ifndef GUI_MOUSESETTING_H
#define GUI_MOUSESETTING_H

namespace gui
{

class MouseSetting
{
public:
    MouseSetting();

    bool operator==(const MouseSetting& aRhs) const;
    bool operator!=(const MouseSetting& aRhs) const { return !(*this == aRhs); }

    void load();
    void save();
    bool invertMainViewScaling;
    bool invertTimeLineScaling;
};

} // namespace gui

#endif // GUI_MOUSESETTING_H
