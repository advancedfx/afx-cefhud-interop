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
                   public CefV8Handler {
 public:
  AfxHandler(CefRefPtr<CefBrowser> const& browser,
             CefRefPtr<CefFrame> const& frame,
             CefRefPtr<CefV8Context> const& context)
      : browser_(browser), frame_(frame), context_(context) {

    fn_connect_ = CefV8Value::CreateFunction(
        "connect", this);
    fn_get_connected_ = CefV8Value::CreateFunction("getConnected", this);
    fn_close_ = CefV8Value::CreateFunction("close", this);
    fn_on_render_ = CefV8Value::CreateFunction("onRenderViewBegin", this);
    fn_on_view_override_ = CefV8Value::CreateFunction("onViewOverride", this);
    fn_on_render_pass_ = CefV8Value::CreateFunction("onRenderPass", this);
    fn_on_hud_begin_ = CefV8Value::CreateFunction("onHudBegin", this);
    fn_on_hud_end_ = CefV8Value::CreateFunction("onHudEnd", this);
    fn_on_render_view_end_ = CefV8Value::CreateFunction("onRenderViewEnd", this);
    fn_on_commands_ = CefV8Value::CreateFunction("onCommands", this);
    fn_schedule_command_ = CefV8Value::CreateFunction("scheduleCommand", this);
    fn_schedule_drawing_begin_frame_ =
        CefV8Value::CreateFunction("scheduleDrawingBeginFrame", this);
    fn_schedule_drawing_connect_ = 
        CefV8Value::CreateFunction("scheduleDrawingConnect", this);

    auto const obj = CefV8Value::CreateObject(this, nullptr);
    obj->SetValue("pipeName", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("connect", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("getConnected", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("close", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onCommands", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("scheduleCommand", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onViewOverride", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onRenderViewBegin", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onRenderPass", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onHudBegin", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onHudEnd", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("onRenderViewEnd", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("scheduleDrawingBeginFrame", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("scheduleDrawingConnect", V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);

    auto window = context->GetGlobal();
    window->SetValue("afxInterop", obj, V8_PROPERTY_ATTRIBUTE_NONE);
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) override {
    if (name == "connect") {
      AfxEnsureEngineInterop();
      retval = CefV8Value::CreateBool(engine_interop_->Connection());

      return true;
    }

    if (name == "getConnected") {
      AfxEnsureEngineInterop();
      retval = CefV8Value::CreateBool(engine_interop_->Connected());
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

    if (name == "onCommands") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsFunction()) {
        AfxEnsureEngineInterop();

        engine_interop_->SetCommandsCallback(
            new AfxCommandsCallback(arguments[0], context_));
      }
    }

    if (name == "scheduleCommand") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsString()) {
        AfxEnsureEngineInterop();

        std::string strValue(arguments[0]->GetStringValue().ToString());
        engine_interop_->ScheduleCommand(strValue.c_str());
      }
    }

    if (name == "onViewOverride") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsFunction()) {
        AfxEnsureEngineInterop();

        engine_interop_->SetOnViewOverrideCallback(
            new AfxOnViewOverrideCallback(arguments[0], context_));
      }
    }

    if (name == "onRenderViewBegin") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsFunction()) {
        AfxEnsureEngineInterop();

        engine_interop_->SetRenderViewBeginCallback(
            new AfxRenderViewBeginCallback(arguments[0], context_));
      }
    }

    if (name == "onRenderPass") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsFunction()) {
        AfxEnsureEngineInterop();

        engine_interop_->SetRenderPassCallback(
            new AfxRenderPassCallback(arguments[0], context_));
      }
    }

    if (name == "onHudBegin") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsFunction()) {
        AfxEnsureEngineInterop();

        engine_interop_->SetHudBeginCallback(
            new AfxEventCallback(arguments[0], context_));
      }
    }

    if (name == "onHudEnd") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsFunction()) {
        AfxEnsureEngineInterop();

        engine_interop_->SetHudEndCallback(
            new AfxEventCallback(arguments[0], context_));
      }
    }

    if (name == "onRenderViewEnd") {
      if (1 == arguments.size() && arguments[0] && arguments[0]->IsFunction()) {
        AfxEnsureEngineInterop();

        engine_interop_->SetRenderViewEndCallback(
            new AfxEventCallback(arguments[0], context_));
      }
    }

    if (name == "scheduleDrawingBeginFrame") {
      auto message = CefProcessMessage::Create("afx-begin-frame");
      if (message != nullptr) {
        int numArgs = (int)arguments.size();
        if (0 < numArgs) {
          SetArguments(message->GetArgumentList(), arguments);
        }
        frame_->SendProcessMessage(PID_BROWSER, message);
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
      }
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
    if (name == "connect") {
      retval = fn_connect_;
      return true;
    }
    if (name == "getConnected") {
      retval = fn_get_connected_;
      return true;
    }
    if (name == "close") {
      retval = fn_close_;
      return true;
    }
    if (name == "onCommands") {
      retval = fn_on_commands_;
      return true;
    }
    if (name == "onRenderViewBegin") {
      retval = fn_on_render_;
      return true;
    }
    if (name == "onViewOverride") {
      retval = fn_on_view_override_;
      return true;
    }
    if (name == "onRenderPass") {
      retval = fn_on_render_pass_;
      return true;
    }
    if (name == "onHudBegin") {
      retval = fn_on_hud_begin_;
      return true;
    }
    if (name == "onHudEnd") {
      retval = fn_on_hud_end_;
      return true;
    }
    if (name == "onRenderViewEnd") {
      retval = fn_on_render_view_end_;
      return true;
    }
    if (name == "scheduleCommand") {
      retval = fn_schedule_command_;
      return true;
    }
    if (name == "scheduleDrawingBeginFrame") {
      retval = fn_schedule_drawing_begin_frame_;
      return true;
    }
    if (name == "scheduleDrawingConnect") {
      retval = fn_schedule_drawing_connect_;
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

 protected:
  virtual ~AfxHandler() override {
      if(engine_interop_) engine_interop_->Release();
      fn_connect_ = nullptr;
      fn_get_connected_ = nullptr;
      fn_close_ = nullptr;
      fn_on_commands_ = nullptr;
      fn_on_render_ = nullptr;
      fn_on_view_override_ = nullptr;
      fn_on_render_pass_ = nullptr;
      fn_on_hud_begin_ = nullptr;
      fn_on_hud_end_ = nullptr;
      fn_on_render_view_end_ = nullptr;
      fn_schedule_command_ = nullptr;
      fn_schedule_drawing_begin_frame_ = nullptr;
      fn_schedule_drawing_connect_ = nullptr;
  }

 private:
  static CefRefPtr<CefV8Value> CreateAfxMatrix4x4(
      const struct advancedfx::interop::Matrix4x4_s& value)
       {
    CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(nullptr, nullptr);

         obj->SetValue(
             "m00", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M00)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m01", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M01)),
             V8_PROPERTY_ATTRIBUTE_NONE);
         obj->SetValue(
             "m02", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M02)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m03", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M03)),
             V8_PROPERTY_ATTRIBUTE_NONE);

         obj->SetValue(
             "m10", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M10)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m11", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M11)),
             V8_PROPERTY_ATTRIBUTE_NONE);
         obj->SetValue(
             "m12", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M12)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m13", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M13)),
             V8_PROPERTY_ATTRIBUTE_NONE);

         obj->SetValue(
             "m20", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M20)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m21", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M21)),
             V8_PROPERTY_ATTRIBUTE_NONE);
         obj->SetValue(
             "m22", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M22)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m23", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M23)),
             V8_PROPERTY_ATTRIBUTE_NONE);

         obj->SetValue(
             "m30", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M30)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m31", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M31)),
             V8_PROPERTY_ATTRIBUTE_NONE);
         obj->SetValue(
             "m32", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M32)),
             V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue(
             "m33", CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M33)),
             V8_PROPERTY_ATTRIBUTE_NONE);

    return obj;
  }

  static CefRefPtr<CefV8Value> CreateAfxView(
      const struct advancedfx::interop::View_s& value) {
    CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(nullptr, nullptr);

         obj->SetValue("x", CefV8Value::CreateInt(value.X),
                         V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("y", CefV8Value::CreateInt(value.Y),
                     V8_PROPERTY_ATTRIBUTE_NONE);
         obj->SetValue("width", CefV8Value::CreateInt(value.Width),
                          V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("height", CefV8Value::CreateInt(value.Height),
                     V8_PROPERTY_ATTRIBUTE_NONE);
         obj->SetValue("viewMatrix", CreateAfxMatrix4x4(value.ViewMatrix),
                       V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("projectionMatrix",
                  CreateAfxMatrix4x4(value.ProjectionMatrix),
                     V8_PROPERTY_ATTRIBUTE_NONE);

    return obj;
  }

    static CefRefPtr<CefV8Value> CreateAfxRenderInfo(
      const struct advancedfx::interop::RenderInfo_s& value) {
    
        CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(nullptr, nullptr);

    obj->SetValue("view",
                      CreateAfxView(value.View),
                      V8_PROPERTY_ATTRIBUTE_NONE);

    obj->SetValue("frameCount", CefV8Value::CreateInt(value.FrameCount),
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("absoluteFrameTime",
                  CefV8Value::CreateDouble(value.AbsoluteFrameTime),
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("curTime", CefV8Value::CreateDouble(value.CurTime),
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("frameTime",
                  CefV8Value::CreateDouble(value.FrameTime),
                  V8_PROPERTY_ATTRIBUTE_NONE);

        return obj;
    }

  class AfxRenderViewBeginCallback : public advancedfx::interop::CRefCounted,
                               public advancedfx::interop::IRenderViewBeginCallback {
   public:
    AfxRenderViewBeginCallback(
        CefRefPtr<CefV8Value> callbackFunc,
        CefRefPtr<CefV8Context> callbackContext)
        : m_CallbackFunc(callbackFunc), m_CallbackContext(callbackContext)
       {

    }

     virtual void AddRef() override {
      advancedfx::interop::CRefCounted::AddRef();
    }

    virtual void Release() override {
      advancedfx::interop::CRefCounted::Release();
    }


    virtual void RenderViewBeginCallback(
        const struct advancedfx::interop::RenderInfo_s& renderInfo,
                                bool& outBeforeTranslucentShadow,
                                bool& outAfterTranslucentShadow,
                                bool& outBeforeTranslucent,
                                bool& outAfterTranslucent,
                                bool& outBeforeHud,
                                bool& outAfterHud,
                                bool& outAfterRenderView) override {

      CefV8ValueList args;
      CefRefPtr<CefV8Value> retval;

      args.push_back(CreateAfxRenderInfo(renderInfo));

      retval = m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext,
                                                          NULL, args);

      if (NULL != retval && retval->IsObject()) {
        CefRefPtr<CefV8Value> beforeTranslucentShadow =
            retval->GetValue("beforeTranslucentShadow");
        if (beforeTranslucentShadow && beforeTranslucentShadow->IsBool())
          outBeforeTranslucentShadow = beforeTranslucentShadow->GetBoolValue();

        CefRefPtr<CefV8Value> afterTranslucentShadow =
            retval->GetValue("afterTranslucentShadow");
        if (afterTranslucentShadow && afterTranslucentShadow->IsBool())
          outAfterTranslucentShadow = afterTranslucentShadow->GetBoolValue();

        CefRefPtr<CefV8Value> beforeTranslucent =
            retval->GetValue("beforeTranslucent");
        if (beforeTranslucent && beforeTranslucent->IsBool())
          outBeforeTranslucent = beforeTranslucent->GetBoolValue();

        CefRefPtr<CefV8Value> afterTranslucent =
            retval->GetValue("afterTranslucent");
        if (afterTranslucent && afterTranslucent->IsBool())
          outAfterTranslucent = afterTranslucent->GetBoolValue();

        CefRefPtr<CefV8Value> beforeHud =
            retval->GetValue("beforeHud");
        if (beforeHud && beforeHud->IsBool())
          outBeforeHud = beforeHud->GetBoolValue();

        CefRefPtr<CefV8Value> afterHud =
            retval->GetValue("afterHud");
        if (afterHud && afterHud->IsBool())
          outAfterHud = afterHud->GetBoolValue();

        CefRefPtr<CefV8Value> afterRenderView =
            retval->GetValue("afterRenderView");
        if (afterRenderView && afterRenderView->IsBool())
          outAfterRenderView = afterRenderView->GetBoolValue();
      }
    }

      private:
    CefRefPtr<CefV8Value> m_CallbackFunc;
       CefRefPtr<CefV8Context> m_CallbackContext;
  };

  class AfxRenderPassCallback
      : public advancedfx::interop::CRefCounted,
        public advancedfx::interop::IRenderPassCallback {
   public:
    AfxRenderPassCallback(CefRefPtr<CefV8Value> callbackFunc,
                               CefRefPtr<CefV8Context> callbackContext)
        : m_CallbackFunc(callbackFunc), m_CallbackContext(callbackContext) {}

    virtual void AddRef() override {
      advancedfx::interop::CRefCounted::AddRef();
    }

    virtual void Release() override {
      advancedfx::interop::CRefCounted::Release();
    }

 virtual void RenderPassCallback(
        enum advancedfx::interop::RenderPassType_e pass,
        const struct advancedfx::interop::View_s& view) override {
      CefV8ValueList args;

      args.push_back(CefV8Value::CreateInt((int)pass));
      args.push_back(CreateAfxView(view));

      m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext,
                                                          NULL, args);
    }

   private:
    CefRefPtr<CefV8Value> m_CallbackFunc;
    CefRefPtr<CefV8Context> m_CallbackContext;
  };

  class AfxCommandsCallback : public advancedfx::interop::CRefCounted,
                            public advancedfx::interop::ICommandsCallback {
   public:
    AfxCommandsCallback(CefRefPtr<CefV8Value> callbackFunc,
                      CefRefPtr<CefV8Context> callbackContext)
        : m_CallbackFunc(callbackFunc), m_CallbackContext(callbackContext) {}

    virtual void AddRef() override {
      advancedfx::interop::CRefCounted::AddRef();
    }

    virtual void Release() override {
      advancedfx::interop::CRefCounted::Release();
    }

  virtual void CommandsCallback(
        const class advancedfx::interop::ICommands* commands) override {

      size_t numCommands = commands->GetCommands();
    CefRefPtr<CefV8Value> objCommands = CefV8Value::CreateArray((int)numCommands);
        for (size_t i = 0; i < numCommands; ++i) {
        size_t numArgs = commands->GetArgs(i);
          CefRefPtr<CefV8Value> objArgs =
              CefV8Value::CreateArray((int)numArgs);
        for (size_t j= 0; j < numArgs; ++j) {
            objArgs->SetValue((int)j,
                            CefV8Value::CreateString(commands->GetArg(i, j)));
        }
        objCommands->SetValue((int)i, objArgs);
      }

      CefV8ValueList args;
      args.push_back(objCommands);

      m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext,
                                                          NULL, args);
    }

   private:
    CefRefPtr<CefV8Value> m_CallbackFunc;
    CefRefPtr<CefV8Context> m_CallbackContext;
  };

  class AfxOnViewOverrideCallback : public advancedfx::interop::CRefCounted,
                              public advancedfx::interop::IOnViewOverrideCallback {
   public:
    AfxOnViewOverrideCallback(CefRefPtr<CefV8Value> callbackFunc,
                        CefRefPtr<CefV8Context> callbackContext)
        : m_CallbackFunc(callbackFunc), m_CallbackContext(callbackContext) {}

    virtual void AddRef() override {
      advancedfx::interop::CRefCounted::AddRef();
    }

    virtual void Release() override {
      advancedfx::interop::CRefCounted::Release();
    }

    virtual bool OnViewOverrideCallback(float& Tx,
        float& Ty,
        float& Tz,
        float& Rx,
        float& Ry,
        float& Rz,
        float& Fov) override {

        CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(nullptr, nullptr);

        obj->SetValue("tX", CefV8Value::CreateDouble(Tx),
                      V8_PROPERTY_ATTRIBUTE_NONE);
        obj->SetValue("tY", CefV8Value::CreateDouble(Ty),
                      V8_PROPERTY_ATTRIBUTE_NONE);
        obj->SetValue("tZ", CefV8Value::CreateDouble(Tz),
                      V8_PROPERTY_ATTRIBUTE_NONE);
        obj->SetValue("rX", CefV8Value::CreateDouble(Rx),
                      V8_PROPERTY_ATTRIBUTE_NONE);
        obj->SetValue("rY", CefV8Value::CreateDouble(Ry),
                      V8_PROPERTY_ATTRIBUTE_NONE);
        obj->SetValue("rZ", CefV8Value::CreateDouble(Rz),
                      V8_PROPERTY_ATTRIBUTE_NONE);
        obj->SetValue("fov", CefV8Value::CreateDouble(Fov),
                      V8_PROPERTY_ATTRIBUTE_NONE);

        CefV8ValueList args;
        CefRefPtr<CefV8Value> retval;

        args.push_back(obj);

        retval = m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext,
                                                            NULL, args);

        bool overriden = false;

        if (NULL != retval && retval->IsObject()) {
          CefRefPtr<CefV8Value> v8Tx =
              retval->GetValue("tX");
          if (v8Tx && v8Tx->IsDouble()) {
            Tx = (float)v8Tx->GetDoubleValue();
            overriden = true;
          }

          CefRefPtr<CefV8Value> v8Ty = retval->GetValue("tY");
          if (v8Ty && v8Ty->IsDouble()) {          
            Ty = (float)v8Ty->GetDoubleValue();
            overriden = true;
          }

          CefRefPtr<CefV8Value> v8Tz = retval->GetValue("tZ");
          if (v8Tz && v8Tz->IsDouble()) {          
            Tz = (float)v8Tz->GetDoubleValue();
            overriden = true;
          }

          CefRefPtr<CefV8Value> v8Rx = retval->GetValue("rX");
          if (v8Rx && v8Rx->IsDouble()) {         
            Rx = (float)v8Rx->GetDoubleValue();
            overriden = true;
          }

          CefRefPtr<CefV8Value> v8Ry = retval->GetValue("rY");
          if (v8Ry && v8Ry->IsDouble()) {          
            Ry = (float)v8Ry->GetDoubleValue();
            overriden = true;
          }

          CefRefPtr<CefV8Value> v8Rz = retval->GetValue("rZ");
          if (v8Rz && v8Rz->IsDouble()) {          
            Rz = (float)v8Rz->GetDoubleValue();
            overriden = true;
          }

          CefRefPtr<CefV8Value> v8Fov = retval->GetValue("fov");
          if (v8Fov && v8Fov->IsDouble()) {
            Fov = (float)v8Fov->GetDoubleValue();
            overriden = true;
          }
        }

        return overriden;
    }

   private:
    CefRefPtr<CefV8Value> m_CallbackFunc;
    CefRefPtr<CefV8Context> m_CallbackContext;
  };

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
  CefRefPtr<CefV8Value> fn_connect_;
  CefRefPtr<CefV8Value> fn_get_connected_;
  CefRefPtr<CefV8Value> fn_close_;
  advancedfx::interop::IEngineInterop* engine_interop_ = nullptr;
  CefString pipe_name_ = "advancedfxInterop";
  CefRefPtr<CefV8Value> fn_on_commands_;
  CefRefPtr<CefV8Value> fn_schedule_command_;
  CefRefPtr<CefV8Value> fn_on_view_override_;
  CefRefPtr<CefV8Value> fn_on_render_;
  CefRefPtr<CefV8Value> fn_on_render_pass_;
  CefRefPtr<CefV8Value> fn_on_hud_begin_;
  CefRefPtr<CefV8Value> fn_on_hud_end_;
  CefRefPtr<CefV8Value> fn_on_render_view_end_;
  CefRefPtr<CefV8Value> fn_schedule_drawing_begin_frame_;
  CefRefPtr<CefV8Value> fn_schedule_drawing_connect_;

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
