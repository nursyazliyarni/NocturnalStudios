#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uTime;

float random(float x)
{
    return fract(sin(x) * 10000.0);
}

float noise(vec2 p)
{
    return random(p.x + p.y * 10000.0);
}

vec2 sw(vec2 p) { return vec2(floor(p.x), floor(p.y)); }
vec2 se(vec2 p) { return vec2(ceil(p.x),  floor(p.y)); }
vec2 nw(vec2 p) { return vec2(floor(p.x), ceil(p.y)); }
vec2 ne(vec2 p) { return vec2(ceil(p.x),  ceil(p.y)); }

float smoothNoise(vec2 p)
{
    vec2 interp = smoothstep(0.0, 1.0, fract(p));
    float s = mix(noise(sw(p)), noise(se(p)), interp.x);
    float n = mix(noise(nw(p)), noise(ne(p)), interp.x);
    return mix(s, n, interp.y);
}

float fractalNoise(vec2 p)
{
    float x = 0.0;
    x += smoothNoise(p);
    x += smoothNoise(p * 2.0) / 2.0;
    x += smoothNoise(p * 4.0) / 4.0;
    x += smoothNoise(p * 8.0) / 8.0;
    x += smoothNoise(p * 16.0) / 16.0;
    x /= 1.0 + 0.5 + 0.25 + 0.125 + 0.0625;
    return x;
}

float movingNoise(vec2 p)
{
    float x = fractalNoise(p + uTime * 0.25);
    float y = fractalNoise(p - uTime * 0.25);
    return fractalNoise(p + vec2(x, y));
}

float nestedNoise(vec2 p)
{
    float x = movingNoise(p);
    float y = movingNoise(p + 100.0);
    return movingNoise(p + vec2(x, y));
}

void main()
{
    vec4 base = texture(uTexture, vTexCoord);

    // keep sprite transparency
    if (base.a < 0.01)
        discard;

    vec2 uv = vTexCoord;

    // animate water in local sprite UV space
    float n = nestedNoise(uv * 6.0);

    // soft horizontal distortion for wave shimmer
    float distortion = (n - 0.5) * 0.03;
    vec2 warpedUV = uv + vec2(distortion, 0.0);

    vec4 texCol = texture(uTexture, warpedUV);

    vec3 shallowCol = vec3(0.40, 0.65, 1.00);
    vec3 deepCol = vec3(0.08, 0.20, 0.85);

    vec3 waterTint = mix(shallowCol, deepCol, n);

    // mix sprite texture with procedural water color
    vec3 finalRGB = mix(texCol.rgb, waterTint, 0.55);

    // tiny highlight shimmer
    float highlight = smoothstep(0.72, 1.0, n) * 0.18;
    finalRGB += vec3(highlight);

    FragColor = vec4(finalRGB, texCol.a);
}