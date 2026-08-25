#version 460 core

in vec2 vert_uv;


out vec4 frag_color;

void main() {

    frag_color = texture(u_texture, vert_uv);
}
