#version 330 core

uniform mat4 u_mat4_view_proj;
uniform mat4 u_mat4_model;

layout (location = 0) in vec3  in_pos;
layout (location = 1) in vec4  in_color;
layout (location = 2) in vec2  in_text_coords;
layout (location = 3) in float in_textured;

out vec4  vert_color;
out vec2  vert_text_coords;
out float vert_textured;

void main() {

    vert_color       = in_color;
    vert_text_coords = in_text_coords;
    vert_textured    = in_textured;

    gl_Position = (
        u_mat4_view_proj *
        u_mat4_model     *
        vec4(in_pos, 1.0)
    );
}
