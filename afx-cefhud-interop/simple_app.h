// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "afx-cefhud-interop/AfxInterop.h"

#include <include/cef_app.h>
#include <include/cef_parser.h>

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
/*
  virtual void OnBrowserCreated( CefRefPtr< CefBrowser > browser, CefRefPtr< CefDictionaryValue > _extra_info ) OVERRIDE {
    extra_info = _extra_info->Copy(false);
  }*/

  // CefRenderProcessHandler methods:

  virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context) OVERRIDE {

    if (frame->IsMain()) {

      CefString url = frame->GetURL();
     
      CefURLParts parts;
      if(!CefParseURL(url, parts)) return;

      std::string query = CefString(&parts.query).ToString();

      size_t pos = query.find("afx=");
      if(std::string::npos != pos) query = query.substr(pos + 4);
      else return;

      pos = query.find("&");
      if(std::string::npos != pos) query = query.substr(0,pos);

      auto val = CefParseJSON(CefURIDecode(query,false,(cef_uri_unescape_rule_t)(UU_SPACES|UU_PATH_SEPARATORS|UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS|UU_SPOOFING_AND_CONTROL_CHARS)), JSON_PARSER_RFC);   

      if(nullptr == val) return;

      auto extra_info = val->GetDictionary();

      if(nullptr == extra_info) return;

      if(extra_info->HasKey("interopType") && extra_info->HasKey("argStr") && extra_info->HasKey("handlerId")) {
        if (extra_info->GetString("interopType").compare("drawing") == 0) {
          auto window = context->GetGlobal();
          window->SetValue(
              "afxInterop",
              advancedfx::interop::CreateDrawingInterop(
                  browser, frame, context, extra_info->GetString("argStr"),
                  extra_info->GetInt("handlerId"), &m_Interop),
              V8_PROPERTY_ATTRIBUTE_NONE);
        }
        else if (extra_info->GetString("interopType").compare("engine") == 0) {
          
          auto window = context->GetGlobal();
          window->SetValue("afxInterop",
                           advancedfx::interop::CreateEngineInterop(
                               browser, frame, context, extra_info->GetString("argStr"),
                  extra_info->GetInt("handlerId"), &m_Interop),
                               V8_PROPERTY_ATTRIBUTE_NONE);
        }
        else if (extra_info->GetString("interopType").compare("index") == 0) {
          auto window = context->GetGlobal();
          window->SetValue("afxInterop",advancedfx::interop::CreateInterop(
                               browser, frame, context, extra_info->GetString("argStr"),
                  extra_info->GetInt("handlerId"), &m_Interop),
                               V8_PROPERTY_ATTRIBUTE_NONE);
        }
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

  // CefApp methods:
  virtual void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) OVERRIDE {
    // disable creation of a GPUCache/ folder on disk
    command_line->AppendSwitch("disable-gpu-shader-disk-cache");

    command_line->AppendSwitch("disable-accelerated-video-decode");

    // un-comment to show the built-in Chromium fps meter
    //command_line->AppendSwitch("show-fps-counter");

    command_line->AppendSwitch("disable-gpu-vsync");

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

    //

    command_line->AppendSwitch("no-sandbox");
    command_line->AppendSwitch("disable-gpu-watchdog");
    command_line->AppendSwitch("disable-hang-monitor");
    //command_line->AppendSwitch("enable-prune-gpu-command-buffers");

    //command_line->AppendSwitch("disable-gpu");
    //command_line->AppendSwitch("disable-gpu-compositing");
    //command_line->AppendSwitch("gpu-sandbox-failures-fatal");
    //command_line->AppendSwitch("disable-gpu-early-init");
    //command_line->AppendSwitch("d3d11");
    //command_line->AppendSwitch("enable-gpu");
    //command_line->AppendSwitch("disable-threaded-compositing");
    //command_line->AppendSwitch("cc-layer-tree-test-no-timeout");
    //command_line->AppendSwitch("skip-gpu-data-loading");
    //command_line->AppendSwitch("disable-mojo-renderer");
    //enable-prune-gpu-command-buffers

  }

 private:
  CefRefPtr<advancedfx::interop::CInterop> m_Interop;
  //CefRefPtr< CefDictionaryValue > extra_info;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
