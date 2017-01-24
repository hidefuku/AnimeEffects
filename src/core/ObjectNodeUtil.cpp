#include "core/ObjectNodeUtil.h"
#include "core/TimeKeyExpans.h"
#include "core/Project.h"
namespace
{

#if 0
QRectF fGetContainedRect(const QRectF& aLhs, const QRectF& aRhs)
{
    QRectF rect = aLhs;
    rect.setLeft(std::min(rect.left(), aRhs.left()));
    rect.setTop(std::min(rect.top(), aRhs.top()));
    rect.setRight(std::max(rect.right(), aRhs.right()));
    rect.setBottom(std::max(rect.bottom(), aRhs.bottom()));
    return rect;
}
#endif

bool fCompareRenderDepth(core::Renderer::SortUnit a, core::Renderer::SortUnit b)
{
    return a.depth < b.depth;
}

void fPushRenderClippeeRecursive(
        core::ObjectNode& aNode,
        std::vector<core::Renderer::SortUnit>& aDest,
        const core::TimeCacheAccessor& aAccessor)
{
    core::Renderer::SortUnit unit;
    unit.renderer = aNode.renderer();
    unit.depth = aAccessor.get(aNode).worldDepth();
    aDest.push_back(unit);

    for (auto child : aNode.children())
    {
        XC_PTR_ASSERT(child);
        if (child->isVisible() && child->renderer() && !child->renderer()->isClipped())
        {
            fPushRenderClippeeRecursive(*child, aDest, aAccessor);
        }
    }
}
}

namespace core
{

namespace ObjectNodeUtil
{

//-------------------------------------------------------------------------------------------------
float getInitialWorldDepth(ObjectNode& aNode)
{
    ObjectNode* node = &aNode;
    float gdepth = 0.0f;

    while (node)
    {
        gdepth += node->initialDepth();
        node = node->parent();
    }
    return gdepth;
}

bool thereAreSomeKeysExceedingFrame(const ObjectNode* aRootNode, int aMaxFrame)
{
    ObjectNode::ConstIterator nodeItr(aRootNode);
    while (nodeItr.hasNext())
    {
        const ObjectNode* node = nodeItr.next();
        XC_PTR_ASSERT(node);
        if (!node->timeLine()) continue;

        for (int i = 0; i < TimeKeyType_TERM; ++i)
        {
            auto& map = node->timeLine()->map((TimeKeyType)i);
            for (auto keyItr = map.begin(); keyItr != map.end(); ++keyItr)
            {
                if (aMaxFrame < keyItr.key())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void collectRenderClippees(ObjectNode& aNode,
                           std::vector<Renderer::SortUnit>& aDest,
                           const TimeCacheAccessor& aAccessor)
{
    aDest.clear();

    auto p = aNode.prevSib();

    while (p && p->isVisible() && p->renderer() && p->renderer()->isClipped())
    {
        fPushRenderClippeeRecursive(*p, aDest, aAccessor);
        p = p->prevSib();
    }
    if (!aDest.empty())
    {
        std::stable_sort(aDest.begin(), aDest.end(), fCompareRenderDepth);
    }
}

//-------------------------------------------------------------------------------------------------
AttributeNotifier::AttributeNotifier(Project& aProject, ObjectNode& aTarget)
    : mProject(aProject)
    , mTarget(aTarget)
{
}

void AttributeNotifier::onExecuted()
{
    mProject.onNodeAttributeModified(mTarget, false);
}

void AttributeNotifier::onUndone()
{
    mProject.onNodeAttributeModified(mTarget, true);
}

void AttributeNotifier::onRedone()
{
    mProject.onNodeAttributeModified(mTarget, false);
}


} // namespace ObjectNodeUtil

} // namespace core
