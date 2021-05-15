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

#include <map>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <tchar.h>
#include <third_party/Detours/src/detours.h>
#include <tlhelp32.h>

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

ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11Query* pQuery = NULL;
ID3D11Texture2D* pLastGoodTexture2d = NULL;

ID3D11VertexShader* pVertexShader = NULL;
ID3D11PixelShader* pPixelShader = NULL;
ID3D11InputLayout* pInputLayout = NULL;
ID3D11Buffer* pVertexBuffer = NULL;

UINT vertex_stride = 5 * sizeof(float);
UINT vertex_offset = 0;
UINT vertex_count = 4;

struct TextureMapElem {
  ID3D11ShaderResourceView* View;
  UINT Width;
  UINT Height;

  TextureMapElem(ID3D11ShaderResourceView* view, UINT width, UINT height) : View(view), Width(width), Height(height) {
    View->AddRef();
  }

  ~TextureMapElem() {
    View->Release();
  }
};


std::map<ID3D11Texture2D *,HANDLE> g_Textures;
std::map<ID3D11Texture2D*, TextureMapElem> g_TextureMap;

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

HANDLE g_ActiveShareHandle = INVALID_HANDLE_VALUE;

bool AfxWaitForGPU()
{
  if(!pDevice) return false;

	bool bOk = false;
	bool immediateContextUsed = false;

	if (!pContext)
	{
		immediateContextUsed = true;
		pDevice->GetImmediateContext(&pContext);
	}

	if (pDevice && pQuery)
	{
		pContext->Flush();

		pContext->End(pQuery);

		while (S_OK != pContext->GetData(pQuery, NULL, 0, 0))
			;

		bOk = true;
	}

	if (immediateContextUsed)
	{
		pContext->Release();
		pContext = NULL;
	}

	return bOk;
}

class CGpuPipeClient : public advancedfx::interop::CPipeClient 
{
public:
} g_GpuPipeClient;

typedef void(STDMETHODCALLTYPE* OMSetRenderTargets_t)(
    ID3D11DeviceContext* This,
    /* [annotation] */
    _In_range_(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) UINT NumViews,
    /* [annotation] */
    _In_reads_opt_(NumViews) ID3D11RenderTargetView* const* ppRenderTargetViews,
    /* [annotation] */
    _In_opt_ ID3D11DepthStencilView* pDepthStencilView);

OMSetRenderTargets_t g_Org_OMSetRenderTargets = nullptr;

bool g_IgnoreNextClear = false;

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

 
  if (NumViews == 1 && ppRenderTargetViews && *ppRenderTargetViews) {
    ID3D11Resource* resource = nullptr;

    ppRenderTargetViews[0]->GetResource(&resource);

    if (resource) {
    
    }
  }
}

typedef void (STDMETHODCALLTYPE * ClearRenderTargetView_t)( 
            ID3D11DeviceContext * This,
            /* [annotation] */ 
            _In_  ID3D11RenderTargetView *pRenderTargetView,
            /* [annotation] */ 
            _In_  const FLOAT ColorRGBA[ 4 ]);

