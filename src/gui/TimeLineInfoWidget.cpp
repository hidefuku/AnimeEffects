#include "gui/TimeLineInfoWidget.h"

namespace gui
{

TimeLineInfoWidget::TimeLineInfoWidget(GUIResources& aResources, QWidget* aParent)
    : QLabel(aParent)
    , mResources(aResources)
    , mProject()
    , mIsFirstTime(true)
    , mSuspendCount(0)
{
    this->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
    this->setContentsMargins(2,0,2,2);
}

void TimeLineInfoWidget::setProject(core::Project* aProject)
{
    mProject = aProject;
    onUpdate();
}

void TimeLineInfoWidget::onUpdate()
{
    if(mProject != nullptr) {
        core::TimeInfo timeInfo = mProject->currentTimeInfo();

        const int frameMax = timeInfo.frameMax;
        const int fps = timeInfo.fps;
        const int currentFrame = timeInfo.frame.get();

        const core::TimeFormat timeFormat(util::Range(0,frameMax),fps);
        auto timeFormatVar = mSettings.value("generalsettings/ui/timeformat");
        core::TimeFormatType formatType = timeFormatVar.isValid() ? static_cast<core::TimeFormatType>(timeFormatVar.toInt()) : core::TimeFormatType::TimeFormatType_Frames_From0;

        QString frameNumber = timeFormat.frameToString(currentFrame, formatType);
        QString frameMaxNumber = timeFormat.frameToString(frameMax, formatType);

        this->setText(frameNumber.rightJustified(frameMaxNumber.length()+1, ' ')+" / "+frameMaxNumber+" @"+QString::number(fps)+" "+tr("fps"));
    }
}

} // namespace gui
