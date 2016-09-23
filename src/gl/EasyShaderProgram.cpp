#include "gl/EasyShaderProgram.h"
#include "gl/Global.h"

namespace gl
{

EasyShaderProgram::EasyShaderProgram()
    : mImpl()
    , mVBOs()
{
    mAttributeLocations.reserve(32);
}

EasyShaderProgram::~EasyShaderProgram()
{
    qDeleteAll(mVBOs.begin(), mVBOs.end());
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

    // unbind vbo
    Global::functions().glBindBuffer(GL_ARRAY_BUFFER, 0);

    GL_CHECK_ERROR();
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

void EasyShaderProgram::makeSureVBO(
        int aLocation, GLsizeiptr aTypeSize, const void* aArray, int aCount)
{
    XC_ASSERT(aLocation >= 0);
    BufferObject*& vbo = mVBOs[aLocation];
    if (!vbo) vbo = new BufferObject(GL_ARRAY_BUFFER);
    vbo->resetRawData(aTypeSize, aArray, aCount, GL_DYNAMIC_DRAW);
    vbo->bind();
}

void EasyShaderProgram::setRawAttributeArray(
        const char* aName, GLenum aType, GLsizeiptr aTypeSize,
        const void* aArray, int aCount, int aTuple, int aStride)
{
    const int location = mImpl.attributeLocation(aName);
    if (location < 0) return;
    makeSureVBO(location, aTypeSize, aArray, aTuple * aCount);
    mImpl.enableAttributeArray(location);
    mImpl.setAttributeArray(location, aType, nullptr, aTuple, aStride);
    mAttributeLocations.push_back(location);
}

void EasyShaderProgram::setRawAttributeArray(
        int aLocation, GLenum aType, GLsizeiptr aTypeSize,
        const void* aArray, int aCount, int aTuple, int aStride)
{
    if (aLocation < 0) return;
    makeSureVBO(aLocation, aTypeSize, aArray, aTuple * aCount);
    mImpl.enableAttributeArray(aLocation);
    mImpl.setAttributeArray(aLocation, aType, nullptr, aTuple, aStride);
    mAttributeLocations.push_back(aLocation);
}

void EasyShaderProgram::setRawAttributeIArray(
        const char* aName, GLenum aType, GLsizeiptr aTypeSize,
        const void* aArray, int aCount, int aTuple, int aStride)
{
    const int location = attributeLocation(aName);
    if (location < 0) return;
    makeSureVBO(location, aTypeSize, aArray, aTuple * aCount);
    mImpl.enableAttributeArray(location);
    gl::Global::functions().glVertexAttribIPointer(location, aTuple, aType, aStride, nullptr);
    mAttributeLocations.push_back(location);
}

void EasyShaderProgram::setRawAttributeIArray(
        int aLocation, GLenum aType, GLsizeiptr aTypeSize,
        const void* aArray, int aCount, int aTuple, int aStride)
{
    if (aLocation < 0) return;
    makeSureVBO(aLocation, aTypeSize, aArray, aTuple * aCount);
    mImpl.enableAttributeArray(aLocation);
    gl::Global::functions().glVertexAttribIPointer(aLocation, aTuple, aType, aStride, nullptr);
    mAttributeLocations.push_back(aLocation);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const GLfloat* aArray, int aCount)
{
    setRawAttributeArray(aName, GL_FLOAT, 4, aArray, aCount, 1);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector2* aArray, int aCount)
{
    setRawAttributeArray(aName, GL_FLOAT, 4, aArray, aCount, 2);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector3* aArray, int aCount)
{
    setRawAttributeArray(aName, GL_FLOAT, 4, aArray, aCount, 3);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector4* aArray, int aCount)
{
    setRawAttributeArray(aName, GL_FLOAT, 4, aArray, aCount, 4);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const GLubyte* aArray, int aCount)
{
    setRawAttributeArray(aName, GL_UNSIGNED_BYTE, 1, aArray, aCount, 1);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector2I* aArray, int aCount)
{
    setRawAttributeIArray(aName, GL_INT, 4, aArray, aCount, 2);
}

void EasyShaderProgram::setAttributeArray(const char* aName, const gl::Vector4I* aArray, int aCount)
{
    setRawAttributeIArray(aName, GL_INT, 4, aArray, aCount, 4);
}

void EasyShaderProgram::setAttributeArray(int aLocation, const GLfloat* aArray, int aCount)
{
    setRawAttributeArray(aLocation, GL_FLOAT, 4, aArray, aCount, 1);
}

void EasyShaderProgram::setAttributeArray(int aLocation, const GLubyte* aArray, int aCount)
{
    setRawAttributeArray(aLocation, GL_UNSIGNED_BYTE, 1, aArray, aCount, 1);
}

} // namespace gl
