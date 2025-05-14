#version 330

layout(location = 0) in vec3 vPosition;

out vec2 tc;

void main()
{
    gl_Position = vec4(vPosition, 1.0);
    tc          = vPosition.xy * vec2(0.5) + vec2(0.5);
}
