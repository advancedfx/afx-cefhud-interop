// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>

#include <shellapi.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#include "include/cef_sandbox_win.h"
#include "afx-cefhud-interop/simple_app.h"
#include "AfxInterop.h"

//#include <include/cef_client.h>

#include <map>

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <tchar.h>
#include <third_party/Detours/src/detours.h>
#include <tlhelp32.h>

#include <mutex>

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
//#define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library may not link successfully with all VS
// versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

HMODULE g_hD3d11Dll = NULL;
LONG g_detours_error = NO_ERROR;

typedef ULONG(STDMETHODCALLTYPE* AddReff_t)(IUnknown* This);
typedef ULONG(STDMETHODCALLTYPE* Release_t)(IUnknown* This);

ID3D11Device* g_pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11Query* pQuery = NULL;

ID3D11VertexShader* pVertexShader = NULL;
ID3D11PixelShader* pPixelShader = NULL;
ID3D11InputLayout* pInputLayout = NULL;
ID3D11Buffer* pVertexBuffer = NULL;

ID3D11Texture2D* pLastGoodTexture2d = NULL;


UINT vertex_stride = 5 * sizeof(float);
UINT vertex_offset = 0;
UINT vertex_count = 4;

struct TextureMapElem {
  UINT Width;
  UINT Height;
  HANDLE ShareHandle;
  HANDLE TempTextureShareHandle;
  ID3D11ShaderResourceView* TempTextureView;
  int ActiveBrowser;
  bool FirstClear = true;
  int FlushCount = 0;

  TextureMapElem(HANDLE shareHandle,
                 HANDLE tempTextureShareHandle,
                 ID3D11ShaderResourceView* tempTextureView,
                 UINT width,
                 UINT height,
                 int activeBrowser)
      : ShareHandle(shareHandle),
        TempTextureShareHandle(tempTextureShareHandle),
        TempTextureView(tempTextureView),
        Width(width),
        Height(height),
        ActiveBrowser(activeBrowser) {
    if (nullptr != TempTextureView)
      TempTextureView->AddRef();
  }

  ~TextureMapElem() {
    if (nullptr != TempTextureView)
      TempTextureView->Release();
    // if (ShareHandle != INVALID_HANDLE_VALUE)
    //  CloseHandle(ShareHandle);
    // if (TempTextureShareHandle != INVALID_HANDLE_VALUE)
    //  CloseHandle(TempTextureShareHandle);
  }
};


std::mutex g_TexturesMutex;
std::map<ID3D11Texture2D*, TextureMapElem> g_Textures;

const char* g_szVertexShaderCode =
    "//matrix World;\n"
    "//matrix View;\n"
    "//matrix Projection;\n"
    "\n"
    "struct VS_INPUT\n"
    "{\n"
    "	float3 pos : POSITION;\n"
    "	float2 t0 : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct VS_OUTPUT\n"
    "{\n"
    "	float4 pos : SV_POSITION;\n"
    "	float2 t0 : TEXCOORD0;\n"
    "};\n"
    "\n"
    "VS_OUTPUT main( const VS_INPUT i )\n"
    "{\n"
    "	VS_OUTPUT o;\n"
    "\n"
    "\n"
    "	o.pos = float4(i.pos, 1);\n"
    "	//o.pos = mul( o.pos, World );\n"
    "	//o.pos = mul( o.pos, View );\n"
    "	//o.pos = mul( o.pos, Projection );\n"
    "	o.pos /= o.pos.w;\n"
    "	o.t0 = i.t0;\n"
    "\n"
    "	return o;\n"
    "}\n";

const char* g_szPixelShaderCode =
    "sampler g_sTextureSampler : register( s0 );\n"
    "Texture2D<float4> g_mainTexture : register(t0);"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "	float4 pos : SV_POSITION;\n"
    "	float2 t0 : TEXCOORD0;\n"
    "};\n"
    "\n"
    "float4 main( PS_INPUT i ) : SV_TARGET\n"
    "{\n"
    "	return float4(g_mainTexture.Sample(g_sTextureSampler, i.t0).xyz, 1);\n"
    "}\n";

    D3D11_INPUT_ELEMENT_DESC g_InputElementDesc[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
     D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
     D3D11_INPUT_PER_VERTEX_DATA, 0}};

/* BUGGED DON'T USE.
bool AfxWaitForGPU(ID3D11DeviceContext* pCtx) {
  if(!g_pDevice) return false;

	bool bOk = false;

	if (g_pDevice && pQuery)
	{
		pCtx->End(pQuery);

		while (S_OK != pCtx->GetData(pQuery, NULL, 0, 0))
			;

		bOk = true;
	}

	return bOk;
}
*/

