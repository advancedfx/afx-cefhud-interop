// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "afx-cefhud-interop/simple_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include <include/cef_base.h>
#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>

namespace {

 // Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace

SimpleHandler::SimpleHandler()  {
}

SimpleHandler::~SimpleHandler() {
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  PlatformTitleChange(browser, title);
}


void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  m_Browsers.emplace(std::piecewise_construct, std::forward_as_tuple(browser->GetIdentifier()), std::forward_as_tuple(browser));
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (m_Browsers.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}


void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  m_PaintedPromiseIds.erase(browser->GetIdentifier());

  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
    m_Browsers.erase(it);
  }

  if (m_Browsers.empty()) {

    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  } else {

    // This was our main window, so ask the others to exit:
    if (browser->GetHost()->GetWindowHandle())
      CloseAllBrowsers(true);
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::CreateDrawingInterop(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefListValue> args) {
  CefBrowserSettings browser_settings;
  browser_settings.file_access_from_file_urls = STATE_ENABLED;
  browser_settings.windowless_frame_rate = 60;  // vsync doesn't matter due to
                                                // external_begin_frame_enabled

  CefWindowInfo window_info;
  window_info.SetAsWindowless(nullptr);
  window_info.shared_texture_enabled = true;
  window_info.external_begin_frame_enabled = true;
  window_info.height = 480;
  window_info.width = 640;
  window_info.x = 0;
  window_info.y = 0;  

  CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
  extra_info->SetString("interopType", "drawing");
  extra_info->SetString("argStr", args->GetString(3));
  extra_info->SetInt("promiseIdLo", args->GetInt(0));
  extra_info->SetInt("promiseIdHi", args->GetInt(1));
  extra_info->SetInt("parentId", browser->GetIdentifier());
  CefBrowserHost::CreateBrowser(window_info, this, args->GetString(2),
                                browser_settings, extra_info, nullptr);
}

void SimpleHandler::CreateEngineInterop(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefListValue> args) {
  CefBrowserSettings browser_settings;
  browser_settings.file_access_from_file_urls = STATE_ENABLED;
  browser_settings.windowless_frame_rate = 30;  // vsync doesn't matter due to
                                                // external_begin_frame_enabled

  CefWindowInfo window_info;
  window_info.SetAsWindowless(nullptr);
  window_info.shared_texture_enabled = true;
  window_info.external_begin_frame_enabled = true;
  window_info.height = 480;
  window_info.width = 640;
  window_info.x = 0;
  window_info.y = 0;  

  CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
  extra_info->SetString("interopType", "engine");
  extra_info->SetString("argStr", args->GetString(3));
  extra_info->SetInt("promiseIdLo", args->GetInt(0));
  extra_info->SetInt("promiseIdHi", args->GetInt(1));
  extra_info->SetInt("parentId", browser->GetIdentifier());

  CefBrowserHost::CreateBrowser(window_info, this, args->GetString(2),
      browser_settings, extra_info, nullptr);
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::Bind(&SimpleHandler::CloseAllBrowsers, this,
                                   force_close));
    return;
  }

  while (!m_Browsers.empty())
    m_Browsers.begin()->second.Browser->GetHost()->CloseBrowser(force_close);
}


void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
                                CefRect& rect) {

  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
    rect.Set(0, 0, it->second.Width, it->second.Height);
  }
}

void SimpleHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    void* share_handle) {

  MessageBoxA(NULL,"OnAcceleratedPaint\n","Paint",MB_OK|MB_ICONINFORMATION);

  if (PET_VIEW == type) {

    auto it = m_PaintedPromiseIds.find(browser->GetIdentifier());

    if (it != m_PaintedPromiseIds.end()) {
      if (!it->second.empty()) {
        auto message = CefProcessMessage::Create("afx-painted");
        auto args = message->GetArgumentList();
        args->SetSize(4);

        PromiseId_s& promiseId = it->second.front();
        args->SetInt(0, promiseId.Lo);
        args->SetInt(1, promiseId.Hi);
        it->second.pop();

        args->SetInt(2, (int)(((unsigned __int64)share_handle) & 0xffffffff));
        args->SetInt(
            3, (int)(((unsigned __int64)share_handle >> 32) & 0xffffffff));

        browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);
      }
    }
  }
}

void SimpleHandler::SendAfxMessage(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefListValue> args) {

  auto it = m_Browsers.find(args->GetInt(0));
  if (it != m_Browsers.end()) {
    auto sendMessage = CefProcessMessage::Create("afx-message");
    auto sendArgs = sendMessage->GetArgumentList();
    sendArgs->SetSize(2);
    sendArgs->SetInt(0, browser->GetIdentifier());
    sendArgs->SetString(1, args->GetString(1));

    it->second.Browser->GetMainFrame()->SendProcessMessage(PID_RENDERER,
                                                           sendMessage);
  }
}

void SimpleHandler::SendExternalBeginFrame(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefListValue> args) {
  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {

    m_PaintedPromiseIds[browser->GetIdentifier()].emplace(args->GetInt(0),
                                                          args->GetInt(1));

    it->second.Browser->GetHost()->SendExternalBeginFrame();
  }
}

void SimpleHandler::DrawingResized(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefListValue> args) {
  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
    it->second.Width = args->GetInt(0);
    it->second.Height = args->GetInt(1);
    it->second.Browser->GetHost()->WasResized();
  }
}

bool SimpleHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {

      auto const name = message->GetName();

  if (name == "afx-send-message") {

        // TODO: Does this really have to be on the UI thread, or can we short-circuit this?

        //CefPostTask(TID_UI,
        //        base::Bind(&SimpleHandler::SendAfxMessage, this,
         ///                      browser, message->GetArgumentList()->Copy()));

        SendAfxMessage(browser, message->GetArgumentList());

        return true;
  }

  if (name == "afx-drawing-resized") {
    CefPostTask(TID_UI,
                base::Bind(&SimpleHandler::DrawingResized, this,
                           browser, message->GetArgumentList()->Copy()));

    return true;

  }

  if (name == "afx-render-cef-frame") {
    CefPostTask(TID_UI,
                base::Bind(&SimpleHandler::SendExternalBeginFrame, this,
                           browser,
                           message->GetArgumentList()->Copy()));

    return true;
  }

  if (name == "afx-create-drawing") {

    CefPostTask(TID_UI, base::Bind(&SimpleHandler::CreateDrawingInterop, this, browser,
                                   message->GetArgumentList()->Copy()));

    return true;
  }

  if (name == "afx-create-engine") {
    CefPostTask(TID_UI, base::Bind(&SimpleHandler::CreateEngineInterop,
                                   this, browser,
                                   message->GetArgumentList()->Copy()));

    return true;
  }

  if (name == "afx-interop-resolve") {
    auto args = message->GetArgumentList();
    auto it = m_Browsers.find(args->GetInt(0));
    if (it != m_Browsers.end()) {
      auto sendMessage = CefProcessMessage::Create("afx-interop-resolve");
      auto sendArgs = sendMessage->GetArgumentList();
      sendArgs->SetSize(3);
      sendArgs->SetInt(0, args->GetInt(1));
      sendArgs->SetInt(1, args->GetInt(2));
      sendArgs->SetInt(2, browser->GetIdentifier());

      it->second.Browser->GetMainFrame()->SendProcessMessage(PID_RENDERER,
                                                             sendMessage);
    }
  }

  return false;
}