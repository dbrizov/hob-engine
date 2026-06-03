// Psychedelic sprite vertex shader.
// Identical to builtin/shaders/sprite.vert.hlsl — the pipeline VBO layout and
// SpriteVS cbuffer pushed by the renderer are fixed, so this just forwards
// the unit-quad position+UV through the standard ortho+rotate path.

cbuffer SpriteVS : register(b0, space1)
{
    float4x4 proj;
    float2 screen_pos;
    float2 size;
    float2 pivot;
    float rotation;
    float _pad;
};

struct VSInput
{
    float2 pos : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    float2 p = input.pos * size;
    float2 d = p - pivot;
    float c = cos(rotation);
    float s = sin(rotation);
    float2 r = float2(c * d.x - s * d.y, s * d.x + c * d.y);
    float2 screen = screen_pos + pivot + r;

    VSOutput o;
    o.pos = mul(proj, float4(screen, 0.0, 1.0));
    o.uv = input.uv;
    return o;
}
