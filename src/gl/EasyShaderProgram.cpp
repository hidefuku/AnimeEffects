#include "gl/EasyShaderProgram.h"
#include "gl/Global.h"

namespace gl
{

EasyShaderProgram::EasyShaderProgram()
    : mImpl()
{
    mAttributeLocations.reserve(32);
}

bool EasyShaderProgram::setAllSource(const ExtendShader& aShader)
{
    bool result = true;
    result &= mImpl.addShaderFromSourceCode(QOpenGLShader::Vertex, aShader.vertexCode());
    result &= mImpl.addShaderFromSourceCode(QOpenGLShader::Fragment, aShader.fragmentCode());
    return result;
}

bool EasyShaderProgram::setVertexSource(const ExtendShader& aShader)
{
    return mImpl.addShaderFromSourceCode(QOpenGLShader::Vertex, aShader.vertexCode());
}

bool EasyShaderProgram::setVertexSource(const QString& aSource)
{
    return mImpl.addShaderFromSourceCode(QOpenGLShader::Vertex, aSource);
}

bool EasyShaderProgram::setFragmentSource(const QString& aSource)
{
    return mImpl.addShaderFromSourceCode(QOpenGLShader::Fragment, aSource);
}

bool EasyShaderProgram::link()
{
    return mImpl.link();
}

QString EasyShaderProgram::log() const
{
    return mImpl.log();
}

GLuint EasyShaderProgram::id() const
{
    return mImpl.programId();
}

void EasyShaderProgram::bind()
{
    mImpl.bind();
    mAttributeLocations.clear();
}

void EasyShaderProgram::release()
{
    mImpl.release();

    for (std::vector<int>::iterator itr = mAttributeLocations.begin(); itr != mAttributeLocations.end(); ++itr)
    {
        mImpl.disableAttributeArray(*itr);
    }
}

int EasyShaderProgram::attributeLocation(const char *aName) const
{
    return mImpl.attributeLocation(aName);
}

int EasyShaderProgram::uniformLocation(const char *aName) const
{
    return mImpl.uniformLocation(aName);
}

void EasyShaderProgram::setAttributeBuffer(
        const char* aName, BufferObject& aObj, GLenum aType, int aTuple, int aOffset)
{
    const int location = mImpl.attributeLocation(aName);
    if (location != -1)
    {
        mImpl.enableAttributeArray(location);
        aObj.bind();
        mImpl.setAttributeBuffer(location, aType, aOffset, aTuple);
        aObj.release();
        mAttributeLocations.push_back(location);
    }
}

void EasyShaderProgram::setRawAttributeArray(
        const char* aName, GLenum aType, const void* aArray, int aTuple, int aStride)
{
    const int location = mImpl.attributeLocation(aName);
    if (location != -1)
    {
        mImpl.enableAttributeArray(location);
        mImpl.setAttributeArray(location, aType, aArray, aTuple, aStride);
        mAttributeLocations.push_back(location);
    }
}

void EasyShaderProgram::setRawAttributeArray(
        int aLocation, GLenum aType, const void* aArray, int aTuple, int aStride)
{
    if (aLocation != -1)
    {
        mImpl.enableAttributeArray(aLocation);
        mImpl.setAttributeArray(aLocation, aType, aArray, aTuple, aStride);
        mAttributeLocations.push_back(aLocation);
    }
}

void EasyShaderProgram::setAttributeArray(const char* aName, const GLfloat* aArray)
{
    setTupleAttributeArray<GLfloat>(aName, aArray, 1);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector2* aArray)
{
    setTupleAttributeArray<GLfloat>(aName, (const GLfloat*)aArray, 2);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector3* aArray)
{
    setTupleAttributeArray<GLfloat>(aName, (const GLfloat*)aArray, 3);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector4* aArray)
{
    setTupleAttributeArray<GLfloat>(aName, (const GLfloat*)aArray, 4);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const GLubyte* aArray)
{
    setRawAttributeArray(aName, GL_UNSIGNED_BYTE, aArray, 1);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector2I* aArray)
{
    const int location = attributeLocation(aName);
    if (location != -1)
    {
        mImpl.enableAttributeArray(location);
        gl::Global::functions().glVertexAttribIPointer(location, 2, GL_INT, 0, aArray);
        mAttributeLocations.push_back(location);
    }
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector4I* aArray)
{
    const int location = attributeLocation(aName);
    if (location != -1)
    {
        mImpl.enableAttributeArray(location);
        gl::Global::functions().glVertexAttribIPointer(location, 4, GL_INT, 0, aArray);
        mAttributeLocations.push_back(location);
    }
}

void EasyShaderProgram::setAttributeArray(int aLocation, const GLfloat* aArray)
{
    setRawAttributeArray(aLocation, GL_FLOAT, aArray, 1);
}

void EasyShaderProgram::setAttributeArray(int aLocation, const GLubyte* aArray)
{
    setRawAttributeArray(aLocation, GL_UNSIGNED_BYTE, aArray, 1);
}

} // namespace gl
