#version 330

layout(location = 0) out vec4 out0;  // color

in vec2 uvc;

uniform sampler2D envMap;
uniform bool debugUV;

void main()
{
    out0 = vec4( debugUV ? vec3(uvc, 0) 
						 : texture2D(envMap, uvc).rgb
			   , 1);
}
