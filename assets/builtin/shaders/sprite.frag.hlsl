// Sprite fragment shader.
// Samples the bound texture and multiplies by the per-draw tint.

Texture2D    sprite_tex  : register(t0, space2);
SamplerState sprite_samp : register(s0, space2);

cbuffer SpriteFS : register(b0, space3)
{
    float4 tint;
};

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
    return sprite_tex.Sample(sprite_samp, uv) * tint;
}
