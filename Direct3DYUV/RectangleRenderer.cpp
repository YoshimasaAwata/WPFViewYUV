#include "pch.h"
#include "RectangleRenderer.h"

using namespace DirectX;

//+-----------------------------------------------------------------------------
//
//  CRectangleRenderer
//
//      Subclass of CRenderer that renders a single, spinning triangle
//
//------------------------------------------------------------------------------

struct CUSTOMVERTEX
{
    FLOAT x, y, z;
    FLOAT tu, tv;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer ctor
//
//------------------------------------------------------------------------------
CRectangleRenderer::CRectangleRenderer() :
	CRenderer(),
	m_pd3dVBPos(NULL),
    m_pd3dTexY(NULL),
    m_pd3dTexU(NULL),
    m_pd3dTexV(NULL),
    m_pFX(NULL),
    m_hWvp(NULL),
    m_hTextureY(0),
    m_hTextureU(0),
    m_hTextureV(0),
    m_pYUVFile(nullptr),
    m_pY(nullptr),
    m_pU(nullptr),
    m_pV(nullptr)
{
    m_pY = new BYTE[CIF_WIDTH * CIF_HEIGHT];
    m_pU = new BYTE[(CIF_WIDTH / 2) * (CIF_HEIGHT / 2)];
    m_pV = new BYTE[(CIF_WIDTH / 2) * (CIF_HEIGHT / 2)];
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer dtor
//
//------------------------------------------------------------------------------
CRectangleRenderer::~CRectangleRenderer()
{
    CloseFile();
    SAFE_RELEASE(m_pd3dVBPos);
    SAFE_RELEASE(m_pFX);
    SAFE_RELEASE(m_pd3dTexY);
    SAFE_RELEASE(m_pd3dTexU);
    SAFE_RELEASE(m_pd3dTexV);
    delete[] m_pY;
    delete[] m_pU;
    delete[] m_pV;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::Create
//
//  Synopsis:
//      Creates the renderer
//
//------------------------------------------------------------------------------
HRESULT CRectangleRenderer::Create(IDirect3D9* pD3D, IDirect3D9Ex* pD3DEx, HWND hwnd, UINT uAdapter, CRenderer** ppRenderer)
{
    HRESULT hr = S_OK;

    CRectangleRenderer* pRenderer = new CRectangleRenderer();
    IFCOOM(pRenderer);

    IFC(pRenderer->Init(pD3D, pD3DEx, hwnd, uAdapter));

    *ppRenderer = pRenderer;
    pRenderer = NULL;

Cleanup:
    delete pRenderer;

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::Init
//
//  Synopsis:
//      Override of CRenderer::Init that calls base to create the device and 
//      then creates the CRectangleRenderer-specific resources
//
//------------------------------------------------------------------------------
HRESULT CRectangleRenderer::Init(IDirect3D9* pD3D, IDirect3D9Ex* pD3DEx, HWND hwnd, UINT uAdapter)
{
    HRESULT hr = S_OK;
    D3DXMATRIXA16 matView, matProj, mat;
    D3DXVECTOR3 vEyePt(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);

    // Set up the VB
    CUSTOMVERTEX vertices[] =
    {
        { -(CIF_WIDTH / 2.0f),  (CIF_HEIGHT / 2.0f), 0.0f, 0.0f, 0.0f, }, // x, y, z, tu, tv
        {  (CIF_WIDTH / 2.0f),  (CIF_HEIGHT / 2.0f), 0.0f, 1.0f, 0.0f, },
        {  (CIF_WIDTH / 2.0f), -(CIF_HEIGHT / 2.0f), 0.0f, 1.0f, 1.0f, },
        { -(CIF_WIDTH / 2.0f), -(CIF_HEIGHT / 2.0f), 0.0f, 0.0f, 1.0f, },
    };

    // Call base to create the device and render target
    IFC(CRenderer::Init(pD3D, pD3DEx, hwnd, uAdapter));

    IFC(EnsureEffect());

    IFC(m_pd3dDevice->CreateVertexBuffer(sizeof(vertices), D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pd3dVBPos, NULL));

    void* pVertices;
    IFC(m_pd3dVBPos->Lock(0, sizeof(vertices), &pVertices, 0));
    memcpy(pVertices, vertices, sizeof(vertices));
    m_pd3dVBPos->Unlock();

    // Set up the camera
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    D3DXMatrixOrthoLH(&matProj, CIF_WIDTH, CIF_WIDTH, 1.0f, 100.0f);

    // テクニックの設定（シェーダプログラムの設定）
    IFC(m_pFX->SetTechnique(m_hTech));

    // シェーダーのグローバル変数の値の設定
    mat = matView * matProj;
    IFC(m_pFX->SetMatrix(m_hWvp, &mat));
    IFC(m_pFX->CommitChanges());

    // Set up the global state
    IFC(m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    IFC(m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
    IFC(m_pd3dDevice->SetStreamSource(0, m_pd3dVBPos, 0, sizeof(CUSTOMVERTEX)));
    IFC(m_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX));

Cleanup:
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::Render
//
//  Synopsis:
//      Renders the Rectangle screen
//
//------------------------------------------------------------------------------
HRESULT CRectangleRenderer::Render()
{
    HRESULT hr = S_OK;

    IFC(EnsureTexture());

    IFC(m_pd3dDevice->BeginScene());
    IFC(m_pd3dDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET,
        D3DCOLOR_ARGB(128, 0, 0, 128),  // NOTE: Premultiplied alpha!
        1.0f,
        0
    ));

    /* シェーダプログラムの開始宣言 */
    IFC(m_pFX->Begin(0, 0));
    IFC(m_pFX->BeginPass(0));

    IFC(m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2));

    /* シェーダプログラムの終了宣言 */
    IFC(m_pFX->EndPass());
    IFC(m_pFX->End());

    IFC(m_pd3dDevice->EndScene());

Cleanup:
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::OpenFile
//
//  Synopsis:
//      Open specified file
//
//------------------------------------------------------------------------------
void CRectangleRenderer::OpenFile(const char* pFileName)
{
    CloseFile();
    m_pYUVFile = new std::ifstream(pFileName, std::ifstream::in | std::ifstream::binary);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::CloseFile
//
//  Synopsis:
//      Close file if opened
//
//------------------------------------------------------------------------------
void CRectangleRenderer::CloseFile()
{
    if (nullptr != m_pYUVFile)
    {
        m_pYUVFile->close();
        delete m_pYUVFile;
        m_pYUVFile = nullptr;
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::ReadYUV
//
//  Synopsis:
//      Read yuv data from file
//
//------------------------------------------------------------------------------
HRESULT CRectangleRenderer::ReadYUV()
{
    HRESULT hr = S_FALSE;

    if ((nullptr != m_pYUVFile) && m_pYUVFile->good())
    {
        m_pYUVFile->read(reinterpret_cast<char*>(m_pY), sizeof(BYTE) * CIF_WIDTH * CIF_HEIGHT);
        m_pYUVFile->read(reinterpret_cast<char*>(m_pU), sizeof(BYTE) * (CIF_WIDTH / 2) * (CIF_HEIGHT / 2));
        m_pYUVFile->read(reinterpret_cast<char*>(m_pV), sizeof(BYTE) * (CIF_WIDTH / 2) * (CIF_HEIGHT / 2));
        if (m_pYUVFile->good())
        {
            hr = S_OK;
        }
        else
        {
            CloseFile();
        }
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::YUV2Tex
//
//  Synopsis:
//      Set yuv data to texture
//
//------------------------------------------------------------------------------
HRESULT CRectangleRenderer::YUV2Tex()
{
    HRESULT hr = S_OK;
    BYTE* pdst;
    BYTE* psrc;
    D3DLOCKED_RECT locked;
    IFC(ReadYUV());

    IFC(m_pd3dTexY->LockRect(0, &locked, NULL, 0));
    psrc = m_pY;
    pdst = static_cast<BYTE*>(locked.pBits);
    for (int h = 0; h < CIF_HEIGHT; h++)
    {
        memcpy(pdst, psrc, CIF_WIDTH);
        psrc += CIF_WIDTH;
        pdst += locked.Pitch;
    }
    IFC(m_pd3dTexY->UnlockRect(0));

    IFC(m_pd3dTexU->LockRect(0, &locked, NULL, 0));
    psrc = m_pU;
    pdst = static_cast<BYTE*>(locked.pBits);
    for (int h = 0; h < (CIF_HEIGHT / 2); h++)
    {
        memcpy(pdst, psrc, (CIF_WIDTH / 2));
        psrc += CIF_WIDTH / 2;
        pdst += locked.Pitch;
    }
    IFC(m_pd3dTexU->UnlockRect(0));

    IFC(m_pd3dTexV->LockRect(0, &locked, NULL, 0));
    psrc = m_pV;
    pdst = static_cast<BYTE*>(locked.pBits);
    for (int h = 0; h < (CIF_HEIGHT / 2); h++)
    {
        memcpy(pdst, psrc, (CIF_WIDTH / 2));
        psrc += CIF_WIDTH / 2;
        pdst += locked.Pitch;
    }
    IFC(m_pd3dTexV->UnlockRect(0));

Cleanup:
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::EnsureTexture
//
//  Synopsis:
//      Makes sure a texture exists if we need it
//
//------------------------------------------------------------------------------
HRESULT CRectangleRenderer::EnsureTexture()
{
    HRESULT hr = S_OK;

    if (nullptr == m_pd3dTexY)
    {
        IFC(m_pd3dDevice->CreateTexture(
            CIF_WIDTH,
            CIF_HEIGHT,
            1,
            D3DUSAGE_DYNAMIC,
            D3DFMT_L8,
            D3DPOOL_DEFAULT,
            &m_pd3dTexY,
            NULL
        ));
        IFC(m_pFX->SetTexture(m_hTextureY, m_pd3dTexY));
        IFC(m_pFX->CommitChanges());
    }
    if (nullptr == m_pd3dTexU)
    {
        IFC(m_pd3dDevice->CreateTexture(
            (CIF_WIDTH / 2),
            (CIF_HEIGHT / 2),
            1,
            D3DUSAGE_DYNAMIC,
            D3DFMT_L8,
            D3DPOOL_DEFAULT,
            &m_pd3dTexU,
            NULL
        ));
        IFC(m_pFX->SetTexture(m_hTextureU, m_pd3dTexU));
        IFC(m_pFX->CommitChanges());
    }
    if (nullptr == m_pd3dTexV)
    {
        IFC(m_pd3dDevice->CreateTexture(
            (CIF_WIDTH / 2),
            (CIF_HEIGHT / 2),
            1,
            D3DUSAGE_DYNAMIC,
            D3DFMT_L8,
            D3DPOOL_DEFAULT,
            &m_pd3dTexV,
            NULL
        ));
        IFC(m_pFX->SetTexture(m_hTextureV, m_pd3dTexV));
        IFC(m_pFX->CommitChanges());
    }

    IFC(YUV2Tex());

    //HRESULT hr = S_OK;

    //if (nullptr == m_pd3dTex)
    //{
    //    IFC(m_pd3dDevice->CreateTexture(
    //        CIF_WIDTH,
    //        CIF_HEIGHT,
    //        1,
    //        D3DUSAGE_DYNAMIC,
    //        D3DFMT_X8R8G8B8,
    //        D3DPOOL_DEFAULT,
    //        &m_pd3dTex,
    //        NULL
    //    ));
    //}

    //IFC(ReadYUV());

    //D3DLOCKED_RECT locked;
    //IFC(m_pd3dTex->LockRect(0, &locked, NULL, 0));
    //YUV2RGB(static_cast<UINT*>(locked.pBits), locked.Pitch / sizeof(UINT));
    //hr = m_pd3dTex->UnlockRect(0);

Cleanup:
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CRectangleRenderer::EnsureEffect
//
//  Synopsis:
//      Read and compile hlsl file and get handles for global variables
//
//------------------------------------------------------------------------------
HRESULT CRectangleRenderer::EnsureEffect()
{
    HRESULT hr = S_OK;
    ID3DXBuffer* perrors = NULL;
    IFC(D3DXCreateEffectFromFile(
        m_pd3dDevice,
        L"Shader.fx",
        NULL,
        NULL,
        D3DXSHADER_DEBUG,
        NULL,
        &m_pFX,
        &perrors));

    // シェーダプログラムへテクニックへのハンドルの取得
    m_hTech = m_pFX->GetTechniqueByName("BasicTech");

    // シェーダープログラムのグローバル変数のハンドルの取得
    m_hWvp = m_pFX->GetParameterByName(0, "g_wvp");
    m_hTextureY = m_pFX->GetParameterByName(0, "g_texy");
    m_hTextureU = m_pFX->GetParameterByName(0, "g_texu");
    m_hTextureV = m_pFX->GetParameterByName(0, "g_texv");

Cleanup:
    if ((hr != S_OK) && (perrors != NULL))
    {
        char* pdbcode = new char[perrors->GetBufferSize() + 1];
        memcpy_s(pdbcode, perrors->GetBufferSize() + 1, perrors->GetBufferPointer(), perrors->GetBufferSize());
        OutputDebugStringA(pdbcode);
        delete[] pdbcode;
    }
    return hr;
}
