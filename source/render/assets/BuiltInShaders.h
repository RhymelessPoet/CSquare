#pragma once
#include <string>

static constexpr std::string_view VertexShader = R"(
#version 450 core
layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;
layout(location = 0) out vec3 color;

layout(std140, binding = 0) uniform VPMatrix
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform MMatrix
{
    mat4 model;
};

void main()
{
    mat4 mvpMatrix = projection * view * model;
    gl_Position = mvpMatrix * vec4(_position.x, _position.y, _position.z, 1.0);
    color = _color;
}

)";

static constexpr std::string_view FragmentShader = R"(
#version 450 core

layout(location = 0) in vec3 color;
out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1.0f);
}

)";