#version 330 core

uniform mat4 view_projection;

layout(location = 0) in vec3 vertex_position;

out vec3 near_point;
out vec3 far_point;

vec3
unproject_point(
    float x,
    float y,
    float z) {

    mat4 view_projection_inverse = inverse(view_projection);

    vec4 unprojected_point = view_projection_inverse * vec4(x,y,z,1.0); 

    return unprojected_point.xyz / unprojected_point.w;
}

void
main() {

    near_point = unproject_point(vertex_position.x, vertex_position.y, 0.0).xyz;
    far_point  = unproject_point(vertex_position.x, vertex_position.y, 1.0).xyz;

    gl_Position = vec4(vertex_position, 1.0);
}