class CGpuPipeClient : public advancedfx::interop::CPipeClient 
{
public:
} g_GpuPipeClient;

std::mutex g_GpuPipeClientMutex;


typedef void(STDMETHODCALLTYPE* Flush_t)(ID3D11DeviceContext* This);

Flush_t g_Org_Flush = nullptr;


typedef void(STDMETHODCALLTYPE* ClearRenderTargetView_t)(
    ID3D11DeviceContext* This,
    /* [annotation] */
    _In_ ID3D11RenderTargetView* pRenderTargetView,
    /* [annotation] */
    _In_ const FLOAT ColorRGBA[4]);

ClearRenderTargetView_t g_Org_ClearRenderTargetView;

void STDMETHODCALLTYPE
My_ClearRenderTargetView(ID3D11DeviceContext* This,
                         /* [annotation] */
                         _In_ ID3D11RenderTargetView* pRenderTargetView,
                         /* [annotation] */
                         _In_ const FLOAT ColorRGBA[4]);

        
typedef void(STDMETHODCALLTYPE * OMSetRenderTargets_t)(
    ID3D11DeviceContext* This,
    /* [annotation] */
_In_range_(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) UINT NumViews,
/* [annotation] */
_In_reads_opt_(NumViews) ID3D11RenderTargetView* const* ppRenderTargetViews,
/* [annotation] */
_In_opt_ ID3D11DepthStencilView* pDepthStencilView);

OMSetRenderTargets_t g_Org_OMSetRenderTargets;

//int g_ClearCount = 0;

void STDMETHODCALLTYPE My_OMSetRenderTargets(
    ID3D11DeviceContext* This,
    /* [annotation] */
    _In_range_(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) UINT NumViews,
    /* [annotation] */
    _In_reads_opt_(NumViews) ID3D11RenderTargetView* const* ppRenderTargetViews,
    /* [annotation] */
    _In_opt_ ID3D11DepthStencilView* pDepthStencilView) {

  g_Org_OMSetRenderTargets(This, NumViews, ppRenderTargetViews,
                           pDepthStencilView);
}

void STDMETHODCALLTYPE My_Flush(ID3D11DeviceContext* This) {
  g_Org_Flush(This);

    ID3D11RenderTargetView* oldRenderTarget[1] = {nullptr};
    pContext->OMGetRenderTargets(1, &oldRenderTarget[0], NULL);

    if (oldRenderTarget[0]) {
      ID3D11Resource* resource = nullptr;

      oldRenderTarget[0]->GetResource(&resource);

      if (resource) {
        auto it = g_Textures.find((ID3D11Texture2D*)resource);
        if (it != g_Textures.end()) {
          it->second.FirstClear = true;
          ++it->second.FlushCount;

          // AfxWaitForGPU(This);

          try {
            g_GpuPipeClient.WriteInt32(
                (INT32)advancedfx::interop::HostGpuMessage::OnAfterRender);
            g_GpuPipeClient.WriteHandle(it->second.ShareHandle);
            g_GpuPipeClient.WriteInt32(it->second.ActiveBrowser);
            g_GpuPipeClient.Flush();

            g_GpuPipeClient.ReadBoolean();
          } catch (const std::exception& e) {
            DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                        << e.what();
          }
        }

        resource->Release();
      }
      oldRenderTarget[0]->Release();
    }
}

