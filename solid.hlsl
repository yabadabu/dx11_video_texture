#include "common.hlsl"

//--------------------------------------------------------------------------------------
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
float4 PS(v2f input) : SV_Target{
    return input.Color;
}
