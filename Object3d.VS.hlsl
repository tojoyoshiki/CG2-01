#include "Object3d.hlsli"

struct Vector4
{
    float x;
    float y;
    float z;
    float w;
};

struct Matrix4x4
{
    float m[4][4];
};


struct TransformationMatrix
{
    float4x4 wvp;
    float4x4 World;
};

struct ParticleForGPU
{
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
//StructuredBuffer<ParticleForGPU> gTransformationMatrices: register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.wvp);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.World));
    return output;
}

