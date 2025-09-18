#pragma once
#include <string>

static constexpr std::string_view HDR_Skybox_VS = R"(
#version 450 core
layout(location = 0) in vec3 _position;
layout(location = 0) out vec3 world_position;

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
    vec4 position = vec4(_position.x, _position.y, _position.z, 1.0);
    gl_Position = position;
    world_position = inverse(mvpMatrix) * position;
}

)";

static constexpr std::string_view HDR_Skybox_FS = R"(
#version 450 core

layout(location = 0) in vec3 world_position;
layout(binding = 2) uniform sampler2D hdr_texture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1.0f);
}

)";