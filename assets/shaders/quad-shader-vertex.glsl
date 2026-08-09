#version 330 core

uniform mat4 u_mat4_proj;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec4 in_color;

out vec4 vert_color;

void main() {

    gl_Position = vec4(in_pos, 1.0);
    vert_color = in_color;
}
