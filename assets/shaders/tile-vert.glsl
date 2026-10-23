#version 330 core

const vec2 position_array[6] = vec2[](

    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),

    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0)
);

const vec4 default_color  = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 sprite_no_tint = vec4(1.0, 1.0, 1.0, 1.0);

// map_tile_flag_e
const uint tile_flag_textured = 2u;

// vertex attributes
layout(location = 0) in uint in_tile;
layout(location = 1) in vec4 in_text_rect; // u_left, v_top, u_right, v_bottom

// uniforms
uniform mat4  u_view_proj;
uniform uint  u_map_count_cols;
uniform vec3  u_map_origin;
uniform int   u_map_offset_rows;
uniform int   u_map_offset_cols;
uniform float u_tile_unit_size;
uniform vec4  u_color_table[16];

// vertex output
flat out vec4  vert_color;
     out vec2  vert_text_coords;
flat out float vert_textured;

void
main() {

    uint in_flags    = (in_tile >> 24u) & 0xFFu;
    uint in_textured = in_flags & tile_flag_textured;
    uint in_color    = (in_tile >> 16u) & 0xFFu;
    int  in_index    = int(in_tile & 0xFFFFu);

    // calculate the normalized color
    // a sprite with no color isn't tinted
    vert_textured = (in_textured != 0u) ? 1.0 : 0.0;
    vert_color    = (in_color < uint(16))
        ? u_color_table[in_color]
        : ((in_textured != 0u) ? sprite_no_tint : default_color);

    // get the position and tile index
    int index_position = gl_VertexID;

    // get the position from the array
    vec2 position = position_array[index_position];

    // the position is normalized, so it picks the corner of the sprite rect
    vert_text_coords = mix(in_text_rect.xy, in_text_rect.zw, position);

    // get the row and column
    int col = (in_index % int(u_map_count_cols)) + u_map_offset_cols; 
    int row = (in_index / int(u_map_count_cols)) + u_map_offset_rows;

    // calculate the tile position
    vec2 tile_position = vec2(col, row) * vec2(u_tile_unit_size, u_tile_unit_size); 

    // calculate the world position
    vec3 world_position = u_map_origin + vec3(
        tile_position.x + position.x * u_tile_unit_size,
        0.0,
        tile_position.y + position.y * u_tile_unit_size
    );

    // set the output 
    gl_Position = u_view_proj * vec4(world_position, 1.0f);
}

