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

  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  // Add to the list of existing browsers.
  m_Browsers.emplace(std::piecewise_construct, std::forward_as_tuple(browser->GetIdentifier()), std::forward_as_tuple(browser));
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (m_Browsers.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  //return false;

  return browser->GetIdentifier() == 1;
}


void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

    std::unique_lock<std::mutex> lock(m_BrowserMutex);

  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
    m_Browsers.erase(it);
  }

  if (m_Browsers.empty()) {

    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  } else {

    bool bHasWindowWithHandle = false;
      
    for (auto it2 = m_Browsers.begin(); it2 != m_Browsers.end(); ++it2) {
      if (it2->second.Browser->GetHost()->GetWindowHandle()) {
        bHasWindowWithHandle = true;
        break;
      }
    }

    // We have no non-offscreen windows open anymore, close all others:
    if (!bHasWindowWithHandle) {
      lock.unlock();
      CloseAllBrowsers(true);
    }
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

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::Bind(&SimpleHandler::CloseAllBrowsers, this,
                                   force_close));
    return;
  }

  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  while (!m_Browsers.empty()) {
    CefRefPtr<CefBrowserHost> browserHost =
        m_Browsers.begin()->second.Browser->GetHost();

    lock.unlock();
    browserHost->CloseBrowser(force_close);
    lock.lock();
  }
}


void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
                                CefRect& rect) {

  rect.Set(0, 0, 640, 480);

  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
      rect.Set(0, 0, it->second.m_Width, it->second.m_Height);
  }
}

void SimpleHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    void* share_handle) {
 
  if (PET_VIEW == type) {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);
    auto it = m_Browsers.find(browser->GetIdentifier());
    if (it != m_Browsers.end()) {
      CefRefPtr<CefBrowser> targetBrowser = it->second.Browser;
      lock.unlock();

      auto targetMessage = CefProcessMessage::Create("afx-render");
      auto targetArgs = targetMessage->GetArgumentList();
      targetArgs->SetInt(0, (int)((unsigned __int64)share_handle & 0xFFFFFFFF));
      targetArgs->SetInt(1, (int)((unsigned __int64)share_handle >> 32));

      targetBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER,
                                                        targetMessage);
    }
  }
}


bool SimpleHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {

    auto name = message->GetName();

    if (name == "afx-message") {
      auto args = message->GetArgumentList();

      auto argTargetId = args->GetInt(0);
      auto argMessage = args->GetString(1);

      std::unique_lock<std::mutex> lock(m_BrowserMutex);
      auto it = m_Browsers.find(argTargetId);
      if (it != m_Browsers.end()) {
        CefRefPtr<CefBrowser> targetBrowser = it->second.Browser;
        lock.unlock();

        auto targetMessage = CefProcessMessage::Create("afx-message");
        auto targetArgs = targetMessage->GetArgumentList();
        targetArgs->SetInt(0, browser->GetIdentifier());
        targetArgs->SetString(1, argMessage);

         targetBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER,
                                                          targetMessage);

         return true;
      }
    } else if (name == "afx-resize") {
      auto args = message->GetArgumentList();
      auto argWidth = args->GetInt(0);
      auto argHeight = args->GetInt(1);

      std::unique_lock<std::mutex> lock(m_BrowserMutex);
      auto it = m_Browsers.find(browser->GetIdentifier());
      if (it != m_Browsers.end()) {
        CefRefPtr<CefBrowser> targetBrowser = it->second.Browser;
        it->second.m_Width = argWidth;
        it->second.m_Height = argHeight;
        lock.unlock();

        browser->GetHost()->WasResized();

        return true;
      }

    } else if (name == "afx-render") {

      std::unique_lock<std::mutex> lock(m_BrowserMutex);
      auto it = m_Browsers.find(browser->GetIdentifier());
      if (it != m_Browsers.end()) {
        CefRefPtr<CefBrowser> targetBrowser = it->second.Browser;
        lock.unlock();

        targetBrowser->GetHost()->SendExternalBeginFrame();

        return true;
      }

    } else if (name == "afx-create-drawing") {
      auto args = message->GetArgumentList();
      auto argUrl = args->GetString(0);
      auto argStr = args->GetString(1);

      CefBrowserSettings browser_settings;
      browser_settings.file_access_from_file_urls = STATE_ENABLED;
      browser_settings.windowless_frame_rate =
          60;  // vsync doesn't matter only if external_begin_frame_enabled
      CefWindowInfo window_info;
      window_info.SetAsWindowless(NULL);
      window_info.shared_texture_enabled = true;
      window_info.external_begin_frame_enabled = true;
      window_info.width = 640;
      window_info.height = 360;

      CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
      extra_info->SetString("interopType", "drawing");
      extra_info->SetString("argStr", argStr);

      CefBrowserHost::CreateBrowser(window_info, this, argUrl, browser_settings,
                                    extra_info, nullptr);

      return true;
    } else if (name == "afx-create-engine") {
      auto args = message->GetArgumentList();
      auto argUrl = args->GetString(0);
      auto argStr = args->GetString(1);

      CefBrowserSettings browser_settings;
      browser_settings.file_access_from_file_urls = STATE_ENABLED;
      browser_settings.windowless_frame_rate = 1;

      CefWindowInfo window_info;
      window_info.SetAsWindowless(NULL);
      window_info.shared_texture_enabled = false;
      window_info.external_begin_frame_enabled = true;
      window_info.width = 640;
      window_info.height = 360;

      CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
      extra_info->SetString("interopType", "engine");
      extra_info->SetString("argStr", argStr);

      CefBrowserHost::CreateBrowser(window_info, this, argUrl, browser_settings,
                                    extra_info, nullptr);

      return true;
    }

  return false;
}