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

std::map<ID3D11Texture2D *,HANDLE> g_Textures;

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

typedef void (STDMETHODCALLTYPE * ClearRenderTargetView_t)( 
            ID3D11DeviceContext * This,
            /* [annotation] */ 
            _In_  ID3D11RenderTargetView *pRenderTargetView,
            /* [annotation] */ 
            _In_  const FLOAT ColorRGBA[ 4 ]);

ClearRenderTargetView_t g_Org_ClearRenderTargetView;

void STDMETHODCALLTYPE My_ClearRenderTargetView( 
            ID3D11DeviceContext * This,
            /* [annotation] */ 
            _In_  ID3D11RenderTargetView *pRenderTargetView,
            /* [annotation] */ 
            _In_  const FLOAT ColorRGBA[ 4 ]) {

  if(pRenderTargetView)
  {
    ID3D11Resource * resource = nullptr;

    pRenderTargetView->GetResource(&resource);

    if(resource)
    {
      auto it = g_Textures.find((ID3D11Texture2D*)resource);
      if(it != g_Textures.end())
      {
        //FLOAT col[4] = {0,0,0.5,0.5};

        g_Org_ClearRenderTargetView(This, pRenderTargetView, ColorRGBA);

        HANDLE hHandle = it->second;

        g_GpuPipeClient.WriteInt32((INT32)advancedfx::interop::HostMessage::GpuOfferShareHandle);
        g_GpuPipeClient.WriteHandle(hHandle);
        g_GpuPipeClient.Flush();
        bool waitForGPU = g_GpuPipeClient.ReadBoolean();
        if(waitForGPU) {
          //AfxWaitForGPU();
          g_GpuPipeClient.WriteBoolean(true);
          g_GpuPipeClient.Flush();      
        }

        resource->Release();
        return;
      }

      resource->Release();
    }
  }

   g_Org_ClearRenderTargetView(This, pRenderTargetView, ColorRGBA);
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
  if(nullptr == pContext)
  {
    pDevice->GetImmediateContext(&pContext);
    if(pContext)
    {
      DWORD oldProtect;
      VirtualProtect(pContext, sizeof(void*) * 51, PAGE_EXECUTE_READWRITE, &oldProtect);
          
      g_Org_ClearRenderTargetView = (ClearRenderTargetView_t) * (void**)((*(char**)(pContext)) + sizeof(void*) * 50);

      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach(&(PVOID&)g_Org_ClearRenderTargetView, My_ClearRenderTargetView);
      DetourTransactionCommit();

      VirtualProtect(pContext, sizeof(void*) * 51, oldProtect, NULL);
    }
  }

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

/*
          MessageBoxA(0,
          (
          +", "+std::to_string(pDesc->Width)
          +", "+std::to_string(pDesc->Height)
          +", "+std::to_string(pDesc->MipLevels)
          +", "+std::to_string(pDesc->ArraySize)
          +", "+std::to_string(pDesc->SampleDesc.Count)
          +", "+std::to_string(pDesc->SampleDesc.Quality)
          +", "+std::to_string(pDesc->Usage)
          +", "+std::to_string(pDesc->BindFlags)
          +", "+std::to_string(pDesc->CPUAccessFlags)
          +", "+std::to_string(pDesc->MiscFlags)
          +", "+std::to_string(0 != pInitialData)
          ).c_str(), "LOL", MB_OK);
*/

    if(pDesc->Width > 1 
        && pDesc->Height > 1
        && pDesc->MipLevels == 1
        && pDesc->ArraySize == 1
        && pDesc->Format == DXGI_FORMAT_B8G8R8A8_UNORM
        && pDesc->SampleDesc.Count == 1
        && pDesc->SampleDesc.Quality == 0
        && pDesc->Usage == D3D11_USAGE_DEFAULT
        && pDesc->BindFlags == (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE)
        && pDesc->CPUAccessFlags == 0
        && pDesc->MiscFlags == 0)
    {
      D3D11_TEXTURE2D_DESC Desc = *pDesc;

      // Desc.Width = Width;
      // Desc.Height = Height;
      Desc.MipLevels = 1;
      Desc.ArraySize = 1;
      Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      Desc.SampleDesc.Count = 1;
      Desc.SampleDesc.Quality = 0;
      Desc.Usage = D3D11_USAGE_DEFAULT;
      Desc.BindFlags =
          D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      Desc.CPUAccessFlags = 0;
      Desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

      HRESULT result = True_CreateTexture2D(This, pDesc, pInitialData, ppTexture2D);
      if(SUCCEEDED(result) && *ppTexture2D) InstallTextureHooks(*ppTexture2D);


      if(pContext && ppTexture2D && SUCCEEDED(result))
      {
        ID3D11RenderTargetView * views[1] = {nullptr};
        pContext->OMGetRenderTargets(1,views,NULL);

        if(views[0])
          views[0]->Release();
        else
          g_Textures.emplace(*ppTexture2D, AfxInteropGetSharedHandle(*ppTexture2D));
      } 

      return result;
    }
    else if(pDesc->Width > 1 
        && pDesc->Height > 1
        && pDesc->MipLevels == 1
        && pDesc->ArraySize == 1
        && pDesc->Format == DXGI_FORMAT_B8G8R8A8_UNORM
        && pDesc->SampleDesc.Count == 1
        && pDesc->SampleDesc.Quality == 0
        && pDesc->Usage == D3D11_USAGE_DEFAULT
        && pDesc->BindFlags == D3D11_BIND_SHADER_RESOURCE
        && pDesc->CPUAccessFlags == 0
        && pDesc->MiscFlags == D3D11_RESOURCE_MISC_SHARED
        )
    {
      D3D11_TEXTURE2D_DESC Desc = *pDesc;

      // Desc.Width = Width;
      // Desc.Height = Height;
      Desc.MipLevels = 1;
      Desc.ArraySize = 1;
      Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      Desc.SampleDesc.Count = 1;
      Desc.SampleDesc.Quality = 0;
      Desc.Usage = D3D11_USAGE_DEFAULT;
      Desc.BindFlags =
          D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
      Desc.CPUAccessFlags = 0;
      Desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

      HRESULT result = True_CreateTexture2D(This, &Desc, pInitialData, ppTexture2D);
      if(SUCCEEDED(result) && *ppTexture2D) InstallTextureHooks(*ppTexture2D);

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

  // Disable logs, since they will become unavailable upon shutdown:


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
