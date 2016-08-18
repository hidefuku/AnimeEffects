#ifndef CORE_MESHTRANSFORMER_H
#define CORE_MESHTRANSFORMER_H

#include "util/NonCopyable.h"
#include "util/ArrayBlock.h"
#include "gl/Vector3.h"
#include "gl/BufferObject.h"
#include "gl/EasyShaderProgram.h"
#include "core/TimeKeyExpans.h"
#include "core/LayerMesh.h"

namespace core
{

class MeshTransformer : private util::NonCopyable
{
public:
    class Resource
    {
    public:
        Resource();
        void setup(const QString& aShaderPath);
        gl::EasyShaderProgram& program(bool aUseSkinning, bool aUseDualQuaternion);
        const gl::EasyShaderProgram& program(bool aUseSkinning, bool aUseDualQuaternion) const;

    private:
        void loadFile(const QString& aPath, QString& aDstCode);
        void buildShader(
                gl::EasyShaderProgram& aProgram, const QString& aCode,
                bool aUseSkinning, bool aUseDualQuaternion);

        gl::EasyShaderProgram mProgram[3];
    };

    MeshTransformer(const QString& aShaderPath);

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
    Resource mResource;
    gl::BufferObject* mOutPositions;
    gl::BufferObject* mOutXArrows;
    gl::BufferObject* mOutYArrows;
};

} // namespace core

#endif // CORE_MESHTRANSFORMER_H
