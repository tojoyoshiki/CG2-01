#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 wvp;
    float4x4 World;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 worldNormal : NORMAL0;
    float3 worldPosition : POSITION1; // Added for Phong Reflection
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // Transform position to world space
    float4 worldPosition = mul(input.position, gTransformationMatrix.World);
    output.worldPosition = worldPosition.xyz; // Pass world position to pixel shader

    // Transform position to clip space
    output.position = mul(input.position, gTransformationMatrix.wvp);

    // Pass other attributes
    output.texcoord = input.texcoord;
    output.worldNormal = mul(float4(input.normal, 0.0), gTransformationMatrix.World).xyz;

    return output;
}