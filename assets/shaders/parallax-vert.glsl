#version 330 core

// a fullscreen quad, the corners come from gl_VertexID so there's no vertex data
const vec2 parallax_corners[6] = vec2[](
    vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0),
    vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0)
);

out vec2 vert_uv;

void main() {

    vec2 corner = parallax_corners[gl_VertexID];

    // the images are uploaded from the top row down, so the top of the screen is v = 0
    vert_uv = vec2(
        corner.x * 0.5 + 0.5,
        0.5 - corner.y * 0.5
    );

    gl_Position = vec4(corner, 0.0, 1.0);
}
