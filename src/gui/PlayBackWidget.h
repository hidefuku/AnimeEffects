#ifndef GUI_PLAYBACKWIDGET_H
#define GUI_PLAYBACKWIDGET_H

#include <vector>
#include <functional>
#include <QWidget>
#include <QPushButton>
#include "gui/GUIResourceSet.h"

namespace gui
{

class PlayBackWidget : public QWidget
{
public:
    enum PushType
    {
        PushType_Play,
        PushType_Pause,
        PushType_Step,
        PushType_StepBack,
        PushType_Rewind,
        PushType_Loop,
        PushType_NoLoop,
        PushType_Fast
    };
    typedef std::function<void(PushType)> PushDelegate;

    PlayBackWidget(GUIResourceSet& aResources, QWidget* aParent);

    void setPushDelegate(const PushDelegate& aDelegate);
    int constantWidth() const;
    void pushPauseButton();

private:
    QPushButton* createButton(
            const QString& aName, bool aIsCheckable,
            int aColumn, const QString& aToolTip);
    GUIResourceSet& mResources;
    std::vector<QPushButton*> mButtons;
    PushDelegate mPushDelegate;
};

} // namespace gui

#endif // GUI_PLAYBACKWIDGET_H
