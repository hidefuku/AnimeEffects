#ifndef CTRL_TIMELINEEDITOR_H
#define CTRL_TIMELINEEDITOR_H

#include <QRect>
#include <QVector>
#include <QScopedPointer>
#include "util/Range.h"
#include "util/LinkPointer.h"
#include "util/PlacePointer.h"
#include "core/Project.h"
#include "core/AbstractCursor.h"
#include "core/ObjectNode.h"
#include "core/CameraInfo.h"
#include "core/TimeKeyPos.h"
#include "ctrl/TimeLineScale.h"
#include "ctrl/TimeLineRow.h"
#include "ctrl/TimeLineFocus.h"
#include "ctrl/TimeLineUtil.h"

namespace ctrl
{

//-------------------------------------------------------------------------------------------------
class TimeLineEditor
{
public:
    enum UpdateFlag
    {
        UpdateFlag_ModFrame = 1,
        UpdateFlag_ModView = 2,
        UpdateFlag_TERM
    };
    typedef unsigned int UpdateFlags;

    TimeLineEditor();

    void setProject(core::Project* aProject);
    void setFrame(core::Frame aFrame);

    UpdateFlags updateCursor(const core::AbstractCursor& aCursor);
    void updateWheel(int aDelta);
    void updateKey();
    void updateProjectAttribute();

    void clearRows();
    void pushRow(core::ObjectNode* aNode, util::Range aWorldTB, bool aClosedFolder);
    void updateRowSelection(const core::ObjectNode* aRepresent);
    void render(QPainter& aPainter, const core::CameraInfo& aCamera, const QRect& aCullRect);

    core::Frame currentFrame() const;
    int maxFrame() const { return mTimeMax; }
    QSize modelSpaceSize() const;
    QPoint currentTimeCursorPos() const;
    bool checkDeletableKeys(core::TimeLineEvent& aEvent, const QPoint& aPos);
    void deleteCheckedKeys(core::TimeLineEvent& aEvent);

private:

    class TimeCurrent
    {
    public:
        TimeCurrent();
        void setMaxFrame(int aMaxFrame);
        void setFrame(const TimeLineScale& aScale, core::Frame aFrame);
        void setHandlePos(const TimeLineScale& aScale, const QPoint& aPos);
        void update(const TimeLineScale& aScale);
        core::Frame frame() const { return mFrame; }
        const QPoint& handlePos() const { return mPos; }
        int handleRange() const { return 5; }
    private:
        int mMaxFrame;
        core::Frame mFrame;
        QPoint mPos;
    };

    enum State
    {
        State_Standby,
        State_MoveCurrent,
        State_MoveKeys,
        State_EncloseKeys,
        State_TERM
    };

    void setMaxFrame(int aValue);
    void clearState();
    void beginMoveKey(const TimeLineFocus::SingleFocus& aTarget);
    bool beginMoveKeys(const QPoint& aWorldPos);
    bool modifyMoveKeys(const QPoint& aWorldPos);

    util::LinkPointer<core::Project> mProject;
    QVector<TimeLineRow> mRows;
    const core::ObjectNode* mSelectingRow;
    int mTimeMax;
    State mState;
    TimeCurrent mTimeCurrent;
    TimeLineScale mTimeScale;
    TimeLineFocus mFocus;

    TimeLineUtil::MoveFrameOfKey* mMoveRef;
    int mMoveFrame;
    bool mOnUpdatingKey;
    bool mShowSelectionRange;
};

} // namespace core

#endif // CTRL_TIMELINEEDITOR_H
