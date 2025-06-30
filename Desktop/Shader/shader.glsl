#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform float iTime;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec2 uv = TexCoords * 4.0 - vec2(2.0);
    uv.x -= 0.5; // Center the fractal

    vec2 z = vec2(0.0);
    vec2 c = uv;
    
    float iter = 0.0;
    const float MAX_ITER = 100.0;
    
    for(float i = 0.0; i < MAX_ITER; i++) {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        
        if(length(z) > 2.0) {
            iter = i;
            break;
        }
        
        if(i == MAX_ITER - 1.0) {
            iter = MAX_ITER;
        }
    }
    
    if(iter == MAX_ITER) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        float hue = iter / MAX_ITER + iTime * 0.1;
        vec3 color = hsv2rgb(vec3(hue, 0.8, 1.0));
        FragColor = vec4(color, 1.0);
    }
} 