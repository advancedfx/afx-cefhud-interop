// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "afx-cefhud-interop/simple_handler.h"

#include "afx-cefhud-interop/simple_app.h"

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

#include <sstream>
#include <string>

#include <d3d11.h>

namespace {

 // Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace


SimpleHandler::SimpleHandler(class SimpleApp * simpleApp) : simple_app_(simpleApp), g_GpuPipeServer(this) {
    m_BrowserWaitConnectionThread =
      std::thread(&SimpleHandler::BrowserWaitConnectionThreadHandler, this);
}

SimpleHandler::~SimpleHandler() {
  m_BrowserWaitConnectionQuit = true;
  CancelSynchronousIo(m_BrowserWaitConnectionThread.native_handle());
  if (m_BrowserWaitConnectionThread.joinable())
    m_BrowserWaitConnectionThread.join();
}


void SimpleHandler::BrowserWaitConnectionThreadHandler(void) {
  std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_handler_");
  strPipeName.append(std::to_string(GetCurrentProcessId()));

   while (!m_BrowserWaitConnectionQuit) {
    try {
      this->WaitForConnection(strPipeName.c_str(), 500);
    } catch (const std::exception& e) {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": " << e.what();
      DebugBreak();
    }
  }
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  PlatformTitleChange(browser, title);
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  if (NULL == browser->GetHost()->GetWindowHandle() &&
      60 == browser->GetHost()->GetWindowlessFrameRate()) {
  }

  auto emplaced = m_Browsers.emplace(
      std::piecewise_construct, std::forward_as_tuple(browser->GetIdentifier()),
      std::forward_as_tuple(browser));
  if (emplaced.second) {

  }
  else
    emplaced.first->second.Browser = browser;
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

  return false;
}


void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  {
    auto it = m_Browsers.find(browser->GetIdentifier());
    if (it != m_Browsers.end()) {
      auto connection = it->second.Connection;
      it->second.Connection = nullptr;
      m_Browsers.erase(it);
      if (connection) {
        connection->DeleteExternal(lock);
      }
    }
  }

  if (m_Browsers.size() == 0) {

    browser = nullptr;
    simple_app_ = nullptr;

    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  } else {
    bool bHasWindowWithHandle = false;

    for (auto it = m_Browsers.begin(); it != m_Browsers.end(); ++it) {
      if (nullptr != it->second.Browser && it->second.Browser->GetHost()
                         ->GetWindowHandle()) {
        bHasWindowWithHandle = true;
        break;
      }
    }

    // TODO: Actually a window should close it's "child" windows and not the way it is now.
    // We have no non-offscreen windows open anymore, close all others:
    if (!bHasWindowWithHandle) {
      if (1 < m_Browsers.size()) {
        for (auto it = m_Browsers.begin(); it != m_Browsers.end(); ++it) {
          if (nullptr != it->second.Browser) {
            lock.unlock();
            it->second.Browser->GetHost()->CloseBrowser(true);
            return;  // CEF should call into this function later again.
          }        
        }       
      }
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

void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
    if (CHostPipeServerConnectionThread* connection = it->second.Connection) {
      rect.Set(0, 0, connection->GetWidth(), connection->GetHeight());
      return;
    }
  }

  rect.Set(0, 0, 640, 480);
}

bool SimpleHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            bool user_gesture,
                            bool is_redirect) {

  // Limit afx features to only the allowed requests:

  std::string url = request->GetURL();

  if(0 == url.find("afx://")) {
    bool afx_enabled = m_Creating || simple_app_->GetAfxEnabled(frame->GetIdentifier());

    if(!afx_enabled) return true; // cancel request, not allowed.
  }

  m_Creating = false; // Maybe this can be exploited with some kind of race condition :thinking:.

  return false;
}

