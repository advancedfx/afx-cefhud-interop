// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_


#include <include/cef_app.h>
#include <include/cef_parser.h>

#include <list>
#include <set>
#include <map>
#include <mutex>

namespace advancedfx {
  namespace interop {
    class CInterop;
  }
}

// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler {
 public:
  SimpleApp();
  ~SimpleApp() {

  }

  bool GetAfxEnabled(int frameId) {
    std::unique_lock<std::mutex> lock(m_InteropMutex);
    return m_Interops.find(frameId) != m_Interops.end();
  }

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
      OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
      OVERRIDE {
    return this;
  }
    
  virtual void SimpleApp::OnRegisterCustomSchemes(
      CefRawPtr<CefSchemeRegistrar> registrar) OVERRIDE;

  // CefBrowserProcessHandler methods:

  virtual void OnContextInitialized() OVERRIDE;

  virtual void OnBrowserCreated( CefRefPtr< CefBrowser > browser, CefRefPtr< CefDictionaryValue > _extra_info ) OVERRIDE {
    extra_info_ = _extra_info->Copy(false);
  }
  
  // CefRenderProcessHandler methods:

  virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context) OVERRIDE;

  virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) OVERRIDE;

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message) OVERRIDE;

  // CefApp methods:
  virtual void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) OVERRIDE;

 private:
  std::map<int,CefRefPtr<class advancedfx::interop::CInterop>> m_Interops;
  std::mutex m_InteropMutex; //TODO: maybe turn this into a shared mutex?
  CefRefPtr<CefDictionaryValue> extra_info_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
