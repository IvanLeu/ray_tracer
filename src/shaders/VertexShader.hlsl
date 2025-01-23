struct PSOut
{
    float2 tc : TEXCOORD;
    float4 position : SV_Position;
};

PSOut main(float3 pos : POSITION, float2 tc : TEXCOORD)
{
    PSOut psOut;
    psOut.position = float4(pos, 1.0f);
    psOut.tc = tc;
    
    return psOut;
}