void STDMETHODCALLTYPE
My_ClearRenderTargetView(ID3D11DeviceContext* This,
                         /* [annotation] */
                         _In_ ID3D11RenderTargetView* pRenderTargetView,
                         /* [annotation] */
                         _In_ const FLOAT ColorRGBA[4]) {
  g_Org_ClearRenderTargetView(This, pRenderTargetView, ColorRGBA);

  if (pRenderTargetView) {
    ID3D11Resource* resource = nullptr;

    pRenderTargetView->GetResource(&resource);

    if (resource) {
      auto it = g_Textures.find((ID3D11Texture2D*)resource);
      if (it != g_Textures.end()) {

        /*
        FLOAT color[4] = {((g_ClearCount << 0) % 256) / 255.0f,
                          ((g_ClearCount << 8) % 256) / 255.0f,
                          ((g_ClearCount << 16) % 256) / 255.0f, 1.0f};
        g_Org_ClearRenderTargetView(pContext, pRenderTargetView, color);*/

        it->second.FirstClear = false;
        it->second.FlushCount = 0;

        if (pInputLayout && pVertexBuffer && pVertexShader && pPixelShader) {
          bool paintFromTempGameTexture = false;

          try {
            std::unique_lock<std::mutex> lock(g_GpuPipeClientMutex);

            g_GpuPipeClient.WriteInt32(
                (INT32)advancedfx::interop::HostGpuMessage::OnAfterClear);
            g_GpuPipeClient.WriteHandle(it->second.TempTextureShareHandle);
            g_GpuPipeClient.WriteInt32(it->second.ActiveBrowser);
            g_GpuPipeClient.Flush();

            paintFromTempGameTexture = g_GpuPipeClient.ReadBoolean();

          } catch (const std::exception& e) {
            DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                        << e.what();
            DebugBreak();
          }

          if (paintFromTempGameTexture) {
            D3D11_RENDER_TARGET_VIEW_DESC desc;
            pRenderTargetView->GetDesc(&desc);

            UINT oldNumViewPorts = 1;
            D3D11_VIEWPORT oldviewport;
            pContext->RSGetViewports(&oldNumViewPorts, &oldviewport);

            ID3D11RenderTargetView* oldRenderTarget[1] = {nullptr};
            pContext->OMGetRenderTargets(1, &oldRenderTarget[0], NULL);

            D3D11_PRIMITIVE_TOPOLOGY oldInputTopology;
            pContext->IAGetPrimitiveTopology(&oldInputTopology);

            ID3D11InputLayout* oldInputLayout = nullptr;
            pContext->IAGetInputLayout(&oldInputLayout);

            ID3D11Buffer* oldBuffers[1] = {nullptr};
            UINT oldStrides[1] = {0};
            UINT oldOffsets[1] = {0};
            pContext->IAGetVertexBuffers(0, 1, &oldBuffers[0], &oldStrides[0],
                                         &oldOffsets[0]);

            ID3D11VertexShader* oldVertexShader = nullptr;
            pContext->VSGetShader(&oldVertexShader, NULL, 0);

            ID3D11PixelShader* oldPixelShader = nullptr;
            pContext->PSGetShader(&oldPixelShader, NULL, 0);

            ID3D11ShaderResourceView* oldShaderResources[1] = {nullptr};
            pContext->PSGetShaderResources(0, 1, &oldShaderResources[0]);

            //

            D3D11_VIEWPORT viewport = {0.0f,
                                       0.0f,
                                       (FLOAT)(it->second.Width),
                                       (FLOAT)(it->second.Height),
                                       0.0f,
                                       1.0f};
            pContext->RSSetViewports(1, &viewport);

            /*
            D3D11_RECT rect = {0, 0, (LONG)it->second.Width * 3 / 4,
                               (LONG)it->second.Height};

            pContext->RSSetScissorRects(1, &rect);
            */

            ID3D11RenderTargetView* views[1] = {pRenderTargetView};
            g_Org_OMSetRenderTargets(This, 1, &views[0], NULL);

            pContext->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            pContext->IASetInputLayout(pInputLayout);
            pContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &vertex_stride,
                                         &vertex_offset);

            pContext->VSSetShader(pVertexShader, NULL, 0);
            pContext->PSSetShader(pPixelShader, NULL, 0);

            ID3D11ShaderResourceView* res[1] = {it->second.TempTextureView};

            pContext->PSSetShaderResources(0, 1, &res[0]);

            /*
            D3D11_RASTERIZER_DESC sd = {};
            ID3D11RasterizerState* state = nullptr;
            pContext->RSGetState(&state);
            if (state) {
              state->GetDesc(&sd);
              state->Release();
              state = nullptr;
            }

            BOOL oldScissorEnable = sd.ScissorEnable;
            sd.ScissorEnable = TRUE;
            g_pDevice->CreateRasterizerState(&sd, &state);
            pContext->RSSetState(state);
            state->Release();
            state = nullptr;*/

            //

            pContext->Draw(vertex_count, 0);

            /*
            sd.ScissorEnable = oldScissorEnable;
            g_pDevice->CreateRasterizerState(&sd, &state);
            pContext->RSSetState(state);
            state->Release();
            state = nullptr;
            */

            //

            pContext->PSSetShaderResources(0, 1, &oldShaderResources[0]);
            pContext->PSSetShader(oldPixelShader, NULL, 0);
            pContext->VSSetShader(oldVertexShader, NULL, 0);
            pContext->IASetVertexBuffers(0, 1, &oldBuffers[0], &oldStrides[0],
                                         &oldOffsets[0]);
            pContext->IASetInputLayout(oldInputLayout);
            pContext->IASetPrimitiveTopology(oldInputTopology);
            g_Org_OMSetRenderTargets(This, 1, &oldRenderTarget[0], NULL);
            if (1 <= oldNumViewPorts)
              pContext->RSSetViewports(1, &oldviewport);

            if (oldShaderResources[0])
              oldShaderResources[0]->Release();
            if (oldPixelShader)
              oldPixelShader->Release();
            if (oldVertexShader)
              oldVertexShader->Release();
            if (oldBuffers[0])
              oldBuffers[0]->Release();
            if (oldInputLayout)
              oldInputLayout->Release();
            if (oldRenderTarget[0])
              oldRenderTarget[0]->Release();
          }
        }
      }

      resource->Release();
    }
  }
}

