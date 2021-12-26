#pragma once
#include "Renderer.h"

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

protected:
    HRESULT Init(IDirect3D9* pD3D, IDirect3D9Ex* pD3DEx, HWND hwnd, UINT uAdapter);

private:
    CRectangleRenderer();

    IDirect3DVertexBuffer9* m_pd3dVBPos;
};
