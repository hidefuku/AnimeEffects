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
#include "ctrl/TimeLineRow.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/time/time_Current.h"
#include "ctrl/time/time_Scaler.h"
#include "ctrl/time/time_Focuser.h"

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
    void updateWheel(int aDelta, bool aInvertScaling);
    void updateKey();
    void updateProjectAttribute();

    void clearRows();
    void pushRow(core::ObjectNode* aNode, util::Range aWorldTB, bool aClosedFolder);
    void updateRowSelection(const core::ObjectNode* aRepresent);
    void render(QPainter& aPainter, const core::CameraInfo& aCamera,
                const QRect& aCullRect);

    core::Frame currentFrame() const;
    int maxFrame() const { return mTimeMax; }
    QSize modelSpaceSize() const;
    QPoint currentTimeCursorPos() const;
    bool checkContactWithKeyFocus(core::TimeLineEvent& aEvent, const QPoint& aPos);
    bool pasteCopiedKeys(core::TimeLineEvent& aEvent, const QPoint& aWorldPos);
    void deleteCheckedKeys(core::TimeLineEvent& aEvent);

private:
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
    void beginMoveKey(const time::Focuser::SingleFocus& aTarget);
    bool beginMoveKeys(const QPoint& aWorldPos);
    bool modifyMoveKeys(const QPoint& aWorldPos);

    util::LinkPointer<core::Project> mProject;
    QVector<TimeLineRow> mRows;
    const core::ObjectNode* mSelectingRow;
    int mTimeMax;
    State mState;
    time::Current mTimeCurrent;
    time::Scaler mTimeScale;
    time::Focuser mFocus;

    TimeLineUtil::MoveFrameOfKey* mMoveRef;
    int mMoveFrame;
    bool mOnUpdatingKey;
    bool mShowSelectionRange;
};

} // namespace core

#endif // CTRL_TIMELINEEDITOR_H
