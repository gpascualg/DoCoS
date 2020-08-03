#pragma once

#include "entity/entity.hpp"
#include "entity/transform.hpp"
#include "gx/shader/program.hpp"
#include "gx/common.hpp"

#include <initializer_list>


class mesh : public entity<mesh>
{
public: 
    void construct(float* vertices, std::size_t vertices_size, float* indices, std::size_t indices_size,
        std::initializer_list<program*>&& progams);

    void scheme_created();

    void sync();

    template <template <typename...> typename S, typename... components>
    constexpr inline void scheme_information(const S<components...>& scheme)
    {
        // Meshes require transforms
        scheme.template require<transform>();
    }

private:
    GLuint _VAO;
    std::vector<program*> _programs;
    ::ticket<entity<transform>>::ptr _transform;
};
