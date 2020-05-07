#include "containers/pooled_static_vector.hpp"
#include "containers/dictionary.hpp"
#include "entity/entity.hpp"
#include "entity/scheme.hpp"
#include "gx/camera/camera.hpp"
#include "gx/mesh/mesh.hpp"
#include "gx/shader/program.hpp"
#include "updater/executor.hpp"
#include "updater/updater.hpp"

#include <cppcoro/sync_wait.hpp>

#include <glm/gtx/string_cast.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "linmath.h"

#include <iostream>


int xc = 5;

class X : public entity<X>
{
public:
    void construct()
    {}

    void update()
    {
        xc = xc * (xc + 1);
        std::cout << "UPDATE X" << std::endl;
    }
};


constexpr uint32_t prereserved_size = 256;
constexpr uint32_t alloc_num = 1000;


float vertices[] = {
    0.5f,  0.5f, 0.0f,  // top right
    0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,  // first Triangle
    1, 2, 3   // second Triangle
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

uint64_t CAMERA_ID = 0;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto ticket = store<entity<camera>>::get(CAMERA_ID);
    if (ticket && ticket->valid())
    {
        float z = 0;
        glReadPixels(xpos, 600 - ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

        float x = xpos / 800;
        float y = (600 - ypos) / 600;
        glm::vec4 screen = glm::vec4(x, y, 1.0f, 1.0f);

        auto cam = reinterpret_cast<camera*>(ticket->get());
        auto mvp = cam->mvp();
        auto proj = glm::inverse(mvp) * (screen * 2.0f - 1.0f);
        // proj = proj / proj.w;

        auto dir = proj - glm::vec4(cam->obs(), 1.0);
        cam->look_towards(glm::normalize(glm::vec3(dir)));
    }
}


class quick_test
{
    template <typename T, uint32_t S=prereserved_size> using vec = pooled_static_vector<T, entity<T>, S>;
    template <typename T, uint32_t S=prereserved_size> using dic = dictionary<T, entity<T>, S>;

public:
    quick_test() :
        obj_scheme(obj_scheme.make()),
        camera_scheme(camera_scheme.make())
    {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit())
        {
            exit(EXIT_FAILURE);
        }

        GLuint vertex_buffer, vertex_shader, fragment_shader;
        GLint vpos_location, vcol_location;
    
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
        _window = glfwCreateWindow(800, 600, "Simple example", NULL, NULL);
        if (!_window)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    
        glfwSetKeyCallback(_window, key_callback);
        glfwSetCursorPosCallback(_window, cursor_position_callback);

        glfwMakeContextCurrent(_window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        
        glfwSwapInterval(1);

        glViewport(0, 0, 800, 600);
    
        // NOTE: OpenGL error checks have been omitted for brevity
    
        _program.create();
        _program.attach(GL_VERTEX_SHADER, "resources/sample.vs");
        _program.attach(GL_FRAGMENT_SHADER, "resources/sample.fs");
        _program.bind_attribute("vCol", 0);
        _program.bind_attribute("vPos", 1);
        _program.link();
        _program.attribute_pointer("vPos", 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
        _program.uniform_pointer("transform", program::uniform_type::transform, (void*)nullptr);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        glDepthRange(0.0f, 1.0f);
    }

    void run()
    {
        auto executor = ::executor::get();
        auto overlap_scheme = overlap(obj_scheme, camera_scheme);
        auto updater = overlap_scheme.make_updater(std::thread::hardware_concurrency());

        executor->create_with_callback(camera_scheme, [](auto camera){
                CAMERA_ID = camera->id();
                std::cout << "HABEMUS CAMERA" << std::endl;
                return std::tuple(camera);
            }, 
            camera_scheme.args<camera>(glm::vec3(20, 20, 20))
        );

        executor->create_with_callback(obj_scheme,
            [](auto x, auto transform, auto mesh)
            {
                transform->local_scale({10, 1, 10});
                return std::tuple(x, transform, mesh);
            },
            obj_scheme.args<X>(), 
            obj_scheme.args<transform>(-10.0f, -10.0f),
            obj_scheme.args<mesh>((float*)vertices, (std::size_t)sizeof(vertices), (float*)indices, (std::size_t)sizeof(indices), std::initializer_list<program*> { &_program })
        );

        executor->create(obj_scheme,
            obj_scheme.args<X>(), 
            obj_scheme.args<transform>(-2.0f, 0.0f),
            obj_scheme.args<mesh>((float*)vertices, (std::size_t)sizeof(vertices), (float*)indices, (std::size_t)sizeof(indices), std::initializer_list<program*> { &_program })
        );

        auto id = executor->create_with_callback(obj_scheme,
            [](auto x, auto transform, auto mesh)
            {
                transform->local_scale({2, 1, 2});
                return std::tuple(x, transform, mesh);
            },
            obj_scheme.args<X>(), 
            obj_scheme.args<transform>(1.0f, -4.0f),
            obj_scheme.args<mesh>((float*)vertices, (std::size_t)sizeof(vertices), (float*)indices, (std::size_t)sizeof(indices), std::initializer_list<program*> { &_program })
        );

        executor->execute_tasks();
 
        while (!glfwWindowShouldClose(_window))
        {
            float ratio;
            int width, height;
            mat4x4 m, p, mvp;
    
            glfwGetFramebufferSize(_window, &width, &height);
            ratio = width / (float) height;
    
            glViewport(0, 0, width, height);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClearDepth(1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
            executor->update(updater);
            executor->execute_tasks();
            executor->draw(updater);
    
            glfwSwapBuffers(_window);
            glfwPollEvents();
        }
 
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

private:
    // Rendering
    GLFWwindow* _window;
    program _program;

    // Prebuilt schemes
    decltype(scheme<dic<X>, dic<transform>, dic<mesh>>::make()) obj_scheme;
    decltype(scheme<dic<camera, 1>>::make()) camera_scheme;
};



int main()
{
    quick_test test;
    test.run();

    return 0;
}
