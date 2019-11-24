#include "gui/InfoLabelWidget.h"

namespace gui
{

InfoLabelWidget::InfoLabelWidget(GUIResources& aResources, QWidget* aParent)
    : QLabel(aParent)
    , mResources(aResources)
    , mProject()
    , mIsFirstTime(true)
    , mSuspendCount(0)
{
    this->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
    this->setContentsMargins(2,0,2,2);
}

void InfoLabelWidget::setProject(core::Project* aProject)
{
    mProject = aProject;
    onUpdate();
}

void InfoLabelWidget::onUpdate()
{
    if(mProject != nullptr) {
        core::TimeInfo timeInfo = mProject->currentTimeInfo();

        const int frameMax = timeInfo.frameMax;
        const int fps = timeInfo.fps;
        const int currentFrame = timeInfo.frame.get();

        const core::TimeFormat timeFormat(util::Range(0,frameMax),fps);
        auto timeScaleFormatVar = mSettings.value("generalsettings/ui/timescaleformat");
        core::TimeFormatType timeScaleFormat = timeScaleFormatVar.isValid() ? static_cast<core::TimeFormatType>(timeScaleFormatVar.toInt()) : core::TimeFormatType::TimeFormatType_Frames_From0;

        QString frameNumber = timeFormat.frameToString(currentFrame, timeScaleFormat);
        QString frameMaxNumber = timeFormat.frameToString(frameMax, timeScaleFormat);

        this->setText(frameNumber.rightJustified(frameMaxNumber.length()+1, ' ')+" / "+frameMaxNumber+" @"+QString::number(fps)+" "+tr("fps"));
    }
}

} // namespace gui
