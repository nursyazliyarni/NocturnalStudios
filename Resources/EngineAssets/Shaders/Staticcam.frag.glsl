#version 330 core

out vec4 FragColor;

uniform vec2 uResolution;
uniform float uTime;

float staticNoise(vec2 pos, float evolve)
{
    float e = fract(evolve * 0.01);

    float cx = pos.x * e;
    float cy = pos.y * e;

    cy = max(abs(cy), 0.0001);

    return fract(
        23.0 * fract(
            2.0 / max(
                fract(
                    fract(cx * 2.4 / (cy * 23.0)) *
                    fract(cx * max(evolve, 0.0001) / pow(cy, 0.05))
                ),
                0.0001
            )
        )
    );
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;

    // base noisy static
    float n = staticNoise(fragCoord, uTime * 60.0);

    vec3 colour = vec3(n);

    // tiny flicker
    float flicker = 0.92 + 0.08 * sin(uTime * 35.0);
    colour *= flicker;

    // faint scanline feel
    float scan = 0.97 + 0.03 * sin(fragCoord.y * 1.5);
    colour *= scan;

    // alpha less than 1 so it works like an overlay
    float alpha = 0.28;

    FragColor = vec4(colour, alpha);
}