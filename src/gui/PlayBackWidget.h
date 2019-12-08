#ifndef GUI_PLAYBACKWIDGET_H
#define GUI_PLAYBACKWIDGET_H

#include <vector>
#include <functional>
#include <QWidget>
#include <QPushButton>
#include "gui/GUIResources.h"

namespace gui
{

class PlayBackWidget : public QWidget
{
    Q_OBJECT
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

    PlayBackWidget(GUIResources& aResources, QWidget* aParent);

    void setPushDelegate(const PushDelegate& aDelegate);
    int constantWidth() const;
    void pushPauseButton();

private:
    QPushButton* createButton(
            const QString& aName, bool aIsCheckable,
            int aColumn, const QString& aToolTip);
    GUIResources& mGUIResources;
    std::vector<QPushButton*> mButtons;
    PushDelegate mPushDelegate;
    void onThemeUpdated();
};

} // namespace gui

#endif // GUI_PLAYBACKWIDGET_H
