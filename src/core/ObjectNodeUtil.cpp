#include "core/ObjectNodeUtil.h"
#include "core/ObjectNodeBox.h"
#include "core/TimeKeyExpans.h"
#include "core/Project.h"
namespace
{

QRectF fGetContainedRect(const QRectF& aLhs, const QRectF& aRhs)
{
    QRectF rect = aLhs;
    rect.setLeft(std::min(rect.left(), aRhs.left()));
    rect.setTop(std::min(rect.top(), aRhs.top()));
    rect.setRight(std::max(rect.right(), aRhs.right()));
    rect.setBottom(std::max(rect.bottom(), aRhs.bottom()));
    return rect;
}

bool fCompareRenderDepth(core::Renderer* a, core::Renderer* b)
{
    return a->renderDepth() < b->renderDepth();
}

void fPushRenderClippeeRecursive(core::ObjectNode& aNode, std::vector<core::Renderer*>& aDest)
{
    aDest.push_back(aNode.renderer());

    for (auto child : aNode.children())
    {
        XC_PTR_ASSERT(child);
        if (child->isVisible() && child->renderer() && !child->renderer()->isClipped())
        {
            fPushRenderClippeeRecursive(*child, aDest);
        }
    }
}
}

namespace core
{

namespace ObjectNodeUtil
{

//-------------------------------------------------------------------------------------------------
float getGlobalDepth(ObjectNode& aNode)
{
    ObjectNode* node = &aNode;
    float gdepth = 0.0f;

    while (node)
    {
        gdepth += node->depth();
        node = node->parent();
    }
    return gdepth;
}

QVector2D getCenterOffset(const ObjectNode& aNode)
{
    return aNode.initialCenter() - QVector2D(aNode.initialRect().topLeft());
}

QVector2D getCenterOffset(const SRTExpans& aExpans)
{
    return aExpans.initialCenter() - QVector2D(aExpans.initialRect().topLeft());
}

QVector3D getCenterOffset3D(const ObjectNode& aNode)
{
    return QVector3D(getCenterOffset(aNode));
}

QVector3D getCenterOffset3D(const SRTExpans& aExpans)
{
    return QVector3D(getCenterOffset(aExpans));
}

void buildBoundingBox(ObjectNode& aNode)
{
    ObjectNodeBox* box = aNode.boxCache();
    if (!box || !box->isPack) return;

    box->rect = QRectF();

    for (ObjectNode* child : aNode.children())
    {
        if (!child) continue;
        buildBoundingBox(*child);

        ObjectNodeBox* childBox = child->boxCache();
        if (!childBox) continue;

        if (childBox->rect.isValid())
        {
            if (box->rect.isValid())
            {
                box->rect = fGetContainedRect(box->rect, childBox->rect);
            }
            else
            {
                box->rect = childBox->rect;
            }
        }
    }

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

void collectRenderClippees(ObjectNode& aNode, std::vector<Renderer*>& aDest)
{
    aDest.clear();

    auto p = aNode.prevSib();

    while (p && p->isVisible() && p->renderer() && p->renderer()->isClipped())
    {
        fPushRenderClippeeRecursive(*p, aDest);
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
