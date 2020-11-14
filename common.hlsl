//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
  matrix World;
  matrix View;
  matrix Projection;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT {
  float4 Pos   : POSITION;
  float2 Tex   : TEXCOORD0;
  float4 Color : COLOR;
};

struct v2f {
  float4 Pos   : SV_POSITION;
  float2 Tex   : TEXCOORD0;
  float4 Color : COLOR;
};
