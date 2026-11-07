#version 330 core

// an upright backdrop standing on the ground behind the map, facing the camera
// the corners come from gl_VertexID so there's no vertex data
const vec2 parallax_corners[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
);

uniform mat4  u_view_proj;
uniform vec4  u_backdrop;   // x left, x right, y top, z
uniform vec2  u_image_size; // the world size of one image, the bottom of the image is on the ground
uniform float u_camera_x;

out vec2 vert_uv;

void main() {

    vec2 corner = parallax_corners[gl_VertexID];

    vec3 world_position = vec3(
        mix(u_backdrop.x, u_backdrop.y, corner.x),
        corner.y * u_backdrop.z,
        u_backdrop.w
    );

    // u is measured from the camera so each layer's offset decides how fast it scrolls
    // the images are uploaded from the top row down, so the ground is v = 1
    // above the image v goes negative and clamps to the top row
    vert_uv = vec2(
        (world_position.x - u_camera_x) / u_image_size.x,
        1.0 - (world_position.y / u_image_size.y)
    );

    gl_Position = u_view_proj * vec4(world_position, 1.0);
}
