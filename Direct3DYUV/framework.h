#pragma once

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
// Windows ヘッダー ファイル
#include <windows.h>

#include <d3d9.h>
#include <d3dx9.h>

#include <assert.h>

#include "RendererManager.h"
#include "Renderer.h"
#include "TriangleRenderer.h"

#define IFC(x) { hr = (x); if (FAILED(hr)) goto Cleanup; }
#define IFCOOM(x) { if ((x) == NULL) { hr = E_OUTOFMEMORY; IFC(hr); } }
#define SAFE_RELEASE(x) { if (x) { x->Release(); x = NULL; } }
