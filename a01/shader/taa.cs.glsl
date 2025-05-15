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
    if (gid.x == 0 || gid.y == 0) return; // add lower bound

    // get filtered sample and color neighborhood
    vec4 color       = vec4(0);
    vec3 colorBoxMax = vec3(-1.0 / 0.0);  // -infinity
    vec3 colorBoxMin = vec3(1.0 / 0.0);   // +infinity

    for (int i = 0; i < 9; i++)
    {
        // TODO b) accumulate weighed 3x3 samples from current frame
        // bound checked at line 29 and 30
        vec4 sampleColor = texelFetch(currFrame, ivec2(gid) + offsets[i], 0);
        // weights are already normalized
        color += sampleColor * weights[i];

        // TODO d) calculate absolute 3x3 neighborhood min/max and clamp history sample
        colorBoxMin = min(colorBoxMin, sampleColor.rgb);
        colorBoxMax = max(colorBoxMax, sampleColor.rgb);
    }

    // Make sure color is less than or equal to 1 after accumulation
    color = min(color, vec4(1));

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
        // Calculate velocity (in pixels)
        vec2 velocity = abs(tex_coord - prev_tex_coord) * res;
        float velocityMagnitude = length(velocity);

        // Calculate luma difference between current and history
        float lumaDiff = abs(luma(color.rgb) - luma(historyColor.rgb));

        // Parameters for tuning
        float velocityThreshold = 1.0; // pixels, adjust as needed
        float lumaThreshold = 0.1;     // adjust as needed

        // Feedback decreases smoothly as velocity or luma difference increases
        float velocityFactor = 1.0 - smoothstep(0.0, velocityThreshold, velocityMagnitude);
        float lumaFactor = 1.0 - smoothstep(0.0, lumaThreshold, lumaDiff);

        // Combine factors (conservative: take minimum)
//        float adaptiveFactor = min(velocityFactor, lumaFactor);
        float adaptiveFactor = velocityFactor * lumaFactor;

        // Map to feedback range [minFeedback, maxFeedback]
        float minFeedback = 0.7;

        feedback = mix(minFeedback, maxFeedback, adaptiveFactor);
    }

    // TODO a) mix current frame with previous frames (feedback)
    vec4 result = mix(color, historyColor, feedback);
    imageStore(taaOut, ivec2(gid), result);
//    imageStore(taaOut, ivec2(gid), color);


        //////////////////////////////////////////////
    if (freeze) imageStore(taaOut, ivec2(gid), texelFetch(history, ivec2(gid), 0));
}