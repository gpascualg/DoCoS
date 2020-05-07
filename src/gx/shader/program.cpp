#include "gx/common.hpp"
#include "gx/shader/program.hpp"
#include "io/memmap.hpp"


program::program()
{}

GLuint program::attach(GLenum type, std::filesystem::path path)
{
    GLuint shader = compile(type, path);
    if (shader != 0)
    {
        /* attach the shader to the program */
        glAttachShader(_program, shader);

        /* delete the shader - it won't actually be
        * destroyed until the program that it's attached
        * to has been destroyed */
        glDeleteShader(shader);
    }

    return shader;
}

GLuint program::compile(GLenum type, const std::filesystem::path& path)
{
    auto file_mapping = map_file(path.c_str());
    if (!file_mapping)
    {
        return 0;
    }

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&file_mapping->addr, &file_mapping->length);
    glCompileShader(shader);
    
    unmap_file(*file_mapping);

    /* Make sure the compilation was successful */
    GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        /* get the shader info log */
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &file_mapping->length);
        char* log = new char[file_mapping->length];
        glGetShaderInfoLog(shader, file_mapping->length, &result, log);

        /* print an error message and the info log */
        //LOGD("Unable to compile %s: %s", path.c_str(), log);
        delete[] log;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool program::bind_attribute(const char* attrib, GLuint index)
{
    _attribs.emplace(attrib, index);
    glBindAttribLocation(_program, index, attrib);
    return true;
}

bool program::link()
{
    GLint result;

    /* link the program and make sure that there were no errors */
    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        GLint length;

        /* get the program info log */
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &length);
        char* log = new char[length];
        glGetProgramInfoLog(_program, length, &result, log);

        /* print an error message and the info log */
        //LOGD("Program linking failed: %s", log);
        delete[] log;

        /* delete the program */
        glDeleteProgram(_program);
        _program = 0;
    }

    // Before linking, bind global matrices
    auto matrices_loc = glGetUniformBlockIndex(_program, "GlobalMatrices");
	glUniformBlockBinding(_program, matrices_loc, GlobalMatricesIndex);

    return _program != 0;
}

void program::attribute_pointer(const char* attrib, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr)
{
    auto pos = attribute_location(attrib);
    _stored_attributes.push_back(attribute {
        .pos = pos,
        .size = size,
        .type = type,
        .normalized = normalized,
        .stride = stride,
        .ptr = ptr
    });
}


void program::use_attributes()
{
    for (auto& attr : _stored_attributes)
    {
        glEnableVertexAttribArray(attr.pos);
        glVertexAttribPointer(attr.pos, attr.size, attr.type, attr.normalized, attr.stride, attr.ptr);
    }
}
