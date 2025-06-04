#version 330

layout(location = 0) out vec4 out0;  // color

in vec3 normal_view_space;
in vec3 position_view_space;
in mat3 view_to_world;

uniform sampler2D envMap;
uniform bool debugUV;
//const float PI = 3.14159265358979323846;
const float EPSILON = 1e-5;

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
    vec3 view_dir_view_space = normalize(position_view_space);
    vec3 reflect_dir = reflect(view_dir_view_space, facet_normal_view_space);
    // TODO: Transform reflection vector into _world_ space
    vec3 reflect_dir_world_space = view_to_world * reflect_dir;

    // TODO: transform reflection vector into polar texture coordinates (see shperical_coordinates.inc.glsl)
    lookup_coord = SphericalCoordinates(normalize(reflect_dir_world_space));

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
    
    // Create tangent space around normal
    vec3 tangent, bitangent;
    if (abs(N.x) > abs(N.y))
        tangent = normalize(vec3(N.z, 0, -N.x));
    else
        tangent = normalize(vec3(0, -N.z, N.y));

    bitangent = cross(N, tangent);

    // Take multiple rays for glossy reflections using micro-facet model
    for (int i = 0; i < glossyRays; i++)
    {
        vec3 perturbed_normal = N;  // reset normal to original

        // Add normally distributed perturbation based on roughness
        if (roughness > 0.0 && i > 0)
        {
            // Generate uniform random values with 2 times the index to get two independent values
            vec2 random_seed = vec2(gl_FragCoord.xy) / 1024.0;
            float u1         = max(EPSILON, random_uniform(random_seed, i * 2));  // Avoid log(0)
            float u2         = random_uniform(random_seed, i * 2 + 1);

            // Box-Muller basic form
            // z1 and z2 are independent random variables with a standard normal distribution.
            float z1 = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
            float z2 = sqrt(-2.0 * log(u1)) * sin(2.0 * PI * u2);

            // Scale by roughness (standard deviation)
            float x = roughness * z1;
            float y = roughness * z2;

            // Apply perturbation in tangent space (offset the tip)
            perturbed_normal = normalize(N + x * tangent + y * bitangent);
        }

        vec2 lookup_coord = reflection(perturbed_normal);
        refl_color += debugUV ? vec3(lookup_coord, 0) : texture(envMap, lookup_coord, -30).rgb;
    }

    // Average the results
    refl_color /= float(glossyRays);

    out0 = vec4(refl_color.xyz, 1.0);
}