HANDLE AfxInteropGetSharedHandle(ID3D11Resource* pD3D11Resource) {
  HANDLE result = INVALID_HANDLE_VALUE;

  if (pD3D11Resource) {
    IDXGIResource* dxgiResource;

    if (SUCCEEDED(pD3D11Resource->QueryInterface(__uuidof(IDXGIResource),
                                           (void**)&dxgiResource))) {
      if (FAILED(dxgiResource->GetSharedHandle(&result))) {
        result = INVALID_HANDLE_VALUE;
      }
      dxgiResource->Release();
    }
  }

  return result;
}

Release_t g_Old_ID3D11Texture2D_Release;

ULONG STDMETHODCALLTYPE My_ID3D11Texture2D_Release(ID3D11Texture2D* This) {

  ULONG count = g_Old_ID3D11Texture2D_Release(This);

  if (0 == count) {

    if (This == pLastGoodTexture2d)
      pLastGoodTexture2d = NULL;

    auto it = g_Textures.find(This);
    if(it != g_Textures.end())
    {
      try {
        std::unique_lock<std::mutex> lock(g_GpuPipeClientMutex);

        g_GpuPipeClient.WriteInt32(
            (INT32)advancedfx::interop::HostGpuMessage::ReleaseShareHandle);
        g_GpuPipeClient.WriteHandle(it->second.TempTextureShareHandle);
        g_GpuPipeClient.Flush();
        g_GpuPipeClient.WriteInt32(
            (INT32)advancedfx::interop::HostGpuMessage::ReleaseShareHandle);
        g_GpuPipeClient.WriteHandle(it->second.ShareHandle);
        g_GpuPipeClient.Flush();
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
        DebugBreak();
      }

      g_Textures.erase(it);
    }

  }

  return count;
}

typedef HRESULT(STDMETHODCALLTYPE* CreateTexture2D_t)(
    ID3D11Device* This,
    /* [annotation] */
    _In_ const D3D11_TEXTURE2D_DESC* pDesc,
    /* [annotation] */
    _In_reads_opt_(_Inexpressible_(pDesc->MipLevels * pDesc->ArraySize))
        const D3D11_SUBRESOURCE_DATA* pInitialData,
    /* [annotation] */
    _COM_Outptr_opt_ ID3D11Texture2D** ppTexture2D);

CreateTexture2D_t True_CreateTexture2D;


void InstallTextureHooks(ID3D11Texture2D* pTexture2D)
{
  if(nullptr == g_Old_ID3D11Texture2D_Release){
    DWORD oldProtect;

    VirtualProtect(pTexture2D, sizeof(void*) * 3, PAGE_EXECUTE_READWRITE, &oldProtect);
    g_Old_ID3D11Texture2D_Release = (Release_t) * (void**)((*(char**)(pTexture2D)) + sizeof(void*) * 2);    
    VirtualProtect(pTexture2D, sizeof(void*) * 3, oldProtect, NULL);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)g_Old_ID3D11Texture2D_Release, My_ID3D11Texture2D_Release);
    DetourTransactionCommit();
  }
}

