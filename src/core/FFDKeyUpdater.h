#ifndef CORE_FFDKEYUPDATER_H
#define CORE_FFDKEYUPDATER_H

#include "cmnd/Stable.h"
#include "core/ObjectNode.h"
#include "core/GridMesh.h"
#include "core/ResourceUpdatingWorkspace.h"

namespace core
{

class FFDKeyUpdater
{
public:
    static cmnd::Stable* createResourceUpdater(
            ObjectNode& aNode, const ResourceUpdatingWorkspacePtr& aWorkspace);
};

} // namespace core

#endif // CORE_FFDKEYUPDATER_H
