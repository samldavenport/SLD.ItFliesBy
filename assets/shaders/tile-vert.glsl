#version 330 core

const vec2 position_array[6] = vec2[](

    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),

    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0)
);

// vertex attributes
layout(location = 0) in uint in_color;

// uniforms
uniform mat4  u_view_proj;
uniform uint  u_map_count_rows;
uniform uint  u_map_count_cols;
uniform int   u_map_offset_row;
uniform int   u_map_offset_col;
uniform float u_tile_width;
uniform float u_tile_height;

// vertex output
flat out vec4 vert_color;

void
main() {

    // calculate the normalized color
    vert_color = vec4(
        float((in_color >> 24u) & 0xFFu) / 255.0,
        float((in_color >> 16u) & 0xFFu) / 255.0,
        float((in_color >>  8u) & 0xFFu) / 255.0,
        float( in_color         & 0xFFu) / 255.0
    );

    // get the position and tile index
    uint index_position = uint(gl_VertexID);
    uint index_tile     = uint(gl_InstanceID);

    // get the position from the array
    vec2 position = position_array[index_position];

    // get the row and column
    uint col = index_tile % u_map_count_cols; 
    uint row = index_tile / u_map_count_rows; 

    // calculate the tile position
    vec2 tile_position = vec2(col, row) * vec2(u_tile_width, u_tile_height); 
    tile_position.x += (u_tile_width  * u_map_offset_col);
    tile_position.y += (u_tile_height * u_map_offset_row);

    // calculate the world position
    vec3 world_position = vec3(
        tile_position.x + position.x * u_tile_width,
        0.0,
        tile_position.y + position.y * u_tile_height
    );

    // set the output 
    gl_Position = u_view_proj * vec4(world_position, 1.0f);
}

