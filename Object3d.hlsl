
struct VertexShaderInput
{
    float32__t4 position : POSOTION0;
    float32_t4 texcoord : TEXCOORD0;
};

struct VertexShaderOutput
{
    float32__t4 position : SV_Position;
    float32_t4 texcoord : TEXCOORD0;
};

output.texcoord = input.texcoord;