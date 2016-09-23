#include "gl/VertexArrayObject.h"
#include "gl/Global.h"

namespace gl
{
VertexArrayObject::VertexArrayObject()
    : mId(0)
{
    gl::Global::functions().glGenVertexArrays(1, &mId);

}

VertexArrayObject::~VertexArrayObject()
{
    gl::Global::functions().glDeleteVertexArrays(1, &mId);
}

void VertexArrayObject::bind()
{
    gl::Global::functions().glBindVertexArray(mId);
}

void VertexArrayObject::release()
{
    gl::Global::functions().glBindVertexArray(0);
}


} // namespace gl
