// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "afx-cefhud-interop/simple_app.h"

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

#include "afx-cefhud-interop/simple_handler.h"

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

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler());

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;
  browser_settings.file_access_from_file_urls = STATE_ENABLED;
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

    // Create the first browser window.
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                  nullptr);
  }
}
