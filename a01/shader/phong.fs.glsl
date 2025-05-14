#version 450

layout(location = 0) out vec4 out0;  // color

in vec3 N;
in vec3 L;
in vec4 P;
in vec2 TC;

uniform bool pattern;
uniform vec3 color;

const float shinyness   = 32.0;
const vec3 ambient_col  = vec3(0.1, 0.1, 0.2);  // grey
const vec3 specular_col = vec3(0.4, 0.4, 0.4);  // white


vec4 blinn_phong_reflectance(vec3 N, vec3 L, vec3 V)
{
    vec3 H = normalize(L + V);  // half way vector

    vec3 amb = ambient_col;

    vec3 col = color;

    if (pattern)
    {
        vec2 modTC = mod(TC * 50 - 0.75f, 2.0);
        col *= 0.2f;
        amb = vec3(0.f);

        if (modTC.x < 0.5 || modTC.y < 0.5) col = color;
    }


    vec3 diff = max(0.0, dot(L, N)) * col;
    vec3 spec = pow(max(0.0, dot(H, N)), shinyness) * specular_col;

    return vec4(amb + diff + spec, 1.0);
}



void main()
{
    vec3 viewDir = normalize(-P.xyz);
    vec3 normal  = normalize(N);
    out0         = vec4(blinn_phong_reflectance(normal, L, viewDir).xyz, gl_FragCoord.z);
}
