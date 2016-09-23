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
    ~EasyShaderProgram();

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
            const char* aName, GLenum aType, GLsizeiptr aTypeSize,
            const void* aArray, int aCount, int aTuple, int aStride = 0);

    void setRawAttributeArray(
            int aLocation, GLenum aType, GLsizeiptr aTypeSize,
            const void* aArray, int aCount, int aTuple, int aStride = 0);

    void setRawAttributeIArray(
            const char* aName, GLenum aType, GLsizeiptr aTypeSize,
            const void* aArray, int aCount, int aTuple, int aStride = 0);

    void setRawAttributeIArray(
            int aLocation, GLenum aType, GLsizeiptr aTypeSize,
            const void* aArray, int aCount, int aTuple, int aStride = 0);

    void setAttributeArray(const char* aName, const GLfloat* aArray, int aCount);
    void setAttributeArray(const char* aName, const gl::Vector2* aArray, int aCount);
    void setAttributeArray(const char* aName, const gl::Vector3* aArray, int aCount);
    void setAttributeArray(const char* aName, const gl::Vector4* aArray, int aCount);
    void setAttributeArray(const char* aName, const GLubyte* aArray, int aCount);
    void setAttributeArray(const char* aName, const gl::Vector2I* aArray, int aCount);
    void setAttributeArray(const char* aName, const gl::Vector4I* aArray, int aCount);
    void setAttributeArray(int aLocation, const GLfloat* aArray, int aCount);
    void setAttributeArray(int aLocation, const GLubyte* aArray, int aCount);

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
    void makeSureVBO(int aLocation, GLsizeiptr aTypeSize, const void* aArray, int aCount);

    QOpenGLShaderProgram mImpl;
    std::vector<int> mAttributeLocations;
    QMap<int, BufferObject*> mVBOs;
};

} // namespace gl

#endif // GL_EASYSHADERPROGRAM_H
