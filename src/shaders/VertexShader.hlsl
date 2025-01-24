struct PSOut
{
    float2 tc : TEXCOORD;
    float4 position : SV_Position;
};

struct Proj
{
    matrix proj;
};

ConstantBuffer<Proj> buf : register(b0);

PSOut main(float3 pos : POSITION, float2 tc : TEXCOORD)
{
    PSOut psOut;
    psOut.position = mul(buf.proj, float4(pos, 1.0f));
    psOut.tc = float2(tc.x, 1.0f - tc.y);
    
    return psOut;
}