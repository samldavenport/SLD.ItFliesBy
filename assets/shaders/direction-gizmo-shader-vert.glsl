#version 330 core

uniform mat4 u_mat4_proj;
uniform mat4 u_mat4_view;
uniform mat4 u_mat4_model;

out vec4 vert_color;

const vec3 directional_axes_pos[6] = vec3[](
    vec3(0,0,0), vec3(0.25,0,0), // x-right
    vec3(0,0,0), vec3(0,0.25,0), // y-up
    vec3(0,0,0), vec3(0,0,0.25)  // z-forward
);

const vec4 directional_axes_colors[6] = vec4[] (
    vec4(1,0,0,1),vec4(1,0,0,1), // x-red
    vec4(0,1,0,1),vec4(0,1,0,1), // y-green
    vec4(0,0,1,1),vec4(0,0,1,1)  // z-blue
);

void main() {

    vec3 vert_pos = directional_axes_pos    [gl_VertexID];
    vert_color    = directional_axes_colors [gl_VertexID]; 

    gl_Position = (
        u_mat4_proj  *
        u_mat4_view  *
        u_mat4_model *
        vec4(vert_pos, 1.0)
    );
}