#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

#include <QVector3D>
#include <QList>
#include "XC.h"
#include "img/BlendMode.h"
#include "core/RenderInfo.h"
#include "core/TimeCacheAccessor.h"

namespace core
{

class Renderer
{
public:
    virtual ~Renderer() {}

    virtual void prerender(const RenderInfo& aInfo,
                           const TimeCacheAccessor&) = 0;

    virtual void render(const RenderInfo& aInfo,
                        const TimeCacheAccessor&) = 0;

    virtual void renderClipper(const RenderInfo& aInfo,
                               const TimeCacheAccessor&,
                               uint8 aClipperId) = 0;

    virtual float renderDepth() const = 0;

    virtual void setClipped(bool aIsClipped) = 0;
    virtual bool isClipped() const = 0;

    virtual bool hasBlendMode() const { return false; }
    virtual img::BlendMode blendMode() const { return img::BlendMode_TERM; }
    virtual void setBlendMode(img::BlendMode) {}
};

} // namespace core

#endif // CORE_RENDERER_H