HRESULT STDMETHODCALLTYPE My_CreateTexture2D(
    ID3D11Device* This,
    /* [annotation] */
    _In_ const D3D11_TEXTURE2D_DESC* pDesc,
    /* [annotation] */
    _In_reads_opt_(_Inexpressible_(pDesc->MipLevels * pDesc->ArraySize))
        const D3D11_SUBRESOURCE_DATA* pInitialData,
    /* [annotation] */
    _COM_Outptr_opt_ ID3D11Texture2D** ppTexture2D) {
  if (pDesc && ppTexture2D) {
    // if (pDesc->Width > 1 &&
    //    pDesc->Height > 1 && pDesc->Usage == D3D11_USAGE_STAGING)
    //  return E_ABORT;

    /* if (pDesc->Width > 1 && pDesc->Height > 1) {
    MessageBoxA(
        0,
        (std::to_string((UINT64)This) + ", " + std::to_string(pDesc->Width) +
        ", " + std::to_string(pDesc->Height) + ", " +
        std::to_string(pDesc->Format) + ", " +
        std::to_string(pDesc->MipLevels) + ", " +
        std::to_string(pDesc->ArraySize) + ", " +
        std::to_string(pDesc->SampleDesc.Count) + ", " +
        std::to_string(pDesc->SampleDesc.Quality) + ", " +
        std::to_string(pDesc->Usage) + ", " +
        std::to_string(pDesc->BindFlags) + ", " +
        std::to_string(pDesc->CPUAccessFlags) + ", " +
        std::to_string(pDesc->MiscFlags) + ", " +
        std::to_string(0 != pInitialData))
            .c_str(),
        "My_CreateTexture2D", MB_OK);
    }*/

    static D3D11_TEXTURE2D_DESC Desc;

    if (pDesc->Width > 1 && pDesc->Height > 1 && pDesc->MipLevels == 1 &&
        pDesc->ArraySize == 1 && pDesc->Format == DXGI_FORMAT_B8G8R8A8_UNORM &&
        pDesc->SampleDesc.Count == 1 && pDesc->SampleDesc.Quality == 0 &&
        pDesc->Usage == D3D11_USAGE_DEFAULT &&
        pDesc->BindFlags ==
            (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE) &&
        pDesc->CPUAccessFlags == 0 && pDesc->MiscFlags == 0) {
      Desc = *pDesc;
      Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

      // Desc.Width = Width;
      // Desc.Height = Height;
      // Desc.MipLevels = 1;
      // Desc.ArraySize = 1;
      // Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      // Desc.SampleDesc.Count = 1;
      // Desc.SampleDesc.Quality = 0;
      // Desc.Usage = D3D11_USAGE_DEFAULT;
      // Desc.CPUAccessFlags = 0;
      Desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

      HRESULT result =
          True_CreateTexture2D(This, &Desc, pInitialData, ppTexture2D);

      if (SUCCEEDED(result) && *ppTexture2D) {
        InstallTextureHooks(*ppTexture2D);
        pLastGoodTexture2d = *ppTexture2D;
      } else {
        pLastGoodTexture2d = NULL;
      }

      return result;
    } else if (pLastGoodTexture2d && pDesc->Width == Desc.Width &&
               pDesc->Height == Desc.Height && pDesc->MipLevels == 1 &&
               pDesc->ArraySize == 1 &&
               pDesc->Format == DXGI_FORMAT_B8G8R8A8_UNORM &&
               pDesc->SampleDesc.Count == 1 && pDesc->SampleDesc.Quality == 0 &&
               pDesc->Usage == D3D11_USAGE_DEFAULT &&
               pDesc->BindFlags == (D3D11_BIND_SHADER_RESOURCE) &&
               pDesc->CPUAccessFlags == 0 &&
               pDesc->MiscFlags == D3D11_RESOURCE_MISC_SHARED) {
      HRESULT result =
          True_CreateTexture2D(This, &Desc, pInitialData, ppTexture2D);

      if (SUCCEEDED(result) && *ppTexture2D) {
        InstallTextureHooks(*ppTexture2D);

        int activeBrowser = 0;
        try {
          std::unique_lock<std::mutex> lock(g_GpuPipeClientMutex);

          g_GpuPipeClient.WriteInt32(
              (INT32)advancedfx::interop::HostGpuMessage::GetActiveBrowser);
          g_GpuPipeClient.Flush();
          activeBrowser = g_GpuPipeClient.ReadInt32();
        } catch (const std::exception& e) {
          DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                      << e.what();
          DebugBreak();
          activeBrowser = 0;
        }

        if (activeBrowser) {
          // ID3D11Texture2D* tempTexture = nullptr;

          // if (SUCCEEDED(True_CreateTexture2D(This, &Desc, nullptr,
          //                                   &tempTexture))) {
          ID3D11ShaderResourceView* view = nullptr;
          if (SUCCEEDED(g_pDevice->CreateShaderResourceView(*ppTexture2D, NULL,
                                                            &view))) {
            HANDLE shareHandle = AfxInteropGetSharedHandle(pLastGoodTexture2d);
            HANDLE tempTextureShareHandle =
                AfxInteropGetSharedHandle(*ppTexture2D);

            // MessageBoxA(0, "OK", "OK", MB_OK);
            g_Textures.emplace(
                std::piecewise_construct, std::make_tuple(pLastGoodTexture2d),
                std::make_tuple(shareHandle, tempTextureShareHandle, view,
                                Desc.Width, Desc.Height, activeBrowser));

            view->Release();
          }

          //  tempTexture->Release();
          //}
        }
        pLastGoodTexture2d = NULL;
      }

      return result;
    }
  }

  return True_CreateTexture2D(This, pDesc, pInitialData, ppTexture2D);
}

