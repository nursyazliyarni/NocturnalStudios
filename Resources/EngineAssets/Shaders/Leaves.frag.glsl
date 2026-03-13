#version 330 core

out vec4 FragColor;

uniform vec2 uResolution;
uniform float uTime;

float hash11(float p)
{
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash21(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

mat2 rot(float a)
{
    float c = cos(a);
    float s = sin(a);
    return mat2(c, -s, s, c);
}

// simple leaf SDF
float sdLeaf(vec2 p, vec2 size)
{
    p.x /= max(size.x, 0.0001);
    p.y /= max(size.y, 0.0001);

    float body = length(vec2(p.x * 0.75, p.y)) - 0.85;
    body += abs(p.x) * 0.22;
    body += p.y * p.y * 0.12;

    return body;
}

vec4 drawLeaf(vec2 uv, float seed)
{
    float t = uTime;

    float speed   = mix(0.10, 0.22, hash11(seed + 1.3));
    float scale   = mix(0.018, 0.040, hash11(seed + 2.7));
    float swayAmp = mix(0.05, 0.16, hash11(seed + 5.1));
    float spinAmp = mix(0.5, 1.6, hash11(seed + 7.4));
    float drift   = mix(-0.18, 0.18, hash11(seed + 9.8));

    float startX = mix(-0.95, 0.95, hash11(seed + 11.0));
    float offset = hash11(seed + 15.0) * 10.0;

    float y = 1.2 - mod(t * speed + offset, 2.4);
    float x = startX
            + sin(t * 0.9 + seed * 3.17) * swayAmp
            + cos(t * 0.55 + seed * 1.91) * swayAmp * 0.45
            + drift * (1.2 - y);

    vec2 pos = vec2(x, y);

    vec2 p = uv - pos;

    float angle = sin(t * spinAmp + seed * 6.2831) * 0.9
                + cos(t * 0.7 + seed * 4.2) * 0.35;

    p = rot(angle) * p;

    vec2 leafSize = vec2(scale * 0.72, scale * 1.35);
    float d = sdLeaf(p, leafSize);

    float fill = 1.0 - smoothstep(0.00, 0.02, d);
    float edge = 1.0 - smoothstep(0.015, 0.04, abs(d));

    float vein = smoothstep(0.018, 0.0, abs(p.x)) * fill * 0.35;

    vec3 c1 = vec3(0.78, 0.43, 0.12);
    vec3 c2 = vec3(0.47, 0.22, 0.05);
    vec3 c3 = vec3(0.92, 0.66, 0.18);

    float colorMix = hash11(seed + 20.0);
    vec3 leafColor = mix(c1, c2, colorMix);
    leafColor = mix(leafColor, c3, hash11(seed + 22.0) * 0.35);

    // soft lighting variation
    float shade = 0.75 + 0.25 * sin(seed * 12.7 + t * 1.3);
    leafColor *= shade;

    vec3 finalColor = leafColor;
    finalColor += vein * vec3(0.20, 0.10, 0.03);
    finalColor += edge * 0.08;

    float alpha = fill * 0.95;

    return vec4(finalColor, alpha);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / uResolution;
    uv = uv * 2.0 - 1.0;
    uv.x *= uResolution.x / uResolution.y;

    vec4 color = vec4(0.0);

    // fewer, clearer leaves
    const int LEAF_COUNT = 18;

    for (int i = 0; i < LEAF_COUNT; ++i)
    {
        float seed = float(i) * 13.137;
        vec4 leaf = drawLeaf(uv, seed);

        // alpha blend manually
        color.rgb = mix(color.rgb, leaf.rgb, leaf.a * (1.0 - color.a));
        color.a   = color.a + leaf.a * (1.0 - color.a);
    }

    FragColor = vec4(color.rgb, clamp(color.a, 0.0, 1.0));
}