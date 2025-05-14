#version 330

layout(location = 0) out vec4 out0;  // color

in vec2 tc;

uniform sampler2D tex;
uniform int zoom = 1;

void main()
{
    vec2 TC = vec2( (tc-0.5) * (1. /  zoom) + 0.5 );
    out0     = vec4(texelFetch(tex, ivec2(TC * textureSize(tex, 0)), 0).rgb, 1.0);
}
