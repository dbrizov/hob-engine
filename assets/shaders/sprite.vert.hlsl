// Sprite vertex shader.
//
// Vertex input: unit-quad position in [0,1] x [0,1] and matching UV (top-left origin).
// Per-draw uniform block holds the orthographic projection plus the sprite's screen-space
// transform: top-left in logical pixels, size in pixels, pivot in pixels (relative to top-left),
// and rotation in radians (CCW in screen space).

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
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
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
    o.position = mul(proj, float4(screen, 0.0, 1.0));
    o.uv = input.uv;
    return o;
}
