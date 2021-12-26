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
    m_pd3dTex(NULL),
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
    SAFE_RELEASE(m_pd3dTex);
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
    D3DXMATRIXA16 matView, matProj;
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

    IFC(m_pd3dDevice->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pd3dVBPos, NULL));

    void* pVertices;
    IFC(m_pd3dVBPos->Lock(0, sizeof(vertices), &pVertices, 0));
    memcpy(pVertices, vertices, sizeof(vertices));
    m_pd3dVBPos->Unlock();

    // Set up the camera
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    IFC(m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView));
    D3DXMatrixOrthoLH(&matProj, CIF_WIDTH, CIF_WIDTH, 1.0f, 100.0f);
    IFC(m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj));

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

    IFC(m_pd3dDevice->SetTexture(0, m_pd3dTex));
    IFC(m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2));

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
//      CRectangleRenderer::YUV2RGB
//
//  Synopsis:
//      Transform yuv data to rgb data and set to texture buffer
//
//------------------------------------------------------------------------------
void CRectangleRenderer::YUV2RGB(UINT* pRGB, UINT stride)
{
    const XMVECTOR DIFF = { (16.0f / 255), (128.0f / 255), (128.0f / 255), 0.0f };
    const XMVECTOR MIN  = { 0.0f, -0.5f, -0.5f, 1.0f };
    const XMVECTOR MAX  = { 1.0f, 0.5f, 0.5f, 1.0f };

    for (int h = 0; h < CIF_HEIGHT; h++)
    {
        int y_pos = h * CIF_WIDTH;
        int uv_pos = (h / 2) * (CIF_WIDTH / 2);
        int rgb_pos = h * stride;

        for (int w = 0; w < CIF_WIDTH; w++)
        {
            XMVECTOR yuvw =
            {
                static_cast<float>(m_pY[y_pos + w]),
                static_cast<float>(m_pU[uv_pos + (w / 2)]),
                static_cast<float>(m_pV[uv_pos + (w / 2)]),
                1.0f
            };
            yuvw /= 255.0f;
            yuvw -= DIFF;
            yuvw = XMVectorMin(yuvw, MAX);
            yuvw = XMVectorMax(yuvw, MIN);
            XMVECTOR rgba = XMColorYUVToRGB(yuvw);
            rgba = XMVectorSaturate(rgba);
            rgba *= 255.0f;
            rgba = XMVectorTruncate(rgba);
            UINT r = static_cast<UINT>(XMVectorGetX(rgba));
            UINT g = static_cast<UINT>(XMVectorGetY(rgba));
            UINT b = static_cast<UINT>(XMVectorGetZ(rgba));
            pRGB[rgb_pos + w] = (r << 16) | (g << 8) | b;
        }
    }
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

    if (nullptr == m_pd3dTex)
    {
        IFC(m_pd3dDevice->CreateTexture(
            CIF_WIDTH,
            CIF_HEIGHT,
            1,
            D3DUSAGE_DYNAMIC,
            D3DFMT_X8R8G8B8,
            D3DPOOL_DEFAULT,
            &m_pd3dTex,
            NULL
        ));
    }

    IFC(ReadYUV());

    D3DLOCKED_RECT locked;
    IFC(m_pd3dTex->LockRect(0, &locked, NULL, 0));
    YUV2RGB(static_cast<UINT*>(locked.pBits), locked.Pitch / sizeof(UINT));
    hr = m_pd3dTex->UnlockRect(0);

Cleanup:
    return hr;
}
