#pragma once
#include "Renderer.h"
#include <fstream>

class CRectangleRenderer :
    public CRenderer
{
public:
    static const UINT CIF_WIDTH = 352;
    static const UINT CIF_HEIGHT = 288;

public:
    static HRESULT Create(IDirect3D9* pD3D, IDirect3D9Ex* pD3DEx, HWND hwnd, UINT uAdapter, CRenderer** ppRenderer);
    ~CRectangleRenderer();

    HRESULT Render();

    virtual void OpenFile(const char* pFileName);

protected:
    HRESULT Init(IDirect3D9* pD3D, IDirect3D9Ex* pD3DEx, HWND hwnd, UINT uAdapter);

    virtual void CloseFile();

    virtual HRESULT ReadYUV();

    virtual HRESULT YUV2Tex();

    virtual HRESULT EnsureTexture();

    virtual HRESULT EnsureEffect();

private:
    CRectangleRenderer();

    IDirect3DVertexBuffer9* m_pd3dVBPos;
    IDirect3DTexture9* m_pd3dTexY;
    IDirect3DTexture9* m_pd3dTexU;
    IDirect3DTexture9* m_pd3dTexV;
    ID3DXEffect* m_pFX;
    D3DXHANDLE m_hTech;
    D3DXHANDLE m_hWvp;
    D3DXHANDLE m_hTextureY;
    D3DXHANDLE m_hTextureU;
    D3DXHANDLE m_hTextureV;

    std::ifstream* m_pYUVFile;
    BYTE* m_pY;
    BYTE* m_pU;
    BYTE* m_pV;
};
