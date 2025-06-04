#version 330

layout(location = 0) out vec4 out0;  // color

in vec2 lookup_coord;
uniform sampler2D envMap;
uniform bool debugUV;

void main()
{
	out0 = vec4( debugUV ? vec3(lookup_coord, 0) 
						 : texture2D(envMap, lookup_coord).rgb
			   , 1);
}
