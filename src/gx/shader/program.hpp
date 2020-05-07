#pragma once

// TODO(gpascualg): I don't really like this include here
#include "entity/transform.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


class program
{
public:
    enum class uniform_type
    {
        none,
        transform
    };

private:
    struct attribute
    {
        GLuint pos;
        GLint size;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        const void* ptr;
    };

    template <typename D>
    struct uniform
    {
        GLuint pos;
        uniform_type type;
        D* data;
    };

public:
    program();

    GLuint attach(GLenum type, std::filesystem::path path);
    bool bind_attribute(const char* attrib, GLuint index);
    bool link();
    void attribute_pointer(const char* attrib, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr);
    void use_attributes();

    template <typename D=void>
    void uniform_pointer(const char* unif, uniform_type type, D* data);

    template <typename T>
    void use_uniforms(T* entity);

    inline void create();
    inline GLuint prog() const;
    inline GLuint attribute_location(const char* attrib);
    inline GLuint uniform_location(const char* unif);
    inline bool bind();

private:
    GLuint compile(GLenum type, const std::filesystem::path& path);

private:
    GLuint _program;
    std::unordered_map<std::string, GLuint> _attribs;
    std::unordered_map<std::string, GLuint> _uniforms;
    std::vector<attribute> _stored_attributes;
    std::vector<std::variant<
        uniform<void>, uniform<glm::mat4>, uniform<glm::vec4>>> _stored_uniforms;
};


inline void program::create()
{
    _program = glCreateProgram();
}

inline GLuint program::prog() const
{
    return this->_program;
}

inline GLuint program::attribute_location(const char* attrib)
{
    return _attribs[attrib];
}

inline GLuint program::uniform_location(const char* unif)
{
    auto ret = _uniforms.find(unif);
    if (ret != _uniforms.end())
    {
        return ret->second;
    }

    GLuint unif_location = glGetUniformLocation(this->_program, unif);
    _uniforms.insert(std::make_pair(unif, unif_location));
    return unif_location;
}

inline bool program::bind()
{
    glUseProgram(_program);
    return glGetError() == 0;
}

template <typename D>
void program::uniform_pointer(const char* unif, uniform_type type, D* data)
{
    _stored_uniforms.emplace_back(uniform<D> {
        .pos = uniform_location(unif),
        .type = type,
        .data = data
    });
}

template <typename T>
void program::use_uniforms(T* entity)
{
    for (auto& variant : _stored_uniforms)
    {
        std::visit([entity](auto& uniform) {
            using E = typename std::remove_pointer<std::decay_t<decltype(uniform.data)>>::type;

            if (uniform.type == uniform_type::transform)
            {
                glUniformMatrix4fv(uniform.pos, 1, GL_FALSE, glm::value_ptr(entity->template get<transform>()->mat()));
            }
            else
            {
                if constexpr (std::is_same_v<E, glm::mat4>)
                {
                    glUniformMatrix4fv(uniform.pos, 1, GL_FALSE, glm::value_ptr(*uniform.data));
                }
                else if constexpr (std::is_same_v<E, glm::vec4>)
                {
                    glUniform4fv(uniform.pos, 1, glm::value_ptr(*uniform.data));
                }
            }
        }, variant);
    }
}
