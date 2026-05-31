// Line vertex shader.
// Per-vertex pos (logical pixels) + color. Projection comes from a per-frame push constant.

cbuffer LineVS : register(b0, space1)
{
    float4x4 proj;
};

struct VSInput
{
    float2 pos   : TEXCOORD0;
    float4 color : TEXCOORD1;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color    : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput o;
    o.position = mul(proj, float4(input.pos, 0.0, 1.0));
    o.color = input.color;
    return o;
}
