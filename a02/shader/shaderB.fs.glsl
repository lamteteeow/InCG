#version 330

layout(location = 0) out vec4 out0;  // color

in vec3 normal_view_space;
in vec3 position_view_space;
in mat3 view_to_world;

uniform sampler2D envMap;
uniform bool debugUV;

#pragma incg_include "spherical_coordinates.inc.glsl"

void main()
{
    vec3 N            = normalize(normal_view_space);
    vec2 lookup_coord = N.xy;

    // TODO: Calculate reflection vector in _view_ space
    // TODO: Transform reflection vector into _world_ space
    vec3 view_dir_view_space = normalize(position_view_space.xyz);
    vec3 reflect_dir = reflect(view_dir_view_space, normal_view_space);
    vec3 reflect_dir_world_space = view_to_world * reflect_dir;

    // TODO: transform reflection vector into polar texture coordinates (see shperical_coordinates.inc.glsl)
    lookup_coord = SphericalCoordinates(normalize(reflect_dir_world_space));

    out0 = vec4( debugUV ? vec3(lookup_coord, 0) 
						 : texture2D(envMap, lookup_coord).rgb
			   , 1);
}
