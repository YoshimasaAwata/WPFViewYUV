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

    virtual void YUV2RGB(UINT* pRGB, UINT stride);

    virtual HRESULT EnsureTexture();

private:
    CRectangleRenderer();

    IDirect3DVertexBuffer9* m_pd3dVBPos;
    IDirect3DTexture9* m_pd3dTex;

    std::ifstream* m_pYUVFile;
    BYTE* m_pY;
    BYTE* m_pU;
    BYTE* m_pV;
};
