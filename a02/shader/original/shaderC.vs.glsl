#version 330

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 normal_view_space;
out vec3 position_view_space;
out mat3 view_to_world;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));

    normal_view_space = normalize(normalMatrix * vNormal.xyz);
    view_to_world     = transpose(mat3(viewMatrix));

    vec4 P              = viewMatrix * modelMatrix * vec4(vPosition, 1);
    position_view_space = P.xyz;

    gl_Position = projMatrix * P;
}
