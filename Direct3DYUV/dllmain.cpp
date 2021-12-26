// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

static CRendererManager* pManager[2] = { NULL, NULL };

static HRESULT EnsureRendererManager(int type)
{
    return pManager[type] ? S_OK : CRendererManager::Create(&pManager[type], static_cast<CRendererManager::RendererType>(type));
}

extern "C" HRESULT WINAPI SetSize(UINT uWidth, UINT uHeight, int type)
{
    HRESULT hr = S_OK;

    IFC(EnsureRendererManager(type));

    pManager[type]->SetSize(uWidth, uHeight);

Cleanup:
    return hr;
}

extern "C" HRESULT WINAPI SetAlpha(BOOL fUseAlpha, int type)
{
    HRESULT hr = S_OK;

    IFC(EnsureRendererManager(type));

    pManager[type]->SetAlpha(!!fUseAlpha);

Cleanup:
    return hr;
}

extern "C" HRESULT WINAPI SetNumDesiredSamples(UINT uNumSamples, int type)
{
    HRESULT hr = S_OK;

    IFC(EnsureRendererManager(type));

    pManager[type]->SetNumDesiredSamples(uNumSamples);

Cleanup:
    return hr;
}

extern "C" HRESULT WINAPI SetAdapter(POINT screenSpacePoint, int type)
{
    HRESULT hr = S_OK;

    IFC(EnsureRendererManager(type));

    pManager[type]->SetAdapter(screenSpacePoint);

Cleanup:
    return hr;
}

extern "C" HRESULT WINAPI GetBackBufferNoRef(IDirect3DSurface9 * *ppSurface, int type)
{
    HRESULT hr = S_OK;

    IFC(EnsureRendererManager(type));

    IFC(pManager[type]->GetBackBufferNoRef(ppSurface));

Cleanup:
    return hr;
}

extern "C" HRESULT WINAPI Render(int type)
{
    assert(pManager);

    return pManager[type]->Render();
}

extern "C" void WINAPI Destroy(int type)
{
    delete pManager[type];
    pManager[type] = NULL;
}
