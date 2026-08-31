#version 330

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec3 in_color;

out vec3 vert_color;

uniform vec2 u_offsets[100];

void
main() {

    // get the offset from the uniform buffer
    vec2 offset = u_offsets[gl_InstanceID];
   
    // calculate the position
    gl_Position = vec4(in_pos + offset, 0.0, 1.0);

    // set the vertex color output
    vert_color = in_color;
}
