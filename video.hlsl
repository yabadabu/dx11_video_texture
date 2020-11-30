#include "common.hlsl"

//--------------------------------------------------------------------------------------
Texture2D    txYUV        : register(t0);
SamplerState samLinear    : register(s0);

//--------------------------------------------------------------------------------------
// Nothing special here
v2f VS(VS_INPUT input) {
  v2f output;
  output.Pos = mul(input.Pos, World);
  output.Pos = mul(output.Pos, View);
  output.Pos = mul(output.Pos, Projection);
  output.Tex = input.Tex;
  output.Color = input.Color;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float3 YUVToRGB(float Luma, float2 Chroma)
{
  min16float OneHalf = (min16float)  0.5f;
  min16float One = (min16float)  1.0f;
  min16float Zero = (min16float)  0.0f;
  min16float RVCoeff = (min16float)  1.402f;
  min16float GUCoeff = (min16float) - 0.344f;
  min16float GVCoeff = (min16float) - 0.714f;
  min16float BUCoeff = (min16float)  1.772f;

  min16float3 RCoeffs = min16float3(One, Zero, RVCoeff);
  min16float3 GCoeffs = min16float3(One, GUCoeff, GVCoeff);
  min16float3 BCoeffs = min16float3(One, BUCoeff, Zero);

  min16float3 Yuv = min16float3(Luma, Chroma - OneHalf);
  min16float R = dot(Yuv, RCoeffs);
  min16float G = dot(Yuv, GCoeffs);
  min16float B = dot(Yuv, BCoeffs);

  return min16float3(R, G, B);
}

float4 PS(v2f input) : SV_Target {
  float y = txYUV.Sample(samLinear, float2(input.Tex.x, input.Tex.y * 0.5)).r;
  float u = txYUV.Sample(samLinear, float2(input.Tex.x*0.5, 0.50 + input.Tex.y * 0.25)).r;
  float v = txYUV.Sample(samLinear, float2(input.Tex.x*0.5, 0.75 + input.Tex.y * 0.25)).r;
  return float4(YUVToRGB(y, float2(u,v)), 1.f);
}