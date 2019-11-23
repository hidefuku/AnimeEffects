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

        /*
        if(timeScaleFormat == core::TimeFormatType_Relative_FPS) {
            QString number;
            frameNumber = number.sprintf("%.1f", static_cast<double>(currentFrame) / fps);
        }
        */

        /*
        QSettings settings;
        auto timeScaleFormatVar = settings.value("generalsettings/ui/timescaleformat");
        auto timeScaleFormat = timeScaleFormatVar.isValid() ? timeScaleFormatVar.toString() : QString();
        */

        //QString number;
        //number.sprintf("%.2f", static_cast<double>( timeInfo.frame.getDecimal() / timeInfo.fps ));
        //qDebug() << number;

        this->setText(frameNumber.rightJustified(frameMaxNumber.length()+1, ' ')+" / "+frameMaxNumber+" @"+QString::number(fps)+" "+tr("fps"));
    }
}

} // namespace gui
