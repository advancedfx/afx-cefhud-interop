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


SimpleHandler::SimpleHandler(class SimpleApp * simpleApp) : simple_app_(simpleApp) {

  // Create fake browser for GPU process communication:
  // TODO: This is a bit stupid, but okay for now.
  {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);
    m_Browsers.emplace(std::piecewise_construct, std::forward_as_tuple(0),
                       std::forward_as_tuple(nullptr));
  }

  m_WaitConnectionThread =
      std::thread(&SimpleHandler::WaitConnectionThreadHandler, this);
}

SimpleHandler::~SimpleHandler() {
  m_WaitConnectionQuit = true;
  CancelSynchronousIo(m_WaitConnectionThread.native_handle());
  if (m_WaitConnectionThread.joinable())
    m_WaitConnectionThread.join();
}


void SimpleHandler::WaitConnectionThreadHandler(void) {
  std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_handler_");
  strPipeName.append(std::to_string(GetCurrentProcessId()));

   while (!m_WaitConnectionQuit) {
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

  auto emplaced = m_Browsers.emplace(std::piecewise_construct, std::forward_as_tuple(browser->GetIdentifier()), std::forward_as_tuple(browser));
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
  if (m_Browsers.size() == 2) {
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

  if (m_Browsers.size() == 1) {

    // Delete the fake 0 browser:

    auto it = m_Browsers.begin();
    it->second.Connection->DeleteExternal(lock);
    m_Browsers.erase(it);

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
  
  m_LastBrowserId = browser->GetIdentifier();

  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
    if (CHostPipeServerConnectionThread* connection = it->second.Connection) {
      rect.Set(0, 0, connection->GetWidth(), connection->GetHeight());
      return;
    }
  }

  rect.Set(0, 0, 640, 480);
}

void SimpleHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                       PaintElementType type,
                                       const RectList& dirtyRects,
                                       void* share_handle) {
  if (PET_VIEW == type) {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);

    auto emplaced =
        m_Browsers.emplace(std::piecewise_construct,
                           std::forward_as_tuple(browser->GetIdentifier()),
                           std::forward_as_tuple(browser));
//    if (emplaced.second) {

//    } else
      emplaced.first->second.Browser = browser;

    CefPostTask(TID_FILE_USER_BLOCKING,
                base::Bind(&SimpleHandler::DoPainted, this,
                           browser->GetIdentifier(), share_handle));

    //lock.unlock();
    //DoPainted(browser->GetIdentifier(), share_handle);
  }
}

void SimpleHandler::DoPainted(int browserId, void* share_handle) {
  std::unique_lock<std::mutex> lock(m_BrowserMutex);
  auto it = m_Browsers.find(browserId);
  if (it != m_Browsers.end()) {
    m_HandleToBrowserId[share_handle] = browserId;
    it->second.ShareHandles.emplace(share_handle);
    if (CHostPipeServerConnectionThread* connection = it->second.Connection) {
      try {
        connection->OnPainted(share_handle);
      } catch (const std::exception & e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
        DebugBreak();
      }
    }
  }
}

bool SimpleHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            bool user_gesture,
                            bool is_redirect) {

  // Limit afx features to only the allowed requests:

  std::string url = request->GetURL();

  if(0 == url.find("afx://")) {
    bool afx_enabled =
        simple_app_->extra_info_ && 3 == simple_app_->extra_info_->GetSize() ||
        simple_app_->GetAfxEnabled(frame->GetIdentifier());

    if(!afx_enabled) return true; // cancel request, not allowed.
  }

  return false;
}

void SimpleHandler::DoCreateDrawing(const std::string& argStr, const std::string& argUrl) {
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

  simple_app_->extra_info_ = CefListValue::Create();
  simple_app_->extra_info_->SetSize(3);
  simple_app_->extra_info_->SetString(0,"drawing");
  simple_app_->extra_info_->SetInt(1, GetCurrentProcessId());
  simple_app_->extra_info_->SetString(2, argStr);

  CefBrowserHost::CreateBrowser(window_info, this, argUrl, browser_settings,
                                nullptr, nullptr);
}

void SimpleHandler::DoCreateEngine(const std::string& argStr, const std::string& argUrl) {
  CefBrowserSettings browser_settings;
  browser_settings.file_access_from_file_urls = STATE_ENABLED;
  browser_settings.windowless_frame_rate = 1;

  CefWindowInfo window_info;
  window_info.SetAsWindowless(NULL);
  window_info.shared_texture_enabled = false;
  window_info.external_begin_frame_enabled = true;
  window_info.width = 640;
  window_info.height = 360;

  simple_app_->extra_info_ = CefListValue::Create();
  simple_app_->extra_info_->SetSize(3);
  simple_app_->extra_info_->SetString(0,"engine");
  simple_app_->extra_info_->SetInt(1, GetCurrentProcessId());
  simple_app_->extra_info_->SetString(2, argStr);

  CefBrowserHost::CreateBrowser(window_info, this, argUrl, browser_settings,
                                nullptr, nullptr);
}
