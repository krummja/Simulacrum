struct Input
{
    [[vk::location(0)]] float2 Position : TEXCOORD0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD1;
    [[vk::location(2)]] float4 Color : TEXCOORD2;
};

struct Output
{
    [[vk::location(0)]] float2 TexCoord : TEXCOORD0;
    [[vk::location(1)]] float4 FragColor : TEXCOORD1;
    float4 Position : SV_POSITION;
};

Output main(Input input)
{
    Output output;
    output.Position = float4(input.Position, 0.0f, 1.0f);
    output.TexCoord = input.TexCoord;
    output.FragColor = input.Color;
    return output;
}
