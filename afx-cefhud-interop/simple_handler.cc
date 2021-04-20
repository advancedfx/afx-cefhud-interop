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

#include <d3d11.h>

namespace {

 // Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace


SimpleHandler::SimpleHandler() {
  m_WaitConnectionThread =
      std::thread(&SimpleHandler::WaitConnectionThreadHandler, this);
}

void SimpleHandler::WaitConnectionThreadHandler(void) {
  std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_handler_");
  strPipeName.append(std::to_string(GetCurrentProcessId()));

  //MessageBoxA(0, strPipeName.c_str(), "SERVER", MB_OK);

  while (!m_WaitConnectionQuit) {
    try {
      this->WaitForConnection(strPipeName.c_str(), INFINITE);
    } catch (...) {
    }
  }
}

SimpleHandler::~SimpleHandler() {
  m_WaitConnectionQuit = true;
  if (m_WaitConnectionThread.joinable())
    m_WaitConnectionThread.join();
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

  return browser->GetIdentifier() != 1;
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

void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  rect.Set(0, 0, 640, 480);

  std::unique_lock<std::mutex> lock(m_BrowserMutex);

  auto it = m_Browsers.find(browser->GetIdentifier());
  if (it != m_Browsers.end()) {
    if (CHostPipeServerConnectionThread* connection = it->second.Connection) {
      rect.Set(0, 0, connection->GetWidth(), connection->GetHeight());
    }
  }
}

void SimpleHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                       PaintElementType type,
                                       const RectList& dirtyRects,
                                       void* share_handle) {

  if (PET_VIEW == type) {

    CefPostTask(TID_IO, base::Bind(&SimpleHandler::DoPainted, this,
                                   browser->GetIdentifier(), share_handle));
  }
}
  extern ID3D11Texture2D * g_ActiveTexture;

void SimpleHandler::DoPainted(int browserId, void* share_handle) {
  std::unique_lock<std::mutex> lock(m_BrowserMutex);
  auto it = m_Browsers.find(browserId);
  if (it != m_Browsers.end()) {
    if (CHostPipeServerConnectionThread* connection = it->second.Connection) {
      try {
        m_HandleToBrowserId[share_handle] = browserId;
        connection->OnPainted(share_handle);
      } catch (const std::exception e) {
        MessageBoxA(0, e.what(), "Error in simple_handler.cc", MB_OK|MB_ICONERROR);
      }
    }
  }
}