typedef HRESULT(STDMETHODCALLTYPE* CreateRenderTargetView_t)(
    ID3D11Device* This,
    /* [annotation] */
    _In_ ID3D11Resource* pResource,
    /* [annotation] */
    _In_opt_ const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
    /* [annotation] */
    _COM_Outptr_opt_ ID3D11RenderTargetView** ppRTView);

CreateRenderTargetView_t g_Old_CreateRenderTargetView = nullptr;

HRESULT STDMETHODCALLTYPE
My_CreateRenderTargetView(ID3D11Device* This,
                          /* [annotation] */
                          _In_ ID3D11Resource* pResource,
                          /* [annotation] */
                          _In_opt_ const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
                          /* [annotation] */
                          _COM_Outptr_opt_ ID3D11RenderTargetView** ppRTView) {

  HRESULT result =
      g_Old_CreateRenderTargetView(This, pResource, pDesc, ppRTView);

  return result;
}

Release_t g_Old_ID3D11Device_Release = nullptr;

ULONG STDMETHODCALLTYPE My_ID3D11Device_Release(ID3D11Device* This) {

  ULONG count = g_Old_ID3D11Device_Release(This);

  if (0 == count) {
    try {
      std::unique_lock<std::mutex> lock(g_GpuPipeClientMutex);
      g_GpuPipeClient.Flush();
    } catch (const std::exception &) {
    }    
    try {
      std::unique_lock<std::mutex> lock(g_GpuPipeClientMutex);
      g_GpuPipeClient.ClosePipe();
    } catch (const std::exception &) {
      DebugBreak();
    }

    if (pQuery) {
      pQuery->Release();
      pQuery = NULL;
    }

    if (pVertexShader) {
      pVertexShader->Release();
      pVertexShader = NULL;
    }
    if (pPixelShader) {
      pPixelShader->Release();
      pPixelShader = NULL;
    }
    if (pInputLayout) {
      pInputLayout->Release();
      pInputLayout = NULL;
    }
    if (pVertexBuffer) {
      pVertexBuffer->Release();
      pVertexBuffer = NULL;
    }

    if (pContext) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourDetach(&(PVOID&)g_Org_ClearRenderTargetView, My_ClearRenderTargetView);
      DetourTransactionCommit();

      pContext->Release();
      pContext = NULL;
    }

    g_pDevice = NULL;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)g_Old_ID3D11Device_Release, My_ID3D11Device_Release);
    DetourDetach(&(PVOID&)True_CreateTexture2D, My_CreateTexture2D);
    //DetourDetach(&(PVOID&)g_Old_CreateRenderTargetView, My_CreateRenderTargetView);
    DetourTransactionCommit();    
  }

  return count;
}

DWORD GetParentProcessId( )
{
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD ppid = 0, pid = GetCurrentProcessId();

    hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    __try{
        if( hSnapshot == INVALID_HANDLE_VALUE ) __leave;

        ZeroMemory( &pe32, sizeof( pe32 ) );
        pe32.dwSize = sizeof( pe32 );
        if( !Process32First( hSnapshot, &pe32 ) ) __leave;

        do{
            if( pe32.th32ProcessID == pid ){
                ppid = pe32.th32ParentProcessID;
                break;
            }
        }while( Process32Next( hSnapshot, &pe32 ) );

    }
    __finally{
        if( hSnapshot != INVALID_HANDLE_VALUE ) CloseHandle( hSnapshot );
    }
    return ppid;
}

typedef HRESULT(WINAPI* D3D11CreateDevice_t)(
    _In_opt_ IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    _In_opt_ const D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    _Out_opt_ ID3D11Device** ppDevice,
    _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
    _Out_opt_ ID3D11DeviceContext** ppImmediateContext);

D3D11CreateDevice_t TrueD3D11CreateDevice = NULL;

