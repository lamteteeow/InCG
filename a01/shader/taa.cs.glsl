#version 450

#define gid gl_GlobalInvocationID.xy
layout(local_size_x = 32, local_size_y = 32) in;
layout(binding = 0) uniform sampler2D currFrame;
layout(binding = 1) uniform sampler2D history;
layout(binding = 2, rgba32f) uniform image2D taaOut;

uniform bool init;
uniform uvec2 res;

uniform float[9] weights;
uniform mat4 reProj;

uniform bool freeze;
uniform bool doFilter;
uniform bool doClamp;
uniform bool doDynamicFeedback;
uniform float maxFeedback;

const ivec2 offsets[9] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
float luma(vec3 rgb)
{
    return 0.299 * rgb.r + 0.578 * rgb.g + 0.144 * rgb.b;
}

void main()
{
    if (gid.x >= res.x || gid.y >= res.y) return;

    // get filtered sample and color neighborhood
    vec4 color       = vec4(0);
    vec3 colorBoxMax = vec3(-1.0 / 0.0);  // -infinity
    vec3 colorBoxMin = vec3(1.0 / 0.0);   // +infinity

    for (int i = 0; i < 9; ++i)
    {
        // TODO b) accumulate weighed 3x3 samples from current frame
        vec4 sampleColor = texelFetch(currFrame, ivec2(gid) + offsets[i], 0);
        // weights are already normalized
        color += sampleColor * weights[i];

        // TODO d) calculate absolute 3x3 neighborhood min/max and clamp history sample
        colorBoxMin = min(colorBoxMin, sampleColor.rgb);
        colorBoxMax = max(colorBoxMax, sampleColor.rgb);
    }

    // if !doFilter use unweighted center pixel
    if (!doFilter) color = texelFetch(currFrame, ivec2(gid) + offsets[4], 0);

    // reprojection
    vec2 tex_coord = (vec2(gid) + 0.5) / res;
    // here, depth is between 0 and 1
    float depth = texelFetch(currFrame, ivec2(gid), 0).a;

    vec2 prev_tex_coord = tex_coord;
    // TODO c) reproject current screen space texture coordinates to coordinates from previous frame
    vec4 currNDC = vec4(tex_coord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0); // NDC: [-1,1]
    vec4 prevNDC = reProj * currNDC;
    prevNDC /= prevNDC.w; // Perspective divide

    // Convert back to texture coordinates [0,1]
    prev_tex_coord = prevNDC.xy * 0.5 + 0.5;

    // no history available (init) or out-of-bounds access
    vec4 historyColor = texture(history, prev_tex_coord);
    if (init || prev_tex_coord != clamp(prev_tex_coord, 0, 1)) historyColor.xyz = color.xyz;

    // history clamping
    if (doClamp)
    {
        // TODO d) clamp history to min/max
        historyColor.rgb = clamp(historyColor.rgb, colorBoxMin, colorBoxMax);
    }

    float feedback = maxFeedback;
    // dynamic feedback
    if (doDynamicFeedback)
    {
        // TODO bonus: adjust feedback depending on luma/velocity
        // Calculate velocity - difference between current and reprojected position
        vec2 velocity = abs(tex_coord - prev_tex_coord) * res;
        float velocityMagnitude = length(velocity);
        
        // Calculate luma difference between current and history
        float lumaDiff = abs(luma(color.rgb) - luma(historyColor.rgb));
        
        // Reduce feedback based on velocity and luma difference
        float velocityFactor = clamp(1.0 - velocityMagnitude * 0.5, 0.0, 1.0);
        float lumaFactor = clamp(1.0 - lumaDiff * 5.0, 0.0, 1.0);
        
        // Combine factors (using minimum for conservative approach)
        feedback = maxFeedback * min(velocityFactor, lumaFactor);
    }

    // TODO a) mix current frame with previous frames (feedback)
    vec4 result = mix(color, historyColor, feedback);
    imageStore(taaOut, ivec2(gid), result);
//    imageStore(taaOut, ivec2(gid), color);


        //////////////////////////////////////////////
    if (freeze) imageStore(taaOut, ivec2(gid), texelFetch(history, ivec2(gid), 0));
}
