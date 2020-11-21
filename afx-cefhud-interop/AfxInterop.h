#pragma once

#include <include/cef_base.h>
#include <include/cef_v8.h>

#include <list>
#include <string>

#include <malloc.h>

#include <windows.h>
#include <d3d9types.h>

namespace advancedfx {
namespace interop {

class CInterop : public CefBaseRefCounted {
 public:
  virtual void CloseInterop() = 0;

  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) = 0;
};

CefRefPtr<CefV8Value> CreateInterop(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefV8Context> context,
                                          const CefString& argStr,
                                          CefRefPtr<CInterop>* out = nullptr);

CefRefPtr<CefV8Value> CreateEngineInterop(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context,
    const CefString& argStr,
    CefRefPtr<CInterop>* out = nullptr);

CefRefPtr<CefV8Value> CreateDrawingInterop(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context,
    const CefString& argStr,
    CefRefPtr<CInterop>* out = nullptr);

}  // namespace interop
}  // namespace advancedfx
