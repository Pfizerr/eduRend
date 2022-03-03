
Texture2D texDiffuse : register(t0);

cbuffer LightCamBuffer : register(b0)
{
	float4 Light_Pos : LIGHT_SOURCE_POSITION;
	float4 Camera_Pos : CAMERA_POSITION;
};

cbuffer PhongComponentBuffer : register(b0)
{

};


struct PSIn
{
	float4 Pos  : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEX;
};

//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1
	return float4(input.Normal*0.5+0.5, 1);

	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
//	return float4(input.TexCoord, 0, 1);
	//float3 L = PSIn.Pos - ;

	//return float(input.)
}