void SimpleHandler::DoCreateDrawing(const std::string& argStr, const std::string& argUrl) {
  CefBrowserSettings browser_settings;
  //browser_settings.file_access_from_file_urls = STATE_ENABLED;
  browser_settings.windowless_frame_rate = 60; // vsync doesn't matter only if external_begin_frame_enabled
  CefWindowInfo window_info;
  window_info.SetAsWindowless(NULL);
  window_info.shared_texture_enabled = true;
  window_info.external_begin_frame_enabled = true;
  window_info.width = 640;
  window_info.height = 360;

  auto extra_info = CefDictionaryValue::Create();
  extra_info->SetString("interopType", "drawing");
  extra_info->SetInt("handlerProcessId", GetCurrentProcessId());
  extra_info->SetString("argStr", argStr);

  m_Creating = true;
  CefBrowserHost::CreateBrowser(window_info, this, argUrl, browser_settings,
                                extra_info, nullptr);
}

void SimpleHandler::DoCreateEngine(const std::string& argStr, const std::string& argUrl) {
  CefBrowserSettings browser_settings;
  //browser_settings.file_access_from_file_urls = STATE_ENABLED;
  browser_settings.windowless_frame_rate = 1;

  CefWindowInfo window_info;
  window_info.SetAsWindowless(NULL);
  window_info.shared_texture_enabled = false;
  window_info.external_begin_frame_enabled = true;
  window_info.width = 640;
  window_info.height = 360;

  auto extra_info = CefDictionaryValue::Create();
  extra_info->SetString("interopType", "engine");
  extra_info->SetInt("handlerProcessId", GetCurrentProcessId());
  extra_info->SetString("argStr", argStr);

  m_Creating = true;
  CefBrowserHost::CreateBrowser(window_info, this, argUrl, browser_settings,
                                extra_info, nullptr);
}

bool SimpleHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  auto name = message->GetName();

  if (name == "afx-lock") {
    {
      std::unique_lock<std::mutex> lock(m_BrowserMutex);
      auto it = m_Browsers.find(browser->GetIdentifier());
      if (it != m_Browsers.end()) {
        if (auto connection = it->second.Connection) {
          connection->Lock();
        }
      }
    }
    auto response = CefProcessMessage::Create("afx-ack");
    browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, response);
    return true;
  } else if (name == "afx-paint") {
    CefPostTask(TID_UI, new advancedfx::interop::CAfxTask([this, browser] {
                  browser->GetHost()->Invalidate(PET_VIEW);
                  browser->GetHost()->SendExternalBeginFrame();
                }));
  } else if (name == "afx-use-clear") {
    {
      auto args = message->GetArgumentList();
      bool useClearTexture = args->GetBool(0);
      std::unique_lock<std::mutex> lock(m_BrowserMutex);
      auto it = m_Browsers.find(browser->GetIdentifier());
      if (it != m_Browsers.end()) {
        if (auto connection = it->second.Connection) {
          connection->SetUseClear(useClearTexture);
        }
      }
    }
    auto response = CefProcessMessage::Create("afx-ack");
    browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, response);
    return true;
  } else if (name == "afx-unlock") {
    {
      auto args = message->GetArgumentList();
      bool updateClearTexture = args->GetBool(0);
      std::unique_lock<std::mutex> lock(m_BrowserMutex);
      auto it = m_Browsers.find(browser->GetIdentifier());
      if (it != m_Browsers.end()) {
        if (auto connection = it->second.Connection) {
          connection->Unlock(updateClearTexture);
        }
      }
    }
    auto response = CefProcessMessage::Create("afx-ack");
    browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, response);
    return true;
  }
  else if (name == "afx-set-frame-rate") {
    auto args = message->GetArgumentList();
    int fps = args->GetInt(0);

    browser->GetHost()->SetWindowlessFrameRate(fps);
    return true;
  } else if (name == "afx-set-size") {
    auto args = message->GetArgumentList();
    int width = args->GetInt(0);
    int height = args->GetInt(1);

    std::unique_lock<std::mutex> lock(m_BrowserMutex);
    auto it = m_Browsers.find(browser->GetIdentifier());
    if (it != m_Browsers.end()) {
      if (auto connection = it->second.Connection) {
        connection->SetSize(width, height);
        lock.unlock();
        browser->GetHost()->WasResized();
        return true;
      }
    }
  }

  return false;
}