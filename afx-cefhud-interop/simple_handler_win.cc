// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>

#include <string>

#include "afx-cefhud-interop/simple_handler.h"
#include "include/cef_browser.h"

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                                        const CefString& title) {
  CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
  SetWindowText(hwnd, std::wstring(title).c_str());
}

void SimpleHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> /*browser*/,
                                       PaintElementType type,
                                       const RectList& dirtyRects,
                                       void* share_handle) {

  switch (type) {
        case PET_VIEW:
            shared_handle_ = share_handle;
            break;
        case PET_POPUP:
            break;
    }
}