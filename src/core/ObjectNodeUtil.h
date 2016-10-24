#ifndef CORE_OBJECTNODEUTIL_H
#define CORE_OBJECTNODEUTIL_H

#include <vector>
#include <QRectF>
#include <QMatrix4x4>
#include "cmnd/Listener.h"
#include "core/ObjectNode.h"
#include "core/LayerSetNode.h"
#include "core/SRTExpans.h"

namespace core
{

namespace ObjectNodeUtil
{
float getGlobalDepth(ObjectNode& aNode);

bool thereAreSomeKeysExceedingFrame(const ObjectNode* aRootNode, int aMaxFrame);

void collectRenderClippees(ObjectNode& aNode, std::vector<Renderer*>& aDest);

void buildBoundingBox(ObjectNode& aNode);

class AttributeNotifier : public cmnd::Listener
{
    Project& mProject;
    ObjectNode& mTarget;
public:
    AttributeNotifier(Project& aProject, ObjectNode& aTarget);
    virtual void onExecuted();
    virtual void onUndone();
    virtual void onRedone();
};

}

} // namespace core

#endif // CORE_OBJECTNODEUTIL_H
