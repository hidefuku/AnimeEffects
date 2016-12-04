#include "ctrl/TimeLineRow.h"

namespace ctrl
{

TimeLineRow::TimeLineRow()
    : node(nullptr)
    , rect()
    , closedFolder()
    , selecting()
{
}

TimeLineRow::TimeLineRow(
        core::ObjectNode* aNode, const QRect& aRect,
        bool aClosedFolder, bool aSelecting)
    : node(aNode)
    , rect(aRect)
    , closedFolder(aClosedFolder)
    , selecting(aSelecting)
{
}

float TimeLineRow::keyHeight(int aIndex, int aValidCount) const
{
    return aValidCount <= 1 ?
               rect.top() + (aIndex + 1) * (0.5f * rect.height()) :
               rect.top() + 0.5f * kIncrease + aIndex * (float)rect.height() / aValidCount;
}

int TimeLineRow::calculateHeight(int aValidCount)
{
    return aValidCount <= 1 ? kHeight : kIncrease * aValidCount;
}

} // namespace ctrl
