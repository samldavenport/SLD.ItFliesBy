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

    // the map is depth tested, so a clear pixel would still hide whatever is behind it
    // dropping it lets cutouts in a sprite (fences, window frames) show what's behind them
    // TODO(SLD): partially clear pixels (glass) still blend in draw order,
    // they'll need their own pass sorted back to front with depth writes off
    if (frag_color.a < 0.5) {
        discard;
    }
}