HRESULT WINAPI
MyD3D11CreateDevice(_In_opt_ IDXGIAdapter* pAdapter,
                    D3D_DRIVER_TYPE DriverType,
                    HMODULE Software,
                    UINT Flags,
                    _In_opt_ const D3D_FEATURE_LEVEL* pFeatureLevels,
                    UINT FeatureLevels,
                    UINT SDKVersion,
                    _Out_opt_ ID3D11Device** ppDevice,
                    _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
                    _Out_opt_ ID3D11DeviceContext** ppImmediateContext) {
  HRESULT result = TrueD3D11CreateDevice(
      pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels,
      SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

  if (IS_ERROR(result) || NULL == ppDevice || NULL == *ppDevice) {
    g_detours_error = E_FAIL;
    return result;
  }

  if (NULL == g_Old_ID3D11Device_Release) {
    g_pDevice = *ppDevice;

    DWORD oldProtect;  

    //MessageBoxA(0, std::to_string(GetParentProcessId()).c_str(), "LOL", MB_OK);

    /*
    HRESULT hr;
    IDXGIDevice2* pDXGIDevice = nullptr;
    hr = g_pDevice->QueryInterface(__uuidof(IDXGIDevice2),
                                      (void**)&pDXGIDevice);

    IDXGIAdapter* pDXGIAdapter = nullptr;
    hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);

    IDXGIFactory2* pIDXGIFactory = nullptr;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pIDXGIFactory);

    if (pIDXGIFactory) {
      VirtualProtect(pIDXGIFactory, sizeof(void*) * 25, PAGE_EXECUTE_READWRITE,
                     &oldProtect);

      g_Old_CreateSwapChainForComposition =
          (CreateSwapChainForComposition_t) *
          (void**)((*(char**)(pIDXGIFactory)) + sizeof(void*) * 24);

      VirtualProtect(pIDXGIFactory, sizeof(void*) * 25, oldProtect, NULL);

          DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
          DetourAttach(&(PVOID&)g_Old_CreateSwapChainForComposition,
                   My_CreateSwapChainForComposition);
    }

    if (pDXGIDevice)
      pDXGIDevice->Release();

    if (pDXGIAdapter)
      pDXGIAdapter->Release();

    if (pIDXGIFactory)
      pIDXGIFactory->Release();

    */

    VirtualProtect(*ppDevice, sizeof(void*) * 10, PAGE_EXECUTE_READWRITE,
                   &oldProtect);

    g_Old_ID3D11Device_Release =
        (Release_t) * (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 2);
    True_CreateTexture2D = (CreateTexture2D_t) *
                           (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 5);
    /* g_Old_CreateRenderTargetView =
        (CreateRenderTargetView_t) *
                           (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 9);*/

    VirtualProtect(*ppDevice, sizeof(void*) * 10, oldProtect, NULL);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)g_Old_ID3D11Device_Release, My_ID3D11Device_Release);
    DetourAttach(&(PVOID&)True_CreateTexture2D, My_CreateTexture2D);
    /* DetourAttach(&(PVOID&)g_Old_CreateRenderTargetView,
                 My_CreateRenderTargetView);*/
    DetourTransactionCommit();

    if (nullptr == pQuery) {
      D3D11_QUERY_DESC queryDesc = {D3D11_QUERY_EVENT, 0};

      if (FAILED((*ppDevice)->CreateQuery(&queryDesc, &pQuery))) {
        pQuery = NULL;
       }
    }

    g_pDevice->GetImmediateContext(&pContext);
    if (pContext) {
      VirtualProtect(pContext, sizeof(void*) * 112, PAGE_EXECUTE_READWRITE,
                     &oldProtect);
      
     g_Org_OMSetRenderTargets =
          (OMSetRenderTargets_t) *
          (void**)((*(char**)(pContext)) + sizeof(void*) * 33);


     g_Org_ClearRenderTargetView =
          (ClearRenderTargetView_t) *
          (void**)((*(char**)(pContext)) + sizeof(void*) * 50);

     g_Org_Flush =
         (Flush_t) *
         (void**)((*(char**)(pContext)) + sizeof(void*) * 111);

     VirtualProtect(pContext, sizeof(void*) * 112, oldProtect, NULL);

      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach(&(PVOID&)g_Org_OMSetRenderTargets, My_OMSetRenderTargets);
      DetourAttach(&(PVOID&)g_Org_ClearRenderTargetView,
                   My_ClearRenderTargetView);
      DetourAttach(&(PVOID&)g_Org_Flush,
                   My_Flush);
      DetourTransactionCommit();
    }

    ID3DBlob* pBlobShader = nullptr;
    ID3DBlob* pBlobErrorMsgs = nullptr;

    size_t lenVertexShaderCode = strlen(g_szVertexShaderCode);
    size_t lenPixelShaderCode = strlen(g_szPixelShaderCode);

    if (SUCCEEDED(D3DCompile2(
        g_szVertexShaderCode, lenVertexShaderCode, "afx_drawtexture_vs50", NULL, NULL,
        "main", "vs_5_0", 0, 0, 0, NULL, 0, &pBlobShader,
                              &pBlobErrorMsgs))) {

      g_pDevice->CreateVertexShader(pBlobShader->GetBufferPointer(),
                                  pBlobShader->GetBufferSize(), NULL,
                                  &pVertexShader);

          if (SUCCEEDED(g_pDevice->CreateInputLayout(
              g_InputElementDesc, ARRAYSIZE(g_InputElementDesc),
              pBlobShader->GetBufferPointer(), pBlobShader->GetBufferSize(),
              &pInputLayout))) {
      }       
    }

    if (pBlobShader) {
      pBlobShader->Release();
      pBlobShader = NULL;
    }
    if (pBlobErrorMsgs) {
      pBlobErrorMsgs->Release();
      pBlobErrorMsgs = NULL;
    }

    if (SUCCEEDED(D3DCompile2(g_szPixelShaderCode, lenPixelShaderCode,
                              "afx_drawtexture_ps50", NULL, NULL, "main",
                              "ps_5_0", 0, 0, 0, NULL, 0, &pBlobShader,
                              &pBlobErrorMsgs))) {
      g_pDevice->CreatePixelShader(pBlobShader->GetBufferPointer(),
                                  pBlobShader->GetBufferSize(), NULL,
                                  &pPixelShader);
    } else {
      if (pBlobErrorMsgs) {
        MessageBoxA(0,(LPCSTR)pBlobErrorMsgs->GetBufferPointer(),"ERROR",MB_OK|MB_ICONERROR);
      }
    }

    if (pBlobShader) {
      pBlobShader->Release();
    }
    if (pBlobErrorMsgs) {
      pBlobErrorMsgs->Release();
    }

    float vertex_data_array[] = {
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f
    };

    D3D11_BUFFER_DESC vertex_buff_descr = {};
    vertex_buff_descr.ByteWidth = sizeof(vertex_data_array);
    vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
    vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sr_data = {0};
    sr_data.pSysMem = vertex_data_array;
    g_pDevice->CreateBuffer(&vertex_buff_descr, &sr_data, &pVertexBuffer);

    std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_gpu_handler_");
    strPipeName.append(std::to_string(GetParentProcessId()));

    try {
      std::unique_lock<std::mutex> lock(g_GpuPipeClientMutex);

      while (true)
      {
        bool bError = false;
        try {
          g_GpuPipeClient.OpenPipe(strPipeName.c_str(), INFINITE);
          Sleep(100);
        }
        catch(const std::exception&)
        {
          bError = true;
        }

        if(!bError) break;
      }

      //MessageBoxA(0, strPipeName.c_str(), "CLIENT", MB_OK);

      g_GpuPipeClient.WriteUInt32(GetCurrentProcessId());
      g_GpuPipeClient.Flush();
    } catch (const std::exception& e) {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << e.what();
      DebugBreak();
    }
  }

  return result;
}

