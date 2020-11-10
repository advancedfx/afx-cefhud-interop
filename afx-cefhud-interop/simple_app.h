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
class AfxHandler : public CefV8Accessor, public CefV8Handler {
 public:
  AfxHandler(CefRefPtr<CefBrowser> const& browser,
             CefRefPtr<CefFrame> const& frame,
             CefRefPtr<CefV8Context> const& context)
      : browser_(browser), frame_(frame), context_(context) {;
    fn_schedule_drawing_begin_frame_ =
        CefV8Value::CreateFunction("scheduleDrawingBeginFrame", this);
    fn_schedule_drawing_connect_ =
        CefV8Value::CreateFunction("scheduleDrawingConnect", this);
    fn_game_event_allow_add =
        CefV8Value::CreateFunction("gameEventAllowAdd", this);
    fn_game_event_allow_remove =
        CefV8Value::CreateFunction("gameEventAllowRemove", this);
    fn_game_event_deny_add =
        CefV8Value::CreateFunction("gameEventDenyAdd", this);
    fn_game_event_deny_remove =
        CefV8Value::CreateFunction("gameEventDenyRemove", this);
    fn_game_event_set_enrichment_ =
        CefV8Value::CreateFunction("gameEventSetEnrichment", this);
    fn_on_game_event_ = CefV8Value::CreateFunction("onGameEvent", this);
    fn_game_event_set_transmit_client_time_ =
        CefV8Value::CreateFunction("gameEventSetTransmitClientTime", this);
    fn_game_event_set_transmit_tick_ =
        CefV8Value::CreateFunction("gameEventSetTransmitTick", this);
    fn_game_event_set_transmit_system_time_ =
        CefV8Value::CreateFunction("gameEventSetTransmitSystemTime", this);

    auto const obj = CefV8Value::CreateObject(this, nullptr);

    obj->SetValue("scheduleDrawingBeginFrame", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("scheduleDrawingConnect", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventAllowAdd", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventAllowRemove", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventDenyAdd", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventDenyRemove", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventSetEnrichment", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onGameEvent", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventSetTransmitClientTime", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventSetTransmitTick", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("gameEventSetTransmitSystemTime", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);

    auto window = context->GetGlobal();
    window->SetValue("afxInterop", obj, V8_PROPERTY_ATTRIBUTE_NONE);
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) override {

    if (name == "scheduleDrawingBeginFrame") {
      auto message = CefProcessMessage::Create("afx-begin-frame");
      if (message != nullptr) {
        int numArgs = (int)arguments.size();
        if (0 < numArgs) {
          SetArguments(message->GetArgumentList(), arguments);
        }
        frame_->SendProcessMessage(PID_BROWSER, message);

        return true;
      }
    }

    if (name == "scheduleDrawingConnect") {
      auto message = CefProcessMessage::Create("afx-connect");
      if (message != nullptr) {
        int numArgs = (int)arguments.size();
        if (0 < numArgs) {
          SetArguments(message->GetArgumentList(), arguments);
        }
        frame_->SendProcessMessage(PID_BROWSER, message);

        return true;
      }
    }
  bool Get(const CefString& name,
           const CefRefPtr<CefV8Value> object,
           CefRefPtr<CefV8Value>& retval,
           CefString& /*exception*/) override {
     if (name == "scheduleDrawingBeginFrame") {
      retval = fn_schedule_drawing_begin_frame_;
      return true;
    }
    if (name == "scheduleDrawingConnect") {
      retval = fn_schedule_drawing_connect_;
      return true;
    }
    if (name == "gameEventAllowAdd") {
      retval = fn_game_event_allow_add;
      return true;
    }
    if (name == "gameEventAllowRemove") {
      retval = fn_game_event_allow_remove;
      return true;
    }
    if (name == "gameEventDenyAdd") {
      retval = fn_game_event_deny_add;
      return true;
    }
    if (name == "gameEventDenyRemove") {
      retval = fn_game_event_deny_remove;
      return true;
    }
    if (name == "gameEventSetEnrichment") {
      retval = fn_game_event_set_enrichment_;
      return true;
    }
    if (name == "onGameEvent") {
      retval = fn_on_game_event_;
      return true;
    }
    if (name == "gameEventSetTransmitClientTime") {
      retval = fn_game_event_set_transmit_client_time_;
      return true;
    }
    if (name == "gameEventSetTransmitTick") {
      retval = fn_game_event_set_transmit_tick_;
      return true;
    }
    if (name == "gameEventSetTransmitSystemTime") {
      retval = fn_game_event_set_transmit_system_time_;
      return true;
    }

    // Value does not exist.
    return false;
  }

 protected:
  virtual ~AfxHandler() override {
    fn_on_hud_begin_ = nullptr;
    fn_on_hud_end_ = nullptr;
    fn_on_render_view_end_ = nullptr;
    fn_schedule_drawing_begin_frame_ = nullptr;
    fn_schedule_drawing_connect_ = nullptr;
    fn_game_event_allow_add = nullptr;
    fn_game_event_allow_remove = nullptr;
    fn_game_event_deny_add = nullptr;
    fn_game_event_deny_remove = nullptr;
    fn_game_event_set_enrichment_ = nullptr;
    fn_on_game_event_ = nullptr;
    fn_game_event_set_transmit_client_time_ = nullptr;
    fn_game_event_set_transmit_tick_ = nullptr;
    fn_game_event_set_transmit_system_time_ = nullptr;
  }

 private:
 
 class AfxEventCallback
      : public advancedfx::interop::CRefCounted,
        public advancedfx::interop::IEventCallback {
   public:
    AfxEventCallback(CefRefPtr<CefV8Value> callbackFunc,
                          CefRefPtr<CefV8Context> callbackContext)
        : m_CallbackFunc(callbackFunc), m_CallbackContext(callbackContext) {}

    virtual void AddRef() override {
      advancedfx::interop::CRefCounted::AddRef();
    }

    virtual void Release() override {
      advancedfx::interop::CRefCounted::Release();
    }

    virtual void EventCallback() override {
      CefV8ValueList args;

      m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext, NULL, args);
    }

   private:
    CefRefPtr<CefV8Value> m_CallbackFunc;
    CefRefPtr<CefV8Context> m_CallbackContext;
  };

  CefRefPtr<CefBrowser> const browser_;
  CefRefPtr<CefFrame> const frame_;
  CefRefPtr<CefV8Context> const context_;
  CefRefPtr<CefV8Value> fn_on_hud_begin_;
  CefRefPtr<CefV8Value> fn_on_hud_end_;
  CefRefPtr<CefV8Value> fn_on_render_view_end_;
  CefRefPtr<CefV8Value> fn_schedule_drawing_begin_frame_;
  CefRefPtr<CefV8Value> fn_schedule_drawing_connect_;
  CefRefPtr<CefV8Value> fn_game_event_allow_add;
  CefRefPtr<CefV8Value> fn_game_event_allow_remove;
  CefRefPtr<CefV8Value> fn_game_event_deny_add;
  CefRefPtr<CefV8Value> fn_game_event_deny_remove;
  CefRefPtr<CefV8Value> fn_game_event_set_enrichment_;
  CefRefPtr<CefV8Value> fn_on_game_event_;
  CefRefPtr<CefV8Value> fn_game_event_set_transmit_client_time_;
  CefRefPtr<CefV8Value> fn_game_event_set_transmit_tick_;
  CefRefPtr<CefV8Value> fn_game_event_set_transmit_system_time_;

  void AfxEnsureEngineInterop() {
    if (nullptr == engine_interop_) {
      engine_interop_ =
          advancedfx::interop::CreateEngineInterop("advancedfxInterop");
    }
  }

void SetArguments(CefRefPtr<CefListValue> list, const CefV8ValueList& arguments) {
    size_t numArgs = arguments.size();
    list->SetSize(numArgs);
    for (int i = 0; i < numArgs; ++i) {
      CefRefPtr<CefV8Value> value = arguments[i];
      if (value) {
        if (value->IsNull()) {
          list->SetNull(i);
        } else if (value->IsBool()) {
          list->SetBool(i, value->GetBoolValue());
        } else if (value->IsInt()) {
          list->SetInt(i, value->GetIntValue());
        } else if (value->IsDouble()) {
          list->SetDouble(i, value->GetDoubleValue());
        } else if (value->IsString()) {
          list->SetString(i, value->GetStringValue());
        } else {
          list->SetNull(i);
        }
      } else
        list->SetNull(i);
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
