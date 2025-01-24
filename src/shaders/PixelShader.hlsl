Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
    return tex.Sample(samp, tc);
}