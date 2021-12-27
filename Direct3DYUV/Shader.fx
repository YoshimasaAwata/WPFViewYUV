//------------------------------------------------
// 定数
//------------------------------------------------
static const float4x4 TORGB = // 転置している事に注意
{
    { 1.164f,   1.164f,  1.164f,  0.0f},
    { 0.0f,   -0.392f, 2.017f, 0.0f},
    { 1.596f, -0.813f, 0.0f,   0.0f},
    { 0.0f,    0.0f,   0.0f,   1.0f}
};
static const float4 DIFF = { (16.0f / 255), (128.0f / 255), (128.0f / 255), 0.0f };
static const float4 MINYUV = { 0.0f,  (-112.0f / 255), (-112.0f / 255), 1.0f };
static const float4 MAXYUV = { (219.0f / 255), (112.0f / 255), (112.0f / 255), 1.0f };

//------------------------------------------------
// グローバル変数
//------------------------------------------------
float4x4    g_wvp;
texture     g_texy;
texture     g_texu;
texture     g_texv;

//------------------------------------------------
// テクスチャサンプラ
//------------------------------------------------
sampler TextureSamplerY =
sampler_state
{
    Texture = <g_texy>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};
sampler TextureSamplerU =
sampler_state
{
    Texture = <g_texu>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};
sampler TextureSamplerV =
sampler_state
{
    Texture = <g_texv>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

//------------------------------------------------
// 頂点シェーダ
//------------------------------------------------
void BasicVS(
    float3 in_pos : POSITION,
    float2 in_tex : TEXCOORD0,
    out float4 out_pos : POSITION,
    out float2 out_tex : TEXCOORD0)
{
    // 座標変換
    out_pos = mul(float4(in_pos, 1.0f), g_wvp);

    // テクスチャ座標はコピーするだけ
    out_tex = in_tex;

}

//------------------------------------------------
// ピクセルシェーダ
//------------------------------------------------
void BasicPS(
    float4 in_tex : TEXCOORD0,
    out float4 out_color : COLOR0)
{
    float4 fy = tex2D(TextureSamplerY, in_tex);
    float4 fu = tex2D(TextureSamplerU, in_tex);
    float4 fv = tex2D(TextureSamplerV, in_tex);
    float4 yuv = { fy.x, fu.x, fv.x, 1.0f };

    yuv -= DIFF;
    yuv = min(MAXYUV, yuv);
    yuv = max(MINYUV, yuv);
    out_color = mul(yuv, TORGB);
    out_color = saturate(out_color);
}

//------------------------------------------------
// テクニック宣言
//------------------------------------------------
technique BasicTech
{
    pass P0
    {
        vertexShader = compile vs_3_0 BasicVS();
        pixelShader = compile ps_3_0 BasicPS();
    }
}
