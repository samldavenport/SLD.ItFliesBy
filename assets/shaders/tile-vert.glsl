#version 330 core

// vertex attributes
layout(location = 0) in vec2 in_corner;
layout(location = 1) in uint in_tile_id;
layout(location = 2) in uint in_background_color;

// uniforms
uniform vec2      u_tile_size;
uniform vec2      u_map_dimensions;
uniform mat4      u_view_proj;
uniform mat4      u_model;
uniform sampler2D u_texture;

// vertex output
out      vec2 vert_uv;
out flat uint vert_tile_id;
out flat uint vert_background_color;

void main() {
/*
    // gl_InstanceID gives us the tile's index in the map.
    const uint column = uint(gl_InstanceID) % uint(u_map_dimensions.x);
    const uint row    = uint(gl_InstanceID) / uint(u_map_dimensions.x);

    // map coordinates become X/Z world coordinates.
    const vec2 tile_position  = vec2(column, row) * u_tile_size;
    const vec2 corner         = (in_corner - 0.5) * u_tile_size;
    const vec3 world_position = vec3(
        tile_position.x + corner.x,
        0.0,
        tile_position.y + corner.y
    );

    // set outputs
    vert_uv               = in_corner;
    vert_tile_id          = in_tile_id;
    vert_background_color = in_background_color;
    gl_Position           = u_view_proj * vec4(world_position, 1.0);
*/
    }