ClearRenderTargetView_t g_Org_ClearRenderTargetView;

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
      auto it = g_TextureMap.find((ID3D11Texture2D*)resource);
      if (it != g_TextureMap.end()) {
        // This->CopyResource(it->first, it->second);

        if (pInputLayout && pVertexBuffer && pVertexShader && pPixelShader) {

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
                                   0.0f, (FLOAT)(it->second.Width),
                                       (FLOAT)(it->second.Height),
                                   0.0f,
                                   1.0f};
        pContext->RSSetViewports(1, &viewport);

          ID3D11RenderTargetView* views[1] = {pRenderTargetView};

          pContext->OMSetRenderTargets(1, &views[0], NULL);

          pContext->IASetPrimitiveTopology(
              D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
          pContext->IASetInputLayout(pInputLayout);
          pContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &vertex_stride,
                                       &vertex_offset);

          pContext->VSSetShader(pVertexShader, NULL, 0);
          pContext->PSSetShader(pPixelShader, NULL, 0);

          ID3D11ShaderResourceView* res[1] = {it->second.View};

          pContext->PSSetShaderResources(0, 1, &res[0]);

          pContext->Draw(vertex_count, 0);

//          AfxWaitForGPU();

          //

          pContext->PSSetShaderResources(0, 1, &oldShaderResources[0]);
          pContext->PSSetShader(oldPixelShader, NULL, 0);
          pContext->VSSetShader(oldVertexShader, NULL, 0);
                    pContext->IASetVertexBuffers(0, 1, &oldBuffers[0], &oldStrides[0],
                                       &oldOffsets[0]);
          pContext->IASetInputLayout(oldInputLayout);
                    pContext->IASetPrimitiveTopology(
              oldInputTopology);
          pContext->OMSetRenderTargets(1, &oldRenderTarget[0], NULL);
          if (1 <= oldNumViewPorts)
            pContext->RSSetViewports(1, &oldviewport);
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

    if (pLastGoodTexture2d == This)
      pLastGoodTexture2d = NULL;

    auto it = g_Textures.find(This);
    if(it != g_Textures.end())
    {
      try {
        g_GpuPipeClient.WriteInt32(
            (INT32)advancedfx::interop::HostMessage::GpuRelaseShareHandle);
        g_GpuPipeClient.WriteHandle(it->second);
        g_GpuPipeClient.Flush();
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
        /*
        std::string str = "Error in " + std::string(__FILE__) + ":" +
                          std::to_string(__LINE__) + ": " +
                          std::string(e.what());

        OutputDebugStringA(str.c_str());*/
      }

      if (INVALID_HANDLE_VALUE != it->second)
        CloseHandle(it->second);
      g_Textures.erase(it);
      g_TextureMap.erase(This);
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
  if (pDesc) {
    if (pDesc->Width > 1 && pDesc->Height > 1 && pDesc->MipLevels == 1 &&
        pDesc->ArraySize == 1 && pDesc->Format == DXGI_FORMAT_B8G8R8A8_UNORM &&
        pDesc->SampleDesc.Count == 1 && pDesc->SampleDesc.Quality == 0 &&
        pDesc->Usage == D3D11_USAGE_DEFAULT &&
        pDesc->BindFlags ==
            (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE) &&
        pDesc->CPUAccessFlags == 0 && pDesc->MiscFlags == 0) {

      HRESULT result =
          True_CreateTexture2D(This, pDesc, pInitialData, ppTexture2D);
      if (SUCCEEDED(result) && *ppTexture2D)
        InstallTextureHooks(*ppTexture2D);

      if (pContext && ppTexture2D && SUCCEEDED(result)) {
        ID3D11RenderTargetView* views[1] = {nullptr};
        pContext->OMGetRenderTargets(1, views, NULL);

        if (views[0])
          views[0]->Release();
        else {
        }
        pLastGoodTexture2d = *ppTexture2D;
        /*MessageBoxA(0,
                    (+", " + std::to_string(pDesc->Width) + ", " +
                     std::to_string(pDesc->Height) + ", " +
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
                    "LOL", MB_OK);*/
      }

      return result;
    } else if (pDesc->Width > 1 && pDesc->Height > 1 && pDesc->MipLevels == 1 &&
               pDesc->ArraySize == 1 &&
               pDesc->Format == DXGI_FORMAT_B8G8R8A8_UNORM &&
               pDesc->SampleDesc.Count == 1 && pDesc->SampleDesc.Quality == 0 &&
               pDesc->Usage == D3D11_USAGE_DEFAULT &&
               pDesc->BindFlags == D3D11_BIND_SHADER_RESOURCE &&
               pDesc->CPUAccessFlags == 0 &&
               pDesc->MiscFlags == D3D11_RESOURCE_MISC_SHARED) {
      D3D11_TEXTURE2D_DESC Desc = *pDesc;

      // Desc.Width = Width;
      // Desc.Height = Height;
      Desc.MipLevels = 1;
      Desc.ArraySize = 1;
      Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      Desc.SampleDesc.Count = 1;
      Desc.SampleDesc.Quality = 0;
      Desc.Usage = D3D11_USAGE_DEFAULT;
      Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      Desc.CPUAccessFlags = 0;
      Desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

      HRESULT result =
          True_CreateTexture2D(This, &Desc, pInitialData, ppTexture2D);
      if (SUCCEEDED(result) && *ppTexture2D) {
        InstallTextureHooks(*ppTexture2D);

        if (pLastGoodTexture2d) {
          ID3D11Texture2D* tempTexture = nullptr;
          
          
          if(SUCCEEDED(True_CreateTexture2D(This, &Desc, pInitialData, &tempTexture))) {
            // MessageBoxA(0, "OK", "OK", MB_OK);
            ID3D11ShaderResourceView* view = nullptr;
            if (SUCCEEDED(pDevice->CreateShaderResourceView(tempTexture, NULL,
                                                            &view))) {
              HANDLE srcHandle = AfxInteropGetSharedHandle(*ppTexture2D);
              HANDLE dstHandle = AfxInteropGetSharedHandle(tempTexture);

              g_Textures.emplace(*ppTexture2D, srcHandle);
              g_Textures.emplace(pLastGoodTexture2d, dstHandle);

              g_GpuPipeClient.WriteInt32(
                  (INT32)advancedfx::interop::HostMessage::MapHandle);
              g_GpuPipeClient.WriteHandle(srcHandle);
              g_GpuPipeClient.WriteHandle(dstHandle);

              g_TextureMap.emplace(std::piecewise_construct,
                                   std::make_tuple(pLastGoodTexture2d),
                                   std::make_tuple(view,Desc.Width,Desc.Height));

              view->Release();
            }

            tempTexture->Release();
          }
          pLastGoodTexture2d = NULL;
        }
      }

      return result;
    }

    return True_CreateTexture2D(This, pDesc, pInitialData, ppTexture2D);
  }

  return True_CreateTexture2D(This, pDesc, pInitialData, ppTexture2D);
}

Release_t g_Old_ID3D11Device_Release;

ULONG STDMETHODCALLTYPE My_ID3D11Device_Release(ID3D11Device* This) {

  ULONG count = g_Old_ID3D11Device_Release(This);

  if (0 == count) {
    try {
      g_GpuPipeClient.Flush();
    } catch (const std::exception &) {
    }    
    try {
      g_GpuPipeClient.ClosePipe();
    } catch (const std::exception &) {
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
      DetourDetach(&(PVOID&)g_Org_OMSetRenderTargets, My_OMSetRenderTargets);
      DetourTransactionCommit();

      pContext->Release();
      pContext = NULL;
    }

    pDevice = NULL;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)g_Old_ID3D11Device_Release, My_ID3D11Device_Release);
    DetourDetach(&(PVOID&)True_CreateTexture2D, My_CreateTexture2D);
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

  if (IS_ERROR(result) || NULL == ppDevice) {
    g_detours_error = E_FAIL;
    return result;
  }

  if (NULL == g_Old_ID3D11Device_Release) {
    DWORD oldProtect;
    VirtualProtect(*ppDevice, sizeof(void*) * 8, PAGE_EXECUTE_READWRITE,
                   &oldProtect);

    g_Old_ID3D11Device_Release =
        (Release_t) * (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 2);
    True_CreateTexture2D = (CreateTexture2D_t) *
                           (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 5);

    VirtualProtect(*ppDevice, sizeof(void*) * 8, oldProtect, NULL);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)g_Old_ID3D11Device_Release, My_ID3D11Device_Release);
    DetourAttach(&(PVOID&)True_CreateTexture2D, My_CreateTexture2D);
    DetourTransactionCommit();

    if (nullptr == pQuery) {
      pDevice = *ppDevice;

      D3D11_QUERY_DESC queryDesc = {D3D11_QUERY_EVENT, 0};

      if (FAILED((*ppDevice)->CreateQuery(&queryDesc, &pQuery))) {
        pQuery = NULL;
       }
    }

    pDevice->GetImmediateContext(&pContext);
    if (pContext) {
      VirtualProtect(pContext, sizeof(void*) * 51, PAGE_EXECUTE_READWRITE,
                     &oldProtect);

/*      g_Org_OMSetRenderTargets =
          (OMSetRenderTargets_t) *
          (void**)((*(char**)(pContext)) + sizeof(void*) * 33);*/
     g_Org_ClearRenderTargetView =
          (ClearRenderTargetView_t) *
          (void**)((*(char**)(pContext)) + sizeof(void*) * 50);

      VirtualProtect(pContext, sizeof(void*) * 51, oldProtect, NULL);

      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach(&(PVOID&)g_Org_ClearRenderTargetView,
                   My_ClearRenderTargetView);
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

      pDevice->CreateVertexShader(pBlobShader->GetBufferPointer(),
                                  pBlobShader->GetBufferSize(), NULL,
                                  &pVertexShader);

          if (SUCCEEDED(pDevice->CreateInputLayout(
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
      pDevice->CreatePixelShader(pBlobShader->GetBufferPointer(),
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
    pDevice->CreateBuffer(&vertex_buff_descr, &sr_data, &pVertexBuffer);

    std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_handler_");
    strPipeName.append(std::to_string(GetParentProcessId()));

    try {
      while(true)
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
      g_GpuPipeClient.WriteInt32(0);
      g_GpuPipeClient.Flush();
    } catch (const std::exception& e) {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << e.what();
    }
  }

  return result;
}

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
