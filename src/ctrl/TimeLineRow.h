#ifndef CTRL_TIMELINEROW_H
#define CTRL_TIMELINEROW_H

#include <QRect>
#include "core/ObjectNode.h"

namespace ctrl
{

class TimeLineRow
{
public:
    enum { kHeight = 22, kIncrease = 18 };

    core::ObjectNode* node;
    QRect rect;
    bool closedFolder;
    bool selecting;

    TimeLineRow();
    TimeLineRow(core::ObjectNode* aNode, const QRect& aRect,
                bool aClosedFolder, bool aSelecting);

    float keyHeight(int aIndex, int aValidCount) const;
    static int calculateHeight(int aValidCount);
};

} // namespace ctrl

#endif // CTRL_TIMELINEROW_H
