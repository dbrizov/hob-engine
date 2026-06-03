// Psychedelic sprite fragment shader.
//
// Three effects stacked on top of each other:
//   1. Wavy UV distortion       — sine ripples on both axes, "underwater" warp.
//   2. Radial chromatic aberration — R/G/B sampled at offset UVs growing toward
//      the edges, like a cheap VHS / lens-fringe look.
//   3. Per-column hue rotation   — the hue of every pixel is rotated around the
//      luminance axis by an angle proportional to UV.x, giving the sprite a
//      vertical rainbow band.
//
// Cbuffer layout must match SpriteFS in the builtin sprite shader (48 bytes) so
// the renderer's SDL_PushGPUFragmentUniformData call lines up. tint is honored;
// outline_color/outline_width/alpha_threshold are reused — alpha_threshold still
// gates the silhouette so transparent pixels stay transparent.

Texture2D    sprite_tex  : register(t0, space2);
SamplerState sprite_samp : register(s0, space2);

// Cbuffer must match the builtin sprite shader's SpriteFS layout (64 bytes) so
// the renderer's blind PushGPUFragmentUniformData lines up — but this shader
// only consumes `tint`, `alpha_threshold`, `texel_size`, and `time`. The
// outline_* fields are irrelevant here.
cbuffer SpriteFS : register(b0, space3)
{
    float4 tint;
    float4 _outline_color_unused;
    float  _outline_width_unused;
    float  alpha_threshold;
    float2 texel_size;
    float  time;          // seconds since play start, refreshed each frame
    float3 _pad;
};

// Rotate an RGB color around the (1,1,1)/sqrt(3) luminance axis by `angle` rad.
float3 hue_shift(float3 rgb, float angle)
{
    const float3 k = float3(0.57735, 0.57735, 0.57735); // normalize(1,1,1)
    float ca = cos(angle);
    float sa = sin(angle);
    return rgb * ca + cross(k, rgb) * sa + k * dot(k, rgb) * (1.0 - ca);
}

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
    // --- 1. Wavy UV warp -----------------------------------------------------
    // Amplitude is measured in source texels so the wobble looks the same on
    // small and large sprites. Sine phase advances with time so the warp ripples.
    float2 warp = float2(
        sin(uv.y * 25.0 + time * 4.0) * texel_size.x * 3.0,
        sin(uv.x * 20.0 + time * 3.0) * texel_size.y * 3.0
    );
    float2 wuv = uv + warp;

    // Silhouette gate — keep the original sprite shape, don't fringe into the
    // transparent border.
    float center_a = sprite_tex.Sample(sprite_samp, wuv).a;
    if (center_a <= alpha_threshold)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    // --- 2. Radial chromatic aberration --------------------------------------
    float2 from_center = wuv - 0.5;
    // Aberration grows with distance from sprite center and breathes with time.
    float ab_strength = 0.06 + 0.04 * sin(time * 2.0);
    float2 ab = from_center * length(from_center) * ab_strength;
    float r = sprite_tex.Sample(sprite_samp, wuv + ab).r;
    float g = sprite_tex.Sample(sprite_samp, wuv).g;
    float b = sprite_tex.Sample(sprite_samp, wuv - ab).b;
    float3 rgb = float3(r, g, b);

    // --- 3. Per-column hue rotation ------------------------------------------
    // Full 2*pi sweep across the sprite plus a time offset, so the rainbow
    // scrolls horizontally over the sprite.
    rgb = hue_shift(rgb, wuv.x * 6.28318 + time * 2.0);

    return float4(rgb, center_a) * tint;
}
