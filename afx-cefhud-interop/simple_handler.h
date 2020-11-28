// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include <tchar.h>

#include <map>
#include <list>

#include <mutex>
#include <condition_variable>
#include <thread>

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefRenderHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler {
 public:
  explicit SimpleHandler();
  ~SimpleHandler();

  // CefClient methods:

  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }

    virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE;


  // CefDisplayHandler methods:

  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;

   virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer,
                       int width,
                       int height) OVERRIDE {
  }

     // CefRenderHandler methods:
  virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                  PaintElementType type,
                                  const RectList& dirtyRects,
                                  void* share_handle) OVERRIDE;

  virtual void GetViewRect(CefRefPtr<CefBrowser> browser,
                           CefRect& rect) OVERRIDE;

    // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods:

  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;
  
  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }
 private:

  bool is_closing_;

  struct BrowserMapElem {
    CefRefPtr<CefBrowser> Browser;
    int Width = 640;
    int Height = 480;

    BrowserMapElem(CefRefPtr<CefBrowser> browser) : Browser(browser) {}
  };

  std::mutex m_BrowsesMutex;
  std::map<int, BrowserMapElem> m_Browsers;

  std::map<int, int> m_BrowserIdToClientId;

  struct PaintRequest_s {
    int Lo;
    int Hi;
    PaintRequest_s(int lo, int hi) : Lo(lo), Hi(hi) {}
    PaintRequest_s(const PaintRequest_s & copyFrom)
        : Lo(copyFrom.Lo), Hi(copyFrom.Hi) {}
  };

  std::mutex m_PaintRequestsMuetx;
  std::map<int,std::list<PaintRequest_s>> m_PaintRequests;


  void SendAfxPainted(int id, int lo, int hi, int shareLo, int shareHi);

   // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  void CreateDrawingInterop(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefListValue> args);

  void CreateEngineInterop(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefListValue> args);

// Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
