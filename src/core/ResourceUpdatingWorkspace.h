#ifndef CORE_RESOURCEUPDATINGWORKSPACE_H
#define CORE_RESOURCEUPDATINGWORKSPACE_H

#include <memory>
#include <QList>
#include "core/GridMesh.h"
namespace core { class TimeKey; }

namespace core
{

class ResourceUpdatingWorkspace
{
public:
    struct Unit
    {
        Unit() : parent(), mesh(), trans() {}
        const TimeKey* parent;
        const GridMesh* mesh;
        GridMesh::Transitions trans;
    };
    ResourceUpdatingWorkspace();

    // null parent means default parent
    GridMesh::Transitions& makeSureTransitions(
            const TimeKey* aParent, const GridMesh& aMesh);

    // null parent means default parent
    const Unit* findUnit(const TimeKey* aParent) const;

    QList<Unit> transUnits;
};

typedef std::shared_ptr<ResourceUpdatingWorkspace> ResourceUpdatingWorkspacePtr;

} // namespace core

#endif // CORE_RESOURCEUPDATINGWORKSPACE_H
