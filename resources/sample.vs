#version 330

layout(std140) uniform GlobalMatrices
{
	mat4 mvp;
};

uniform mat4 transform;
in vec3 vCol;
in vec3 vPos;
smooth out vec3 color;

void main()
{
    gl_Position = mvp * transform * vec4(vPos.x, 0.0, vPos.y, 1.0);
    color = vec3(1.0, 1.0, 0.0);
}
