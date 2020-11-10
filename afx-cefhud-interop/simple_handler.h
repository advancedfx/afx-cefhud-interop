// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include "afx-cefhud-interop/AfxInterop.h"

#include <list>

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefRenderHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler {
 public:
  explicit SimpleHandler();
  ~SimpleHandler();

  // Provide access to the single global instance of this object.
  static SimpleHandler* GetInstance();

  // CefClient methods:

  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }

  virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message) override;

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;

  virtual void OnPaint(CefRefPtr<CefBrowser> /*browser*/,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer,
                       int width,
                       int height) OVERRIDE {
    // this application doesn't support software rasterizing
  }

  // CefRenderHandler methods:
  virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> /*browser*/,
                                  PaintElementType type,
                                  const RectList& dirtyRects,
                                  void* share_handle) OVERRIDE;

  virtual void GetViewRect(CefRefPtr<CefBrowser> /*browser*/,
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
  

  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;

  bool is_closing_;

  int width_ = 640;
  int height_ = 480;
  void* shared_handle_ = 0;
  class advancedfx::interop::IDrawingInterop* drawing_interop_ = nullptr;

  void EnsureDrawingInterop() {
    if (nullptr == drawing_interop_)
      drawing_interop_ =
          advancedfx::interop::CreateDrawingInterop("advancedfxInterop_drawing");
  }

  void AfxSetSize(CefRefPtr<CefBrowser> browser, int width, int height) {

    if (width != width_ || height != height_) {
      width_ = width;
      height_ = height;

      if (browser) {
        browser->GetHost()->WasResized();
      }
    }
  }

  void AfxDrawingInteropSetPipeName(const CefString& pipeName) {
    EnsureDrawingInterop();
    drawing_interop_->SetPipeName(
        pipeName.ToString().append("_drawing").c_str());
  }

  bool AfxDrawingInteropConnection() {
    EnsureDrawingInterop();
    return drawing_interop_->Connection();
  }

  bool AfxDrawingInteropConnection(int frameCount) {
    EnsureDrawingInterop();

    return drawing_interop_->Connection(frameCount);
  }

  void AfxDrawingInteropClose() {
    EnsureDrawingInterop();
    drawing_interop_->Close();
  }

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
