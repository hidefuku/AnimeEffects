#include "core/ResourceUpdatingWorkspace.h"

namespace core
{

ResourceUpdatingWorkspace::ResourceUpdatingWorkspace()
    : transUnits()
{
}


GridMesh::Transitions& ResourceUpdatingWorkspace::makeSureTransitions(
        const TimeKey* aParent, const GridMesh& aMesh)
{
    for (auto& unit : transUnits)
    {
        // found
        if (unit.parent == aParent)
        {
            unit.mesh = &aMesh;
            return unit.trans;
        }
    }
    // append
    transUnits.push_back(Unit());
    transUnits.back().parent = aParent;
    transUnits.back().mesh = &aMesh;
    return transUnits.back().trans;
}

const ResourceUpdatingWorkspace::Unit* ResourceUpdatingWorkspace::findUnit(const TimeKey* aParent) const
{
    for (auto& unit : transUnits)
    {
        // found
        if (unit.parent == aParent)
        {
            return &unit;
        }
    }
    return nullptr;
}

} // namespace core
