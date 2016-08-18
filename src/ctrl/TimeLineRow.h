#ifndef CTRL_TIMELINEROW
#define CTRL_TIMELINEROW

#include <QRect>
#include "core/ObjectNode.h"

namespace ctrl
{

struct TimeLineRow
{
    enum { kHeight = 22, kIncrease = 18 };

    core::ObjectNode* node;
    QRect rect;
    bool closedFolder;
    bool selecting;

    float keyHeight(int aIndex, int aValidCount) const
    {
        if (aValidCount <= 1)
            return rect.top() + (aIndex + 1) * (0.5f * rect.height());
        else
            return rect.top() + 0.5f * kIncrease + aIndex * (float)rect.height() / aValidCount;
    }

    static int calculateHeight(int aValidCount)
    {
        return aValidCount <= 1 ? kHeight : kIncrease * aValidCount;
    }
};

} // namespace ctrl

#endif // CTRL_TIMELINEROW

