#version 330 core

const int PARALLAX_LAYER_COUNT = 3;

// layer 0 is the back, the last layer is the front
uniform sampler2DArray u_layers;

// only xy is used, it's a vec4 so the array can be set with glUniform4fv
uniform vec4 u_uv_offset[PARALLAX_LAYER_COUNT];

in  vec2 vert_uv;
out vec4 frag_color;

void main() {

    vec4 color = vec4(0.0);

    // composite back to front
    for (int i_layer = 0; i_layer < PARALLAX_LAYER_COUNT; ++i_layer) {

        vec2 uv    = vert_uv + u_uv_offset[i_layer].xy;
        vec4 texel = texture(u_layers, vec3(uv, float(i_layer)));
        color      = mix(color, texel, texel.a);
    }

    frag_color = color;
}
