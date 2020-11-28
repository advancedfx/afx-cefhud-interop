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

#include <d3d11.h>
#include <tchar.h>
#include "third_party/Detours/src/detours.h"

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

typedef ULONG(STDMETHODCALLTYPE* AddReff_t)(ID3D11Device* This);
typedef ULONG(STDMETHODCALLTYPE* Release_t)(ID3D11Device* This);

ID3D11Device* pDevice = NULL;
//ID3D11DeviceContext* pContext = NULL;
ID3D11Query* pQuery = NULL;

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
    switch (pDesc->Format) {
      default:
        break;
      case DXGI_FORMAT_B8G8R8A8_UNORM: 
        if (pDesc->MiscFlags & D3D11_RESOURCE_MISC_SHARED)
        {
          D3D11_TEXTURE2D_DESC*pDescNc = const_cast<D3D11_TEXTURE2D_DESC*>(pDesc);
          pDescNc->BindFlags |= D3D11_BIND_RENDER_TARGET;
          //MessageBoxA(0, "HI", "HI", MB_OK);
        }
        break;
    }
  }

  return True_CreateTexture2D(This, pDesc, pInitialData, ppTexture2D);
}

ULONG g_RefCOunt = 1;
AddReff_t True_AddRef = NULL;
Release_t True_Release;

ULONG STDMETHODCALLTYPE My_AddRef(ID3D11Device* This) {
  g_RefCOunt = True_AddRef(This);

  return g_RefCOunt;
}

ULONG STDMETHODCALLTYPE My_Release(ID3D11Device* This) {
  if (1 == g_RefCOunt) {
    if (pQuery) {
      pQuery->Release();
      pQuery = NULL;
    }

    pDevice = NULL;
  }

  g_RefCOunt = True_Release(This);

  if (0 == g_RefCOunt) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)True_AddRef, My_AddRef);
    DetourDetach(&(PVOID&)True_Release, My_Release);
    DetourDetach(&(PVOID&)True_CreateTexture2D, My_CreateTexture2D);
    //DetourDetach(&(PVOID&)True_CreateDeferredContext, My_CreateDeferredContext);
    g_detours_error = DetourTransactionCommit();
  }

  return g_RefCOunt;
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

  if (NULL == True_AddRef) {
    DWORD oldProtect;
    VirtualProtect(*ppDevice, sizeof(void*) * 27, PAGE_EXECUTE_READWRITE,
                   &oldProtect);

    True_AddRef =
        (AddReff_t) * (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 1);
    True_Release =
        (Release_t) * (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 2);
    True_CreateTexture2D = (CreateTexture2D_t) *
                           (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 5);
    //True_CreateDeferredContext =  (CreateDeferredContext_t) * (void**)((*(char**)(*ppDevice)) + sizeof(void*) * 27);

    VirtualProtect(*ppDevice, sizeof(void*) * 27, oldProtect, NULL);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)True_AddRef, My_AddRef);
    DetourAttach(&(PVOID&)True_Release, My_Release);
    DetourAttach(&(PVOID&)True_CreateTexture2D, My_CreateTexture2D);
    //DetourAttach(&(PVOID&)True_CreateDeferredContext, My_CreateDeferredContext);
    g_detours_error = DetourTransactionCommit();
  }

  if (SUCCEEDED(g_detours_error)) {
    pDevice = *ppDevice;

    D3D11_QUERY_DESC queryDesc = {D3D11_QUERY_EVENT, 0};

    if (FAILED((*ppDevice)->CreateQuery(&queryDesc, &pQuery))) {
      pQuery = NULL;
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

  {
    LPWSTR* szArglist;
    int nArgs;
    int i;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (NULL != szArglist) {
      bool enableConsole = false;
      for (i = 0; i < nArgs; i++) {
        if (0 == wcsicmp(L"--afxConsole", szArglist[i]))
          enableConsole = true;
      }
      if (enableConsole) {
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
  CefInitialize(main_args, settings, app.get(), sandbox_info);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

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
