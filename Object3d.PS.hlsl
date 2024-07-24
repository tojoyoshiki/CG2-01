//float4 main() : SV_TARGET
//{
//    return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}

struct Material
{
    float4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);
struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    return output;
}