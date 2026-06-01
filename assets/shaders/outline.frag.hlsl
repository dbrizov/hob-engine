// Outline sprite fragment shader.
//
// Samples 8 neighbors in UV space; if the current texel is transparent but any
// neighbor is opaque, paint an outline color. Otherwise behaves like the default
// sprite shader (texture * tint).
//
// The outline width is a fixed UV offset, so it scales with the sprite's UV
// range — fine for a test shader; a production version would take texel size
// via a uniform.

Texture2D    sprite_tex  : register(t0, space2);
SamplerState sprite_samp : register(s0, space2);

cbuffer SpriteFS : register(b0, space3)
{
    float4 tint;
};

static const float OUTLINE_WIDTH = 0.02;
static const float ALPHA_THRESHOLD = 0.1;
static const float4 OUTLINE_COLOR = float4(1.0, 1.0, 1.0, 1.0);

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
    float4 center = sprite_tex.Sample(sprite_samp, uv);
    if (center.a > ALPHA_THRESHOLD)
    {
        return center * tint;
    }

    float2 offs[8] = {
        float2( OUTLINE_WIDTH, 0.0),
        float2(-OUTLINE_WIDTH, 0.0),
        float2(0.0,  OUTLINE_WIDTH),
        float2(0.0, -OUTLINE_WIDTH),
        float2( OUTLINE_WIDTH,  OUTLINE_WIDTH),
        float2( OUTLINE_WIDTH, -OUTLINE_WIDTH),
        float2(-OUTLINE_WIDTH,  OUTLINE_WIDTH),
        float2(-OUTLINE_WIDTH, -OUTLINE_WIDTH),
    };

    float max_neighbor_a = 0.0;
    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        max_neighbor_a = max(max_neighbor_a, sprite_tex.Sample(sprite_samp, uv + offs[i]).a);
    }

    if (max_neighbor_a > ALPHA_THRESHOLD)
    {
        return OUTLINE_COLOR;
    }

    return float4(0.0, 0.0, 0.0, 0.0);
}