/*
class TestClient : public CefClient, public CefLifeSpanHandler {
  IMPLEMENT_REFCOUNTING(TestClient);

  public:
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }

  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE { return false; }
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE {
  
      CefQuitMessageLoop();
  }
};

class TestApp : public CefApp, public CefBrowserProcessHandler {
  IMPLEMENT_REFCOUNTING(TestApp);

 public:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
      OVERRIDE {
    return this;
  }

  void OnContextInitialized() OVERRIDE {
    CefWindowInfo window_info;
    window_info.SetAsPopup(NULL, "cefsimple");

    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = 60;

    CefBrowserHost::CreateBrowser(window_info, new TestClient(),
                                  "http://www.matrixstorm.com/",
                                  browser_settings, nullptr, nullptr);
  };
};
*/

// Entry point function for all processes.
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine,
                      int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);

  bool afxWindow = true;

  {
    LPWSTR* szArglist;
    int nArgs;
    int i;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (NULL != szArglist) {
      for (i = 0; i < nArgs; i++) {
        if (0 == wcsicmp(L"--afx-no-window", szArglist[i]))
          afxWindow = false;
      }
    }
  }

  //
  g_hD3d11Dll = LoadLibrary(_T("d3d11.dll"));

  TrueD3D11CreateDevice =
      (D3D11CreateDevice_t)GetProcAddress(g_hD3d11Dll, "D3D11CreateDevice");

  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach(&(PVOID&)TrueD3D11CreateDevice, MyD3D11CreateDevice);
  g_detours_error = DetourTransactionCommit();
  //
  
  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();

  void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  // SimpleApp implements application-level callbacks for the browser process.
  // It will create the first browser instance in OnContextInitialized() after
  // CEF has initialized.
  CefRefPtr<SimpleApp> app(new SimpleApp);

  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Specify CEF global settings here.
  CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  settings.multi_threaded_message_loop = false;
  settings.windowless_rendering_enabled = true;
  
  // Initialize CEF.
  CefInitialize(main_args, settings, app, sandbox_info);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  app = nullptr;

  // Shut down CEF.
  CefShutdown();

  //
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourDetach(&(PVOID&)TrueD3D11CreateDevice, MyD3D11CreateDevice);
  g_detours_error = DetourTransactionCommit();
  //

  FreeLibrary(g_hD3d11Dll);

  return 0;
}
