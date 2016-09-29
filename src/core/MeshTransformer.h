#ifndef CORE_MESHTRANSFORMER_H
#define CORE_MESHTRANSFORMER_H

#include <QScopedPointer>
#include "util/NonCopyable.h"
#include "util/ArrayBlock.h"
#include "gl/Vector3.h"
#include "gl/BufferObject.h"
#include "core/TimeKeyExpans.h"
#include "core/LayerMesh.h"
namespace core { class MeshTransformerResource; }

namespace core
{

class MeshTransformer : private util::NonCopyable
{
public:
    MeshTransformer(const QString& aShaderPath);
    MeshTransformer(MeshTransformerResource& aResource);
    ~MeshTransformer();

    void callGL(const TimeKeyExpans& aExpans, LayerMesh::MeshBuffer& aMeshBuffer,
                util::ArrayBlock<const gl::Vector3> aPositions,
                bool aNonPosed = false, bool aUseInfluence = true);

    gl::BufferObject& positions() { return *mOutPositions; }
    const gl::BufferObject& positions() const { return *mOutPositions; }

    gl::BufferObject& xArrows() { return *mOutXArrows; }
    const gl::BufferObject& xArrows() const { return *mOutXArrows; }

    gl::BufferObject& yArrows() { return *mOutYArrows; }
    const gl::BufferObject& yArrows() const { return *mOutYArrows; }

private:
    MeshTransformerResource& mResource;
    bool mResourceOwns;
    gl::BufferObject* mOutPositions;
    gl::BufferObject* mOutXArrows;
    gl::BufferObject* mOutYArrows;
};

} // namespace core

#endif // CORE_MESHTRANSFORMER_H
