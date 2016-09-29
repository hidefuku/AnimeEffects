#ifndef CTRL_FFD_TASKRESOURCE_H
#define CTRL_FFD_TASKRESOURCE_H

#include "gl/EasyShaderProgram.h"

namespace ctrl {
namespace ffd {

class TaskResource
{
public:
    enum { kType = 2, kHardness = 3, kVariation = 6 };
    enum { kTypeDeformer = 0 };
    enum { kTypeEraser = 1 };

    TaskResource();

    void setup(const QString& aBrushPath,
               const QString& aEraserPath,
               const QString& aBlurPath);

    gl::EasyShaderProgram& program(int aType, int aHard);
    const gl::EasyShaderProgram& program(int aType, int aHard) const;

    gl::EasyShaderProgram& blurProgram() { return mBlurProgram; }
    const gl::EasyShaderProgram& blurProgram() const { return mBlurProgram; }

private:
    void loadFile(const QString& aPath, QString& aDstCode);
    void buildShader(
            gl::EasyShaderProgram& aProgram, const QString& aCode,
            int aType, int aHard);
    void buildBlurShader(
            gl::EasyShaderProgram& aProgram, const QString& aCode);

    gl::EasyShaderProgram mProgram[kVariation];
    gl::EasyShaderProgram mBlurProgram;
};

} // namespace ffd
} // namespace ctrl

#endif // CTRL_FFD_TASKRESOURCE_H
