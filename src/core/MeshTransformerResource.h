#ifndef CORE_MESHTRANSFORMERRESOURCE_H
#define CORE_MESHTRANSFORMERRESOURCE_H

#include "gl/EasyShaderProgram.h"

namespace core
{

class MeshTransformerResource
{
public:
    MeshTransformerResource();
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

} // namespace core

#endif // CORE_MESHTRANSFORMERRESOURCE_H
