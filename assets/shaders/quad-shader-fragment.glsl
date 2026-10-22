#version 330 core

in  vec4  vert_color;
in  vec2  vert_text_coords;
in  float vert_textured;
out vec4  out_color;

uniform sampler2D u_texture_0_sprite_atlas;

void main() {

    // solid quads (0) use the vertex color, textured quads (1) tint the sprite with it
    vec4 text_color = texture(u_texture_0_sprite_atlas, vert_text_coords);
    out_color       = vert_color * mix(vec4(1.0), text_color, vert_textured);
}
