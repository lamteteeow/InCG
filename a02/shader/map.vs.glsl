#version 330

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vUV;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 modelMatrix;

out vec2 uvc;

void main()
{
    uvc         = vUV;
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1);
}
