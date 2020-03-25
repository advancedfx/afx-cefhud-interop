// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"

#include "afx-cefhud-interop/AfxInterop.h"

//
// V8 handler for our 'afx' object available to javascript
// running in a page within this application
//
class AfxHandler : public CefV8Accessor,
                   public CefV8Handler,
                   public advancedfx::interop::IRenderCallback {
 public:
  AfxHandler(CefRefPtr<CefBrowser> const& browser,
             CefRefPtr<CefFrame> const& frame,
             CefRefPtr<CefV8Context> const& context)
      : browser_(browser), frame_(frame), context_(context) {

    fn_process_ = CefV8Value::CreateFunction(
        "process", this);
    fn_close_ = CefV8Value::CreateFunction("close", this);

    auto window = context->GetGlobal();
    auto const obj = CefV8Value::CreateObject(this, nullptr);
    obj->SetValue("pipeName", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("process", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("close", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    window->SetValue("afxInterop", obj, V8_PROPERTY_ATTRIBUTE_NONE);
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) override {
    if (name == "process") {
      AfxEnsureEngineInterop();
      retval = CefV8Value::CreateBool(engine_interop_->Connection());

      if (!engine_interop_->Connected()) {
        auto message = CefProcessMessage::Create("afx-process");
        if (message != nullptr && browser_ != nullptr) {
          frame_->SendProcessMessage(PID_BROWSER, message);
        }      
      }

      return true;
    }

    if (name == "close") {
      AfxEnsureEngineInterop();

      engine_interop_->Close();

      auto message = CefProcessMessage::Create("afx-close");
      if (message != nullptr && browser_ != nullptr) {
        frame_->SendProcessMessage(PID_BROWSER, message);
      }

      return true;
    }

    return false;
  }

  bool Get(const CefString& name,
           const CefRefPtr<CefV8Value> object,
           CefRefPtr<CefV8Value>& retval,
           CefString& /*exception*/) override {
    if (name == "pipeName") {
      retval = CefV8Value::CreateString(pipe_name_);
      return true;
    }
    if (name == "process") {
      retval = fn_process_;
      return true;
    }
    if (name == "pipeName") {
      retval = fn_close_;
      return true;
    }


    // Value does not exist.
    return false;
  }

  bool Set(const CefString& name,
           const CefRefPtr<CefV8Value> object,
           const CefRefPtr<CefV8Value> value,
           CefString& /*exception*/) override {
    if (name == "pipeName" && value->IsString()) {
      pipe_name_ = value->GetStringValue();
      AfxEnsureEngineInterop();
      engine_interop_->SetPipeName(pipe_name_.ToString().c_str());

      // notify the browser process that we want stats
      auto message = CefProcessMessage::Create("afx-set-pipename");
      if (message != nullptr && browser_ != nullptr) {
        frame_->SendProcessMessage(PID_BROWSER, message);
      }
      return true;
    }
    return false;
  }

  virtual void advancedfx::interop::IRenderCallback::AddRef() {
  }

  virtual void advancedfx::interop::IRenderCallback::Release() {
  }

  virtual void advancedfx::interop::IRenderCallback::RenderCallback(
      const struct advancedfx::interop::RenderInfo_s& renderInfo,
      bool& outBeforeTranslucentShadow,
      bool& outAfterTranslucentShadow,
      bool& outBeforeTranslucent,
      bool& outAfterTranslucent,
      bool& outBeforeHud,
      bool& outAfterHud) {
      
    outAfterHud = true;

    auto message = CefProcessMessage::Create("afx-process");
    if (message != nullptr && browser_ != nullptr) {
        auto messageArgs = message->GetArgumentList();
        messageArgs->SetSize(3);
        messageArgs->SetInt(0, renderInfo.FrameCount);
        messageArgs->SetInt(1, renderInfo.View.Width);
        messageArgs->SetInt(2, renderInfo.View.Height);
        frame_->SendProcessMessage(PID_BROWSER, message);
    }
  }

 protected:
  virtual ~AfxHandler() override {
      if(engine_interop_) engine_interop_->Release();
      fn_process_ = nullptr;
      fn_close_ = nullptr;
  }

 private:
  CefRefPtr<CefBrowser> const browser_;
  CefRefPtr<CefFrame> const frame_;
  CefRefPtr<CefV8Context> const context_;
  CefRefPtr<CefV8Value> fn_process_;
  CefRefPtr<CefV8Value> fn_close_;
  advancedfx::interop::IEngineInterop* engine_interop_ = nullptr;
  CefString pipe_name_ = "advancedfxInterop";

  void AfxEnsureEngineInterop() {
    if (nullptr == engine_interop_) {
      engine_interop_ =
          advancedfx::interop::CreateEngineInterop("advancedfxInterop");
      engine_interop_->SetRenderCallback(this);
    }
  }

  IMPLEMENT_REFCOUNTING(AfxHandler);
};

// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler {
 public:
  SimpleApp();

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
      OVERRIDE {
    return this;
  }

  virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
      OVERRIDE {
    return this;
  }

  // CefBrowserProcessHandler methods:
  virtual void OnContextInitialized() OVERRIDE;

  // CefRenderProcessHandler methods:
  virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context) OVERRIDE {

    afx_handler_ = new AfxHandler(browser, frame, context);
  }
  
  virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) OVERRIDE {
    afx_handler_ = nullptr;
  }
  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE;

  // CefApp methods:
   virtual void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) OVERRIDE {
    // disable creation of a GPUCache/ folder on disk
    command_line->AppendSwitch("disable-gpu-shader-disk-cache");

    // command_line->AppendSwitch("disable-accelerated-video-decode");

    // un-comment to show the built-in Chromium fps meter
    // command_line->AppendSwitch("show-fps-counter");

    // command_line->AppendSwitch("disable-gpu-vsync");

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
  }

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);

  CefRefPtr<AfxHandler> afx_handler_ = nullptr;
  advancedfx::interop::IDrawingInterop* drawing_interop_ = nullptr;
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
