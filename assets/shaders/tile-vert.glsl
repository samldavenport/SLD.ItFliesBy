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
const uint tile_flag_textured    = 2u;
const uint tile_flag_wall_row    = 4u;
const uint tile_flag_wall_column = 8u;

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

    // walls are on the tile edges, so they're indexed on a grid one column wider
    bool is_wall_row    = ((in_flags & tile_flag_wall_row)    != 0u);
    bool is_wall_column = ((in_flags & tile_flag_wall_column) != 0u);
    bool is_wall        = (is_wall_row || is_wall_column);
    int  index_stride   = int(u_map_count_cols) + (is_wall ? 1 : 0);

    // the position is normalized, so it picks the corner of the sprite rect
    // a wall's y goes up from the floor, so its bottom edge is the bottom of the sprite
    vert_text_coords = is_wall
        ? vec2(mix(in_text_rect.x, in_text_rect.z, position.x), mix(in_text_rect.w, in_text_rect.y, position.y))
        : mix(in_text_rect.xy, in_text_rect.zw, position);

    // get the row and column
    int col = (in_index % index_stride) + u_map_offset_cols; 
    int row = (in_index / index_stride) + u_map_offset_rows;

    // calculate the tile position
    vec2 tile_position = vec2(col, row) * vec2(u_tile_unit_size, u_tile_unit_size); 

    // calculate the world position
    // floors lie flat on the tile,
    // row walls stand on the line z = row and run along x,
    // column walls stand on the line x = col and run along z
    vec3 local_position;
    if (is_wall_row) {
        local_position = vec3(position.x * u_tile_unit_size, position.y * u_tile_unit_size, 0.0);
    }
    else if (is_wall_column) {
        local_position = vec3(0.0, position.y * u_tile_unit_size, position.x * u_tile_unit_size);
    }
    else {
        local_position = vec3(position.x * u_tile_unit_size, 0.0, position.y * u_tile_unit_size);
    }
    vec3 world_position = u_map_origin + vec3(tile_position.x, 0.0, tile_position.y) + local_position;

    // set the output 
    gl_Position = u_view_proj * vec4(world_position, 1.0f);
}

