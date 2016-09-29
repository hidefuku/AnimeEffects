#include "core/MeshTransformerResource.h"
#include "ctrl/DriverResources.h"
#include "ctrl/ffd/ffd_TaskResource.h"

namespace ctrl
{

DriverResources::DriverResources()
    : mMeshTransformerResource()
    , mFFDTaskResource()
{
}

DriverResources::~DriverResources()
{
}

void DriverResources::grabMeshTransformerResoure(core::MeshTransformerResource* aResource)
{
    mMeshTransformerResource.reset(aResource);
}

core::MeshTransformerResource* DriverResources::meshTransformerResource() const
{
    return mMeshTransformerResource.data();
}

void DriverResources::grabFFDTaskResource(ffd::TaskResource* aResource)
{
    mFFDTaskResource.reset(aResource);
}

ffd::TaskResource* DriverResources::ffdTaskResource() const
{
    return mFFDTaskResource.data();
}

} // namespace ctrl
