#version 330 core

out vec4 FragColor;

uniform vec2 uResolution;
uniform float uTime;

float hash(float x)
{
    return fract(sin(x) * 43758.5453) * 2.0 - 1.0;
}

vec2 startPos(float x, vec2 pos)
{
    return vec2(
        pos.x + (cos(hash(x)) * 0.05),
        pos.y + (sin(hash(x * 1.4)) * 0.05)
    );
}

vec2 endPos(float x, vec2 pos)
{
    return vec2(
        pos.x + (cos(hash(x)) * 0.30),
        pos.y + (sin(hash(x * 1.7)) * 0.30)
    );
}

vec4 particles(vec2 uv, float timeOffset, float mx, float my, float speed)
{
    float px = mod(uv.x, 0.86) - 0.43;
    float py = mod(uv.y, 0.86) - 0.43;

    vec2 center = vec2(uv.x + px * mx, uv.y + py * my);

    float t = uTime + timeOffset;

    const float seqDuration = 2.0;
    float currentSeq = floor(t / seqDuration);
    float currentSeqTime = mod(t, seqDuration);

    vec2 startPosVal = startPos(currentSeq + ((uv.x - px) + (uv.y - py)), center);
    vec2 endPosVal   = endPos(currentSeq + ((uv.x - px) + (uv.y - py)), center);

    float endDist = length(endPosVal - startPosVal);
    float potentialDist = speed * currentSeqTime;

    vec2 currentPos = mix(
        startPosVal,
        endPosVal,
        clamp(potentialDist / max(endDist, 0.0001), 0.0, 1.0)
    );

    float distLength    = length(endPosVal - startPosVal);
    float distBrightOff = max(distLength * 0.2, 0.0001);
    float distCurrent   = length(endPosVal - currentPos);

    float trailAlpha = smoothstep(0.0, distBrightOff, distLength - distCurrent) *
                       smoothstep(0.0, distBrightOff, distCurrent);

    float d = length(uv - currentPos);

    // Smaller particle core + soft outer glow
    float coreRadius = 0.004;
    float glowRadius = 0.018;

    float core = 1.0 - smoothstep(coreRadius, coreRadius * 1.8, d);
    float glow = 1.0 - smoothstep(coreRadius * 0.6, glowRadius, d);

    vec3 coreColor = vec3(1.00, 0.95, 0.35); // warm yellow
    vec3 glowColor = vec3(1.00, 0.42, 0.08); // faint orange

    vec3 color = glowColor * glow * 0.35 +
                 coreColor * core * 1.15;

    float alpha = trailAlpha * (glow * 0.10 + core * 0.22);

    return vec4(color, alpha);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / uResolution;
    uv = uv * 2.0 - 1.0;
    uv.x *= uResolution.x / uResolution.y;

    vec4 color = vec4(0.0);

    color += particles(uv + vec2(25.0), 0.0,  1.0,  1.0, 0.20);
    color += particles(uv - vec2(20.0), 0.25, -1.0, -1.0, 0.08);
    color += particles(uv + vec2(20.0), 0.5,  -1.0,  1.0, 0.14);
    color += particles(uv - vec2(40.0), 0.75,  1.0, -1.0, 0.10);

    color += particles(uv + vec2(10.0), 1.75, -1.0,  1.0, 0.16);
    color += particles(uv - vec2(40.0), 1.0,  -1.0, -1.0, 0.11);
    color += particles(uv + vec2(30.0), 1.25, -1.0,  1.0, 0.09);
    color += particles(uv - vec2(30.0), 1.5,   1.0, -1.0, 0.13);

    color += particles(uv - vec2(10.0), 0.35,  1.0, -1.0, 0.16);
    color += particles(uv - vec2(15.0), 0.65,  1.0,  1.0, 0.13);
    color += particles(uv + vec2(15.0), 1.35,  1.0,  1.0, 0.11);
    color += particles(uv - vec2(25.0), 1.65,  1.0, -1.0, 0.09);

    fragColor = vec4(color.rgb, clamp(color.a, 0.0, 1.0));
}

void main()
{
    mainImage(FragColor, gl_FragCoord.xy);
}