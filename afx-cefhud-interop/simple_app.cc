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
#include "afx-cefhud-interop/scheme_strings.h"


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
     registrar->AddCustomScheme(scheme_handler::kScheme, true, false, false, true, true, false);
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
  CefRefPtr<SimpleHandler> handler(new SimpleHandler());

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;
  browser_settings.windowless_frame_rate = 60;

  std::string argUrl = command_line->GetSwitchValue("url");

 {
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
    window_info.width = 405;
    window_info.height = 720;
    window_info.shared_texture_enabled =  false;
    window_info.external_begin_frame_enabled = bWindow ? false : true; // No need to draw what one can't see.
#endif

    std::string url;

    if (argUrl.empty()) {
      std::stringstream ss;
      ss << "<html><body bgcolor=\"white\">"
            "<h2>You forgot to pass the URL d(o)(O)b</h2></body></html>";
      url = GetDataURI(ss.str(), "text/html");
    }
    else {
      CefRefPtr<CefDictionaryValue> _extra_info = CefDictionaryValue::Create();
      _extra_info->SetString("interopType", "index");

      auto argStr = CefValue::Create();
      auto argCmd = CefDictionaryValue::Create();
      argCmd->SetString("commandLineString",
                        command_line->GetCommandLineString());
      argStr->SetDictionary(argCmd);

      _extra_info->SetString("argStr", CefWriteJSON(argStr, JSON_WRITER_DEFAULT));
      _extra_info->SetInt("handlerId", GetCurrentProcessId());


      auto val = CefValue::Create();
      val->SetDictionary(_extra_info);

      std::string prefix;
      std::string query;
      std::string suffix;

      size_t pos_hash = argUrl.find("#"); 
      if(std::string::npos != pos_hash)
      {
        prefix = argUrl.substr(0, pos_hash);
        suffix = argUrl.substr(pos_hash);
      }
      else {
        prefix = argUrl;
      }
      size_t pos_query = prefix.find("?");

      if(std::string::npos != pos_query)
      {
        query = prefix.substr(pos_query + 1);
        prefix = prefix.substr(0, pos_query);
      }

      if(0 < query.size()) query += "&";
      query += "afx="+CefURIEncode(CefWriteJSON(val, JSON_WRITER_DEFAULT), false).ToString();

      url = prefix+"?"+query+suffix;
    }

    auto req_ctx = CefRequestContext::CreateContext(CefRequestContext::GetGlobalContext(), nullptr);
    auto plugins_enabled = CefListValue::Create();
    plugins_enabled->SetString(0,"afx");
    auto val_plugins_enabled = CefValue::Create();
    val_plugins_enabled->SetList(plugins_enabled);
    CefString req_ctx_pref_error;
    req_ctx->SetPreference("plugins.plugins_enabled", val_plugins_enabled, req_ctx_pref_error);

    //DLOG(ERROR) << "ERROR: " << req_ctx_pref_error.ToString().c_str();

    // Create the first browser window.
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                  req_ctx);
  }
}

void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context) {

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
         
          browser->SendProcessMessage(PID_BROWSER, CefProcessMessage::Create("afx-scheme-enable"));

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

void SimpleApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) {

      if (frame->IsMain() && nullptr != m_Interop) {
        m_Interop->CloseInterop();
        m_Interop = nullptr;
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
