// Blit vertex shader.
// Synthesizes a fullscreen triangle from SV_VertexID — no vertex buffer needed.
// The triangle's vertices are (-1,-1), (3,-1), (-1,3) in clip space; the on-screen
// portion forms the screen quad and the UVs sweep [0,1] across it.

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

VSOutput main(uint vid : SV_VertexID)
{
    VSOutput o;
    o.uv = float2((vid << 1) & 2, vid & 2);
    o.position = float4(o.uv * 2.0 - 1.0, 0.0, 1.0);
    return o;
}
