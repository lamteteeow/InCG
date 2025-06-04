#version 330

layout(location = 0) out vec4 out0;  // color

in vec3 normal_view_space;
in vec3 position_view_space;
in mat3 view_to_world;

uniform sampler2D envMap;
uniform bool debugUV;

#pragma incg_include "spherical_coordinates.inc.glsl"

// uniform distribution in [0 .. 1], pretty good numbers,
// inspired by http://web.archive.org/web/20080211204527/http://lumina.sourceforge.net/Tutorials/Noise.html
// init: A freely choosen, constant seed for the distribution
// index: The index, which number of the distribution to calculate (0,1,2,...)
float random_uniform(vec2 init, int index)
{
    int x = int(1048576.0 * init.x);
    int y = int(1048576.0 * init.y);
    int n = x + y * 789221 + index;
    n     = (n << 13) ^ n;
    return 1.0 - 0.5 * float((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0;
}

vec2 reflection(vec3 facet_normal_view_space)
{
    vec2 lookup_coord = facet_normal_view_space.xy;
    // TODO: Calculate reflection vector in _view_ space
    // TODO: Transform reflection vector into _world_ space







    // TODO: transform reflection vector into polar texture coordinates (see shperical_coordinates.inc.glsl)

    return lookup_coord;
}


uniform float roughness;
uniform int glossyRays;


void main()
{
    vec3 N          = normalize(normal_view_space);
    vec3 refl_color = vec3(0, 0, 0);

    // TODO: Take glossyRays reflected samples, note the texture() call below.
    // The bias is used to get accurate samples and to bypass the texture mipmap level (wich otherwise results in the
    // brown visible pixel-seam)
    vec2 lookup_coord = reflection(N);
    refl_color += debugUV ? vec3(lookup_coord,0) : texture(envMap, lookup_coord, -30).rgb;













































    out0 = vec4(refl_color.xyz, 1.0);
}
