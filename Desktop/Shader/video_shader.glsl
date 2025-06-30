#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D videoTexture;
uniform float iTime;
uniform vec2 iResolution;

// Complex number operations
vec2 cmul(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

// Noise functions
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 st) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    // Daha detaylı ve organik görünüm için oktav sayısını artırdık
    for(int i = 0; i < 6; i++) {
        value += amplitude * noise(st * frequency);
        st = st * 2.0 + iTime * 0.1;
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}

// Mini Julia set calculation
float julia(vec2 p, vec2 c, float scale) {
    p *= scale;
    float iter = 0.0;
    const float MAX_ITER = 32.0;
    
    for(float i = 0.0; i < MAX_ITER; i++) {
        p = cmul(p, p) + c;
        if(length(p) > 2.0) {
            iter = i;
            break;
        }
    }
    
    return smoothstep(0.0, MAX_ITER, iter) * 0.8;
}

void main() {
    // Fix video orientation
    vec2 fixedCoords = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec4 videoColor = texture(videoTexture, fixedCoords);
    
    // Create normalized coordinates
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;
    
    // Create flowing noise field
    float t = iTime * 0.2;
    vec2 flowOffset = vec2(
        fbm(uv + t * 0.5),
        fbm(uv + vec2(2.3, 1.7) + t * 0.3)
    );
    
    // Create multiple scattered Julia sets
    float pattern = 0.0;
    for(int i = 0; i < 5; i++) {
        float fi = float(i) / 4.0;
        vec2 offset = vec2(
            sin(t * (0.5 + fi) + fi * 6.28) * 0.5,
            cos(t * (0.3 + fi) + fi * 6.28) * 0.5
        );
        
        vec2 c = vec2(
            0.25 * sin(t * (0.2 + fi)),
            0.25 * cos(t * (0.3 + fi))
        );
        
        // Modulate position with flow field
        vec2 pos = uv + offset + flowOffset * 0.2;
        
        // Add smaller Julia sets with varying parameters
        pattern += julia(pos, c, 2.0 + sin(fi * 6.28) * 0.5) * (0.3 - fi * 0.1);
    }
    
    // Add flow field influence
    pattern += fbm(uv * 3.0 + flowOffset + t) * 0.2;
    pattern = clamp(pattern, 0.0, 1.0);
    
    // Create organic color palette
    vec3 fractalColor = vec3(0.0);
    float angle = pattern * 6.28318 + t;
    fractalColor.r = 0.5 + 0.5 * sin(angle);
    fractalColor.g = 0.5 + 0.5 * sin(angle + 2.09439);
    fractalColor.b = 0.5 + 0.5 * sin(angle + 4.18879);
    
    // Video-reactive effect
    float luminance = dot(videoColor.rgb, vec3(0.299, 0.587, 0.114));
    float videoInfluence = smoothstep(0.2, 0.8, luminance);
    
    // Dynamic pattern strength
    float patternStrength = 0.4 + 0.2 * sin(t * 0.7);
    patternStrength *= (1.0 - luminance * 0.6); // Reduce in bright areas
    
    // Blend with video using screen blend
    vec3 blendedColor = 1.0 - (1.0 - videoColor.rgb) * (1.0 - fractalColor * patternStrength);
    
    // Add subtle glow
    float glow = exp(-pattern * 2.0) * 0.3;
    blendedColor += glow * fractalColor * patternStrength;
    
    // Final color
    FragColor = vec4(blendedColor, 1.0);
} 