#include "gx/common.hpp"
#include "gx/camera/camera.hpp"
#include "updater/executor.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>


void camera::construct(const glm::vec3& scene_dimensions)
{
    glGenBuffers(1, &_matrices_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, _matrices_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, GlobalMatricesIndex, _matrices_ubo, 0, sizeof(glm::mat4));
    
    _model = glm::scale(glm::mat4(1.0), glm::vec3(2 / scene_dimensions[0], 2 / scene_dimensions[1], 2 / scene_dimensions[2]));
    
    _vup = glm::vec3(0, 0, -1);
    _dir = glm::vec3(0, -1, 0); // _vrp = (0, 0, 0) = (1, 1, 0) + (-1, -1, 0)
    _obs = glm::vec3(0, 1, 0);

    calculate_view();
    calculate_proj();
}

void camera::look_towards(glm::vec3 dir)
{
    auto diff = dir - _dir;
    _vup = glm::normalize(_vup + diff * _vup); 

    _dir = dir;
    calculate_view();
}

void camera::calculate_view()
{
    _view = glm::lookAt(_obs, _obs + _dir, _vup);
    schedule();
}

void camera::calculate_proj()
{
    _proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    schedule();
}

void camera::schedule()
{
    if (!_scheduled)
    {
        _scheduled = true;
        
        cppcoro::sync_wait(executor::instance->schedule([this]() {
            _mvp = _proj * _view * _model;

            glBindBuffer(GL_UNIFORM_BUFFER, _matrices_ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(_mvp));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            _scheduled = false;
        }));
    }
}
