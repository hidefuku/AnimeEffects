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
    if (aValidCount <= 1 || (node && node->isSlimmedDown()))
    {
        return rect.top() + 0.5f * rect.height();
    }
    else
    {
        return rect.top() + 0.5f * kIncrease + aIndex * (float)rect.height() / aValidCount;
    }
}

int TimeLineRow::calculateHeight(const core::ObjectNode& aNode)
{
    if (!aNode.timeLine()) return kHeight;

    auto validCount = aNode.timeLine()->validTypeCount();
    if (validCount <= 1 || aNode.isSlimmedDown())
    {
        return kHeight;
    }
    else
    {
        return kIncrease * validCount;
    }
}

} // namespace ctrl
