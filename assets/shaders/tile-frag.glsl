#version 460 core

flat in vec4  vert_color;
     in vec2  vert_text_coords;
flat in float vert_textured;

out vec4 frag_color;

uniform sampler2D u_texture_0_sprite_atlas;

void main() {

    // solid tiles (0) use the vertex color, textured tiles (1) tint the sprite with it
    vec4 text_color = texture(u_texture_0_sprite_atlas, vert_text_coords);
    frag_color      = vert_color * mix(vec4(1.0), text_color, vert_textured);
}
