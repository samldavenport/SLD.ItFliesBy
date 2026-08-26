#version 330 core

// vertex attributes
layout(location = 0) in vec2 in_corner;
layout(location = 1) in uint in_tile_id;
layout(location = 2) in uint in_background_color;

// uniforms
uniform vec2      u_vec2_tile_size;
uniform vec2      u_vec2_map_dimensions;
uniform mat4      u_mat4_view_proj;
uniform mat4      u_mat4_model;
uniform sampler2D u_sampler2d_texture;

// vertex output
out vec2      vert_uv;
flat out uint vert_tile_id;
flat out uint vert_background_color;

void
main() {

    uint inst_id         = uint(gl_InstanceID);
    uint map_dims_width  = uint(u_vec2_map_dimensions.x);
    uint map_dims_height = uint(u_vec2_map_dimensions.y);
    
    // gl_InstanceID gives us the tile's index in the map.
    uint column = inst_id % map_dims_width;
    uint row    = inst_id / map_dims_width;

    // Map coordinates become X/Z world coordinates.
    vec2 tile_position  = vec2(column, row) * u_vec2_tile_size;
    vec2 corner         = (in_corner - 0.5) * u_vec2_tile_size;
    vec3 world_position = vec3(
        tile_position.x + corner.x,
        0.0,
        tile_position.y + corner.y
    );

    // set outputs
    vert_uv               = in_corner;
    vert_tile_id          = in_tile_id;
    vert_background_color = in_background_color;
    
    // set position
    gl_Position = u_mat4_view_proj * vec4(world_position, 1.0);
}

