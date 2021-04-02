// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"
#include <include/base/cef_bind.h>
#include "include/wrapper/cef_closure_task.h"

#include "AfxInterop.h"

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
                      public CefLoadHandler,
                      private advancedfx::interop::CPipeServer {
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

protected:
  class CHostPipeServerConnectionThread;

  virtual advancedfx::interop::CPipeServerConnectionThread* OnNewConnection(
      HANDLE handle) override {
    return new CHostPipeServerConnectionThread(handle, this);
  }

 private:
     CefRefPtr<CefBrowser> AddBrowserConnection(
      int browserId,
      class CHostPipeServerConnectionThread * connection) {
        std::unique_lock<std::mutex> lock(m_BrowserMutex);
       auto it = m_Browsers.find(browserId);
        if (it == m_Browsers.end())
          throw "CHostPipeServerConnectionThread::AddBrowserConnection failed: no browser.";

       it->second.Connection = connection;

       return it->second.Browser;
     }

     void RemoveBrowserConnection(int browserId) {
        std::unique_lock<std::mutex> lock(m_BrowserMutex);
        auto it = m_Browsers.find(browserId);
        if (it == m_Browsers.end())
          throw "CHostPipeServerConnectionThread::RemoveBrowserConnection failed: no browser.";

       it->second.Connection = nullptr;
     }

     void Message(int senderId, int targetId, const std::string & message) {
        std::unique_lock<std::mutex> lock(m_BrowserMutex);
        auto it = m_Browsers.find(targetId);
        if (it == m_Browsers.end())
          throw "CHostPipeServerConnectionThread::Message failed: no target browser.";

        if(it->second.Connection)
         it->second.Connection->Message(senderId, message);
        else
          throw "CHostPipeServerConnectionThread::Message failed: no target connection.";
     }

    void DoDrawingResized(CefRefPtr<CefBrowser> browser) {

       browser->GetHost()->WasResized();
     }

     void DoRenderFrame(CefRefPtr<CefBrowser> browser) {
       browser->GetHost()->SendExternalBeginFrame();
     }

     void DoCreateDrawing(const std::string& argStr,
                          const std::string& argUrl) {

       CefBrowserSettings browser_settings;
       browser_settings.file_access_from_file_urls = STATE_ENABLED;
       browser_settings.windowless_frame_rate =
           60;  // vsync doesn't matter only if external_begin_frame_enabled
       CefWindowInfo window_info;
       window_info.SetAsWindowless(NULL);
       window_info.shared_texture_enabled = true;
       window_info.external_begin_frame_enabled = true;
       window_info.width = 640;
       window_info.height = 360;

       CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
       extra_info->SetString("interopType", "drawing");
       extra_info->SetString("argStr", argStr);
       extra_info->SetInt("handlerId", (int)GetCurrentProcessId());

       CefBrowserHost::CreateBrowser(window_info, this, argUrl,
                                     browser_settings, extra_info, nullptr);
     }

     void DoCreateEngine(const std::string& argStr, const std::string& argUrl) {
       CefBrowserSettings browser_settings;
       browser_settings.file_access_from_file_urls = STATE_ENABLED;
       browser_settings.windowless_frame_rate = 1;

       CefWindowInfo window_info;
       window_info.SetAsWindowless(NULL);
       window_info.shared_texture_enabled = false;
       window_info.external_begin_frame_enabled = true;
       window_info.width = 640;
       window_info.height = 360;

       CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
       extra_info->SetString("interopType", "engine");
       extra_info->SetString("argStr", argStr);
       extra_info->SetInt("handlerId", GetCurrentProcessId());

       CefBrowserHost::CreateBrowser(window_info, this, argUrl,
                                     browser_settings, extra_info, nullptr);
     }


 protected:
     class CHostPipeServerConnectionThread
      : public advancedfx::interop::CPipeServerConnectionThread {
   public:

      CHostPipeServerConnectionThread(HANDLE handle, SimpleHandler * host)
           : advancedfx::interop::CPipeServerConnectionThread(handle), m_Host(host) {}


      ~CHostPipeServerConnectionThread() {
        m_ClientConnection.ClosePipe();
      }

      int GetWidth() { return m_Width;
      }

      int GetHeight() { return m_Height;
      }

      /**
       * @throws exception
       */
      void OnPainted(void* share_handle) {

        std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
        m_ClientConnection.WriteInt32((int)advancedfx::interop::ClientMessage::TextureHandle);
        m_ClientConnection.WriteUInt64((UINT64)share_handle);
        m_ClientConnection.Flush();
      }

      /**
       * @throws exception
       */
      void Message(int senderId, const std::string & message) {
        std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
        m_ClientConnection.WriteInt32((int)advancedfx::interop::ClientMessage::Message);
        m_ClientConnection.WriteInt32(senderId);
        m_ClientConnection.WriteStringUTF8(message);
        m_ClientConnection.Flush();
      }

    /**
     * @throws exception
     */
    virtual void HandleConnection() override {

      DWORD processId;
      int browserId;

      processId = ReadUInt32();
      browserId = ReadInt32();     

      std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_client_");
      strPipeName.append(std::to_string(processId));
      strPipeName.append("_");
      strPipeName.append(std::to_string(browserId));

      m_ClientConnection.OpenPipe(strPipeName.c_str(), 3000);

      try {
        m_Browser = m_Host->AddBrowserConnection(browserId, this);

        while (true) {
          advancedfx::interop::HostMessage message = (advancedfx::interop::HostMessage)ReadInt32();
          switch (message) {
          case advancedfx::interop::HostMessage::Quit:
            {
              std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
              m_ClientConnection.WriteInt32((int)advancedfx::interop::ClientMessage::Quit);
              m_ClientConnection.Flush();
              m_ClientConnection.ClosePipe();
            }
            return;
          case advancedfx::interop::HostMessage::DrawingResized: {
              m_Width = ReadInt32();
              m_Height = ReadInt32();

              CefPostTask(
                  TID_UI, base::Bind(&SimpleHandler::DoDrawingResized,
                             m_Host, m_Browser));
          } break;
          case advancedfx::interop::HostMessage::RenderFrame: {
            CefPostTask(
                TID_UI, base::Bind(&SimpleHandler::DoRenderFrame,
                           m_Host, m_Browser));
          } break;
          case advancedfx::interop::HostMessage::CreateDrawing: {

            std::string argUrl;
            ReadStringUTF8(argUrl);

            std::string argStr;
            ReadStringUTF8(argStr);

            CefPostTask(TID_UI, base::Bind(&SimpleHandler::DoCreateDrawing,
                                           m_Host, argStr, argUrl));
          } break;
          case advancedfx::interop::HostMessage::CreateEngine: {

            std::string argUrl;
            ReadStringUTF8(argUrl);

            std::string argStr;
            ReadStringUTF8(argStr);

            CefPostTask(TID_UI,
                base::Bind(&SimpleHandler::DoCreateEngine,
                                           m_Host, argStr, argUrl));
          } break;
          case advancedfx::interop::HostMessage::Message: {
            int targetId = ReadInt32();
            std::string argMessage;
            ReadStringUTF8(argMessage);
            m_Host->Message(browserId, targetId, argMessage);
          } break;
          default:
            throw "CHostPipeServerConnectionThread::HandleConnection: Unknown message.";
          }
        }
      }
      catch(const std::exception& e) {
        if(m_Browser) m_Host->RemoveBrowserConnection(browserId);
        throw e;
      }
    }

   private:
    SimpleHandler* m_Host;
    CefRefPtr<CefBrowser> m_Browser;
    advancedfx::interop::CPipeClient m_ClientConnection;
    std::mutex m_ClientConnectionMutex;
    int m_Width = 640;
    int m_Height = 480;

    };

  bool is_closing_;

  bool m_WaitConnectionQuit = false;
  std::thread m_WaitConnectionThread;
  void WaitConnectionThreadHandler(void);

  struct BrowserMapElem {
    CefRefPtr<CefBrowser> Browser;
    class CHostPipeServerConnectionThread* Connection = nullptr;

    BrowserMapElem(CefRefPtr<CefBrowser> browser) : Browser(browser) {}
  };

  std::mutex m_BrowserMutex;
  std::map<int, BrowserMapElem> m_Browsers;

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

// Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
