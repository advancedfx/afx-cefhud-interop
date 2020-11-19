// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"

#include "afx-cefhud-interop/AfxInterop.h"

#include <list>

// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler {
 public:
  SimpleApp();

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
      OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
      OVERRIDE {
    return this;
  }

  // CefBrowserProcessHandler methods:

  virtual void OnContextInitialized() OVERRIDE;


  // CefRenderProcessHandler methods:

  virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser,
          CefRefPtr<CefDictionaryValue> extra_info) OVERRIDE {
    m_ExtraInfo = extra_info->Copy(true);
  }

  virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context) OVERRIDE {
    if (frame->IsMain()) {
      if(m_ExtraInfo->HasKey("interopType") && m_ExtraInfo->HasKey("argStr")) {
        if (m_ExtraInfo->GetString("interopType").compare("drawing") == 0)
          m_Interop = advancedfx::interop::CreateDrawingInterop(browser, frame,
                                                                context, m_ExtraInfo->GetString("argStr"));
        else if (m_ExtraInfo->GetString("interopType").compare("engine") == 0)
          m_Interop = advancedfx::interop::CreateEngineInterop(browser, frame, context, m_ExtraInfo->GetString("argStr"));
        else if (m_ExtraInfo->GetString("interopType").compare("index") == 0)
          m_Interop = advancedfx::interop::CreateInterop(
              browser, frame, context, m_ExtraInfo->GetString("argStr"));
      }
    }
  }

  virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) OVERRIDE {

      if (frame->IsMain() && nullptr != m_Interop) {
        m_Interop->CloseInterop();
        m_Interop = nullptr;
      }
  }

  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE;

  // CefApp methods:
   virtual void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) OVERRIDE {
    // disable creation of a GPUCache/ folder on disk
    command_line->AppendSwitch("disable-gpu-shader-disk-cache");

    // command_line->AppendSwitch("disable-accelerated-video-decode");

    // un-comment to show the built-in Chromium fps meter
    // command_line->AppendSwitch("show-fps-counter");

    // command_line->AppendSwitch("disable-gpu-vsync");

    // Most systems would not need to use this switch - but on older hardware,
    // Chromium may still choose to disable D3D11 for gpu workarounds.
    // Accelerated OSR will not at all with D3D11 disabled, so we force it on.
    //
    // See the discussion on this issue:
    // https://github.com/daktronics/cef-mixer/issues/10
    //
    command_line->AppendSwitchWithValue("use-angle", "d3d11");

    // tell Chromium to autoplay <video> elements without
    // requiring the muted attribute or user interaction
    command_line->AppendSwitchWithValue("autoplay-policy",
                                        "no-user-gesture-required");
  }

 private:

  CefRefPtr<CefDictionaryValue> m_ExtraInfo;
  CefRefPtr<advancedfx::interop::CInterop> m_Interop;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
