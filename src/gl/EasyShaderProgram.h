#ifndef GL_EASYSHADERPROGRAM_H
#define GL_EASYSHADERPROGRAM_H

#include <QOpenGLShaderProgram>
#include "util/ArrayBlock.h"
#include "gl/ExtendShader.h"
#include "gl/BufferObject.h"
#include "gl/Vector2.h"
#include "gl/Vector3.h"
#include "gl/Vector4.h"
#include "gl/Vector2I.h"
#include "gl/Vector4I.h"

namespace gl
{

class EasyShaderProgram
{
public:
    EasyShaderProgram();

    bool setAllSource(const ExtendShader& aShader);
    bool setVertexSource(const ExtendShader& aShader);
    bool setVertexSource(const QString& aSource);
    bool setFragmentSource(const QString& aSource);

    bool link();
    QString log() const;

    GLuint id() const;
    void bind();
    void release();

    int attributeLocation(const char* aName) const;

    void setAttributeBuffer(
            const char* aName, BufferObject& aObj,
            GLenum aType, int aTuple, int aOffset = 0);

    void setRawAttributeArray(
            const char* aName, GLenum aType,
            const void* aArray, int aTuple, int aStride = 0);

    void setRawAttributeArray(
            int aLocation, GLenum aType,
            const void* aArray, int aTuple, int aStride = 0);

    void setAttributeArray(const char* aName, const GLfloat* aArray);
    void setAttributeArray(const char* aName, const gl::Vector2* aArray);
    void setAttributeArray(const char* aName, const gl::Vector3* aArray);
    void setAttributeArray(const char* aName, const gl::Vector4* aArray);
    void setAttributeArray(const char* aName, const GLubyte* aArray);
    void setAttributeArray(const char* aName, const gl::Vector2I* aArray);
    void setAttributeArray(const char* aName, const gl::Vector4I* aArray);

    void setAttributeArray(int aLocation, const GLfloat* aArray);
    void setAttributeArray(int aLocation, const GLubyte* aArray);

    template<typename tType>
    void setTupleAttributeArray(const char* aName, const tType* aArray, int aTuple)
    {
        const int location = mImpl.attributeLocation(aName);
        mImpl.enableAttributeArray(location);
        mImpl.setAttributeArray(location, aArray, aTuple);
        mAttributeLocations.push_back(location);
    }

    int uniformLocation(const char* aName) const;

    template<typename tType>
    void setUniformValue(const char* aName, const tType& aValue)
    {
        mImpl.setUniformValue(aName, aValue);
    }

    template<typename tType>
    void setUniformValueArray(const char* aName, const tType* aValue, int aCount)
    {
        mImpl.setUniformValueArray(aName, aValue, aCount);
    }

    template<typename tType>
    void setUniformValueArray(const char* aName, const util::ArrayBlock<tType>& aValue)
    {
        mImpl.setUniformValueArray(aName, aValue.array(), aValue.count());
    }

    template<typename tType>
    void setTupleUniformValueArray(const char* aName, const tType* aValue, int aCount, int aTuple)
    {
        mImpl.setUniformValueArray(aName, aValue, aCount, aTuple);
    }

private:
    QOpenGLShaderProgram mImpl;
    std::vector<int> mAttributeLocations;
};

} // namespace gl

#endif // GL_EASYSHADERPROGRAM_H
