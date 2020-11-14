#pragma once

#include <include/cef_v8.h>

#include <list>
#include <string>

#include <malloc.h>

#include <windows.h>
#include <d3d9types.h>

namespace advancedfx {
namespace interop {

class CEngineInterop : public CefV8Accessor, public CefV8Handler {
 public:
  virtual void CloseInterop() = 0;

  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) = 0;
};

class CEngineInterop* CreateEngineInterop(
      CefRefPtr<CefBrowser> const& browser,
      CefRefPtr<CefFrame> const& frame,
    CefRefPtr<CefV8Context> const& context);


class CDrawingInterop : public CefBaseRefCounted {
 public:
  virtual void CloseInterop() = 0;

  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message) = 0;
  
  virtual void GetViewRect(CefRefPtr<CefBrowser> browser,
                           CefRect& rect) = 0;

  virtual void SetSharedHandle(void * handle) = 0;
};

class CDrawingInterop* CreateDrawingInterop(
    CefRefPtr<CefBrowser> const& browser);

}  // namespace interop
}  // namespace advancedfx
