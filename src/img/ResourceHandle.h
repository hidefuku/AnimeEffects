#ifndef IMG_RESOURCEHANDLE_H
#define IMG_RESOURCEHANDLE_H

#include <memory>
#include "img/ResourceData.h"

namespace img
{

typedef std::shared_ptr<ResourceData> ResourceHandle;

} // namespace img

#endif // IMG_RESOURCEHANDLE_H
