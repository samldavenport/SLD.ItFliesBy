#version 330 core

uniform mat4 u_mat4_view_proj;
uniform mat4 u_mat4_model;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec4 in_color;

out vec4 vert_color;

void main() {

    vert_color = in_color;

    gl_Position = (
        u_mat4_view_proj *
        u_mat4_model     *
        vec4(in_pos, 1.0)
    );
}
