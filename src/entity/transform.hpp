#pragma once

#include "entity/entity.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class transform : public entity<transform>
{
public:
    void construct(float tx, float ty)
    {
        _parent = nullptr;
        _global = glm::translate(glm::mat4(1.0), glm::vec3(tx, 0, ty));
        _local = glm::mat4(1.0);
    }

    void scale(const glm::vec3& sc)
    {
        _global = glm::scale(_global, sc);
    }

    void translate(const glm::vec3& sc)
    {
        _global = glm::translate(_global, sc);
    }

    void local_scale(const glm::vec3& sc)
    {
        _local = glm::scale(_local, sc);
    }

    void local_translate(const glm::vec3& sc)
    {
        _local = glm::translate(_local, sc);
    }

    inline glm::mat4 mat() const
    {
        return mat_impl() * _local;
    }

    void parent(::ticket<entity<transform>>::ptr transform)
    {
        _parent = transform;
    }

    ::ticket<entity<transform>>::ptr parent()
    {
        return _parent;
    }

private:

    inline glm::mat4 mat_impl() const
    {
        if (_parent && _parent->valid())
        {
            return _parent->get<transform>()->mat() * _global;
        }

        return _global;
    }

private:
    ::ticket<entity<transform>>::ptr _parent;

    glm::mat4 _local;
    glm::mat4 _global;
};
