#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D equirectangularMap;
uniform mat4 inverseProjection;
uniform mat4 inverseView;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 ndc = TexCoords * 2.0 - 1.0;
    vec4 clip = vec4(ndc, 1.0, 1.0);
    vec4 viewPos = inverseProjection * clip;
    viewPos /= viewPos.w;

    vec3 worldDir = normalize((inverseView * vec4(viewPos.xyz, 0.0)).xyz);
    vec2 uv = SampleSphericalMap(worldDir);
    vec3 color = texture(equirectangularMap, uv).rgb;
    color = vec3(1.0) - exp(-color * 1.35);
    color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}
