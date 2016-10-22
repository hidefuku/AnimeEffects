#ifndef CORE_SHADERHOLDER_H
#define CORE_SHADERHOLDER_H

#include <QVector>
#include <QScopedPointer>
#include "gl/EasyShaderProgram.h"
#include "img/BlendMode.h"

namespace core
{

class ShaderHolder
{
public:
    ShaderHolder();
    ~ShaderHolder();

    gl::EasyShaderProgram& reserveShader(img::BlendMode aBlendMode, bool aIsClippee);
    void reserveShaders(img::BlendMode aBlendMode);
    gl::EasyShaderProgram& shader(img::BlendMode aBlendMode, bool aIsClippee);
    const gl::EasyShaderProgram& shader(img::BlendMode aBlendMode, bool aIsClippee) const;

    gl::EasyShaderProgram& reserveGridShader();
    gl::EasyShaderProgram& gridShader();
    const gl::EasyShaderProgram& gridShader() const;

    gl::EasyShaderProgram& reserveClipperShader(bool aIsClippee);
    void reserveClipperShaders();
    gl::EasyShaderProgram& clipperShader(bool aIsClippee);
    const gl::EasyShaderProgram& clipperShader(bool aIsClippee) const;

private:
    QVector<gl::EasyShaderProgram*> mShaders;
    QVector<gl::EasyShaderProgram*> mGridShaders;
    QVector<gl::EasyShaderProgram*> mClipperShaders;
};

} // namespace core

#endif // CORE_SHADERHOLDER_H
