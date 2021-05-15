// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "afx-cefhud-interop/simple_app.h"

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_parser.h"
#include "include/cef_request_context.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

#include "afx-cefhud-interop/simple_handler.h"
#include "afx-cefhud-interop/scheme_handler_impl.h"
#include "afx-cefhud-interop/AfxInterop.h"

namespace {

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class SimpleWindowDelegate : public CefWindowDelegate {
 public:
  explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
    // Add the browser view and show the window.
    window->AddChildView(browser_view_);
    window->Show();

    // Give keyboard focus to the browser view.
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
    // Allow the window to close if the browser says it's OK.
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser)
      return browser->GetHost()->TryCloseBrowser();
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) OVERRIDE {
    return CefSize(800, 600);
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  SimpleBrowserViewDelegate() {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) OVERRIDE {
    // Create a new top-level Window for the popup. It will show itself after
    // creation.
    CefWindow::CreateTopLevelWindow(
        new SimpleWindowDelegate(popup_browser_view));

    // We created the Window.
    return true;
  }

 private:
  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
};

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace

SimpleApp::SimpleApp() {}

  void SimpleApp::OnRegisterCustomSchemes(
      CefRawPtr<CefSchemeRegistrar> registrar) {

     // Register the custom scheme as standard and secure.
     // Must be the same implementation in all processes.
     //registrar->AddCustomScheme("afx", true, false, false, true, true, false);
    registrar->AddCustomScheme("afx", CEF_SCHEME_OPTION_STANDARD |
                                             CEF_SCHEME_OPTION_SECURE |
                                         CEF_SCHEME_OPTION_CORS_ENABLED);
  }


void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  //auto all_prefs = CefRequestContext::GetGlobalContext()->GetAllPreferences(true);
  //auto all_prefs_val = CefValue::Create();
  //all_prefs_val->SetDictionary(all_prefs);
  //DLOG(INFO) << "PREFS: " << CefWriteJSON(all_prefs_val, JSON_WRITER_DEFAULT).ToString().c_str();  

  scheme_handler::RegisterSchemeHandlerFactory();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler(this));

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;
  browser_settings.windowless_frame_rate = 60;

  std::string argUrl = command_line->GetSwitchValue("url");

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().

  bool bWindow = !command_line->HasSwitch("afx-no-window");

  if(bWindow)
    window_info.SetAsPopup(NULL, "cefsimple");
  else
    window_info.SetAsWindowless(NULL);
  window_info.width = 640;
  window_info.height = 400;
  window_info.shared_texture_enabled =  false;
  window_info.external_begin_frame_enabled = bWindow ? false : true; // No need to draw what one can't see.
#endif

  if (argUrl.empty()) {
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\" color=\"black\">"
          "<h2>You forgot to pass the URL d(o)(O)b</h2></body></html>";
    argUrl = GetDataURI(ss.str(), "text/html");
  }

  auto argStr = CefValue::Create();
  auto argCmd = CefDictionaryValue::Create();
  argCmd->SetString("commandLineString",
                    command_line->GetCommandLineString());
  argStr->SetDictionary(argCmd);

  extra_info_ = CefListValue::Create();
  extra_info_->SetSize(3);
  extra_info_->SetString(0,"index");
  extra_info_->SetInt(1, GetCurrentProcessId());
  extra_info_->SetString(2, CefWriteJSON(argStr, JSON_WRITER_DEFAULT));

  // Create the first browser window.
  CefBrowserHost::CreateBrowser(window_info, handler, argUrl, browser_settings,
                                nullptr, nullptr);
}

void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) {
  if (extra_info_ && 3 == extra_info_->GetSize()) {
    auto interopType = extra_info_->GetString(0);
    auto handlerProcessId = extra_info_->GetInt(1);
    auto argStr = extra_info_->GetString(2);

    if (frame->IsMain()) {
      if (interopType.compare("drawing") == 0) {
        std::unique_lock<std::mutex> lock(m_InteropMutex);

        CefRefPtr<advancedfx::interop::CInterop> interop;

        auto window = context->GetGlobal();
        window->SetValue(
            "afxInterop",
            advancedfx::interop::CreateDrawingInterop(
                browser, frame, context, argStr, handlerProcessId, &interop),
            V8_PROPERTY_ATTRIBUTE_NONE);
        m_Interops[frame->GetIdentifier()] = interop;
      } else if (interopType.compare("engine") == 0) {
        std::unique_lock<std::mutex> lock(m_InteropMutex);

        CefRefPtr<advancedfx::interop::CInterop> interop;

        auto window = context->GetGlobal();
        window->SetValue(
            "afxInterop",
            advancedfx::interop::CreateEngineInterop(
                browser, frame, context, argStr, handlerProcessId, &interop),
            V8_PROPERTY_ATTRIBUTE_NONE);
        m_Interops[frame->GetIdentifier()] = interop;

      } else if (interopType.compare("index") == 0) {
        std::unique_lock<std::mutex> lock(m_InteropMutex);

        CefRefPtr<advancedfx::interop::CInterop> interop;

        auto window = context->GetGlobal();
        window->SetValue(
            "afxInterop",
            advancedfx::interop::CreateInterop(browser, frame, context, argStr,
                                               handlerProcessId, &interop),
            V8_PROPERTY_ATTRIBUTE_NONE);
        m_Interops[frame->GetIdentifier()] = interop;

      }
    }

    extra_info_ = nullptr;
  }
}

void SimpleApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) {
  auto it = m_Interops.find(frame->GetIdentifier());
  if (it != m_Interops.end()) {
    it->second->CloseInterop();
    m_Interops.erase(it);
  }
}

void SimpleApp::OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) {
    // disable creation of a GPUCache/ folder on disk
    command_line->AppendSwitch("disable-gpu-shader-disk-cache");

    //command_line->AppendSwitch("disable-accelerated-video-decode");

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
  }