#version 330 core

uniform mat4 view_projection;

in vec3 near_point;
in vec3 far_point;

vec4
grid(vec3 frag_pos, float scale) {

    vec2 coord      = frag_pos.xz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid       = abs(fract(coord - 0.5) - 0.5) / derivative;
    
    float line     = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);

    vec4 color     = vec4(0.19608, 0.18824, 0.18431, 1.0 - min(line, 1.0));

    // z axis
    if(frag_pos.x > -0.1 * minimumx && frag_pos.x < 0.1 * minimumx) {
        color.x = 0.019;
        color.y = 0.345;
        color.z = 0.533;
    }
    
    // x axis
    if(frag_pos.z > -0.1 * minimumz && frag_pos.z < 0.1 * minimumz) {
        color.x = 0.760;
        color.y = 0.254;
        color.z = 0.050;
    }
    
    return color;
}

float
compute_depth_new(vec3 frag_pos) {
    
    float far  = gl_DepthRange.far; 
    float near = gl_DepthRange.near;

    vec4 clip_space_pos  = view_projection * vec4(frag_pos.xyz,1.0);

    float ndc_depth = clip_space_pos.z / clip_space_pos.w;

    float depth = (((far-near) * ndc_depth) + near + far) / 2.0;

    return(depth);
}


float
compute_linear_depth(vec3 pos) {

    float far  = gl_DepthRange.far; 
    float near = gl_DepthRange.near;

    vec4  clip_space_pos   = view_projection * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth      = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100
    
    return (linearDepth / far); // normalize
}

void
main() {

    float t = -near_point.y / (far_point.y - near_point.y);

    vec3 frag_pos = near_point + t * (far_point - near_point);
    
    gl_FragDepth = compute_depth_new(frag_pos);

    float linear_depth = compute_linear_depth(frag_pos);
    float fading       = max(0, (0.5 - linear_depth));
    
    vec4 grid_color = (grid(frag_pos, 10) + grid(frag_pos, 1)) * float(t > 0); // adding multiple resolution for the grid
    gl_FragColor = grid_color; 
    gl_FragColor.a *= fading;

}
