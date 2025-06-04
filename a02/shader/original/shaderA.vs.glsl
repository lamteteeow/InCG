#version 330

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

#pragma incg_include "spherical_coordinates.inc.glsl"

out vec2 lookup_coord;
void main()
{
    // transforms model- to view-normals:
    mat3 normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));

    // normal_view_space, pos_view_space is in Camera-space
    vec3 normal_view_space   = normalize(normalMatrix * vNormal.xyz);
    vec4 position_view_space = viewMatrix * modelMatrix * vec4(vPosition, 1);

    gl_Position  = projMatrix * position_view_space;
    lookup_coord = normal_view_space.xy;

    mat3 view_to_world = inverse(mat3(viewMatrix));

    // TODO: Calculate reflection vector in _view_ space
    // TODO: Transform reflection vector into _world_ space












    // TODO: transform reflection vector into polar texture coordinates (see shperical_coordinates.inc.glsl)

}
