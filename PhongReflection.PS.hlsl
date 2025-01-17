#include "Object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float4 color;
    int enableLighting;
    float shininess; // Added for specular reflection
    float4x4 uvTransform;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct CameraData
{
    float3 position; // Camera position in world space
};

ConstantBuffer<CameraData> gCamera : register(b2);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 worldNormal : NORMAL0;
    float3 worldPosition : POSITION1;
};

float3 CalculatePhongReflection(PixelShaderInput input)
{
    float3 normal = normalize(input.worldNormal);
    float3 lightDir = normalize(-gDirectionalLight.direction);
    float3 viewDir = normalize(gCamera.position - input.worldPosition);

    // Diffuse reflection
    float diffuseIntensity = saturate(dot(normal, lightDir));
    float3 diffuse = gDirectionalLight.color.rgb * diffuseIntensity * gDirectionalLight.intensity;

    // Specular reflection
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(saturate(dot(viewDir, reflectDir)), gMaterial.shininess);
    float3 specular = gDirectionalLight.color.rgb * spec * gDirectionalLight.intensity;

    // Combine reflections
    return diffuse + specular;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    // Sample texture
    float4 texColor = gTexture.Sample(gSampler, input.texcoord);

    // Calculate Phong reflection
    float3 lighting = CalculatePhongReflection(input);

    // Combine with material color
    float3 finalColor = texColor.rgb * gMaterial.color.rgb * lighting;

    return float4(finalColor, texColor.a);
}