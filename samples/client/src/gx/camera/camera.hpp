#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "entity/entity.hpp"


class camera : public entity<camera>
{
public:
    void construct(const glm::vec3& scene_dimensions);

    inline glm::mat4 mvp();
    inline glm::vec3 obs();
    inline glm::vec3 up();
    inline glm::mat4 model();
    inline glm::mat4 proj();

    void look_towards(glm::vec3 dir);

private:
    void calculate_view();
    void calculate_proj();
    void schedule();

private:
    bool _scheduled;
    glm::mat4 _model;
    glm::mat4 _view;
    glm::mat4 _proj;
    glm::mat4 _mvp;

    glm::vec3 _obs;
    glm::vec3 _dir;
    glm::vec3 _vup;

    GLuint _matrices_ubo;
};


inline glm::mat4 camera::mvp()
{
    return _mvp;
}

inline glm::vec3 camera::obs()
{
    return _obs;
}

inline glm::vec3 camera::up()
{
    return _vup;
}

inline glm::mat4 camera::model()
{
    return _model;
}

inline glm::mat4 camera::proj()
{
    return _proj;
}



