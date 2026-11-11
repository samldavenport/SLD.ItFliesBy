#version 330 core

in  vec4  vert_color;
in  vec2  vert_text_coords;
in  float vert_textured;
out vec4  out_color;

uniform sampler2D u_texture_0_sprite_atlas;
uniform sampler2D u_texture_1_font_atlas;

void main() {

    // 0 is a solid color, 1 tints the sprite with the vertex color,
    // 2 uses the font's coverage (one channel) as the alpha of the vertex color
    int mode = int(vert_textured + 0.5);
    if (mode == 1) {
        out_color = vert_color * texture(u_texture_0_sprite_atlas, vert_text_coords);
    }
    else if (mode == 2) {
        float coverage = texture(u_texture_1_font_atlas, vert_text_coords).r;
        out_color      = vec4(vert_color.rgb, vert_color.a * coverage);
    }
    else {
        out_color = vert_color;
    }
}
