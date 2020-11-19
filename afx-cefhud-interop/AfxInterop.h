#pragma once

#include <include/cef_v8.h>

#include <list>
#include <string>

#include <malloc.h>

#include <windows.h>
#include <d3d9types.h>

namespace advancedfx {
namespace interop {

class CInterop : public virtual CefBaseRefCounted {
 public:
  virtual void CloseInterop() = 0;

  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) = 0;
};

class CInterop* CreateInterop(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefV8Context> context, const CefString & argStr);

class CInterop* CreateEngineInterop(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context,
                                    const CefString& argStr);

class CInterop* CreateDrawingInterop(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context,
                                     const CefString& argStr);

}  // namespace interop
}  // namespace advancedfx
