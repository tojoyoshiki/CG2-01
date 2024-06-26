
struct VertexShaderInput
{
    float32_t4 position : POSOTION0;
    float32_t2 texcoord : TEXCOORD0;
};

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};
