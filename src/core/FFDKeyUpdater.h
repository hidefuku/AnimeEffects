#ifndef CORE_FFDKEYUPDATER_H
#define CORE_FFDKEYUPDATER_H

#include "cmnd/Stable.h"
#include "core/ObjectNode.h"
#include "core/GridMesh.h"

namespace core
{

class FFDKeyUpdater
{
public:
    static cmnd::Stable* createResourceUpdater(
            ObjectNode& aNode,
            const GridMesh& aNewMesh,
            const GridMesh::Transitions& aTransitions);
};

} // namespace core

#endif // CORE_FFDKEYUPDATER_H
