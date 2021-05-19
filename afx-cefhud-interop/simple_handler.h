// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include <include/cef_client.h>
#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>

#include <include/cef_parser.h>

#include "AfxInterop.h"

#include <tchar.h>

#include <set>
#include <map>
#include <list>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefRenderHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRequestHandler,
                      private advancedfx::interop::CPipeServer {
 public:

  explicit SimpleHandler(class SimpleApp * simpleApp);
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

  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }

  // CefDisplayHandler methods:

  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;

   virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer,
                       int width,
                       int height) OVERRIDE {

     std::unique_lock<std::mutex> lock(m_BrowserMutex);
     m_LastBrowserId = browser->GetIdentifier();
  }

   virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
       PaintElementType type,
       const RectList& dirtyRects,
       void* shared_handle) OVERRIDE {

     std::unique_lock<std::mutex> lock(m_BrowserMutex);
     m_LastBrowserId = browser->GetIdentifier();
   }

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
  
  bool IsClosing() const { return is_closing_; }

  // CefRquestHandler methods:

  virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefRequest> request,
                              bool user_gesture,
                              bool is_redirect) OVERRIDE;


 protected:
  class CHostPipeServerConnectionThread;

  advancedfx::interop::CPipeServerConnectionThread * OnNewConnection(HANDLE handle) override {
    return new CHostPipeServerConnectionThread(handle, this);
  }

 private:

  /**
   * @throws exception
   */
  void Message(int senderId, int targetId, const std::string& message) {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);
    auto it = m_Browsers.find(targetId);
    if (it == m_Browsers.end()) {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << "Message: browser not found.";
      DebugBreak();
      return;
    }

    if (CHostPipeServerConnectionThread* connection = it->second.Connection)
    {
      connection->Message(senderId, message);
    }
    else {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << "Message: no connection.";
      DebugBreak();
      return;
    }
  }
   
    /**
     * @throws exception
     */
  void ReleaseShareHandle(HANDLE share_handle)
  {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);    

    auto it = m_HandleToBrowserId.emplace(share_handle, 0);
    if(!it.second)
    {
      if(0 != it.first->second)
      {
        auto it2 = m_Browsers.find(it.first->second);
        if(it2 != m_Browsers.end()) {
          it2->second.Connection->RelaseShareHandle(share_handle);
        }
      }
    }
    m_HandleToBrowserId.erase(it.first);
  }

  /**
   * @throws exception
   */
  bool OnAfterClear(HANDLE shareHandle) {
    bool bHandled = false;

    std::unique_lock<std::mutex> lock(m_BrowserMutex);

    if (0 != m_LastBrowserId) {
      m_HandleToBrowserId[shareHandle] = m_LastBrowserId;

      auto it2 = m_Browsers.find(m_LastBrowserId);
      if (it2 != m_Browsers.end() && it2->second.Connection) {
        it2->second.Connection->OnAfterClear(shareHandle);
        bHandled = true;
      }
    }

    return bHandled;
  }

  /**
   * @throws exception
   */
  bool OnAfterRender(HANDLE shareHandle) {
    bool bHandled = false;

    std::unique_lock<std::mutex> lock(m_BrowserMutex);

    if (0 != m_LastBrowserId) {
      m_HandleToBrowserId[shareHandle] = m_LastBrowserId;

      auto it2 = m_Browsers.find(m_LastBrowserId);
      if (it2 != m_Browsers.end() && it2->second.Connection) {
        it2->second.Connection->OnAfterRender(shareHandle);
        bHandled = true;
      }
    }

    return bHandled;
  }

  void DoDrawingResized(CefRefPtr<CefBrowser> browser, int width, int height) {


    browser->GetHost()->WasResized();
  }

  void DoRenderFrame(CefRefPtr<CefBrowser> browser) {
    browser->GetHost()->SendExternalBeginFrame();
  }
  void DoCreateDrawing(const std::string& argStr, const std::string& argUrl);

  void DoCreateEngine(const std::string& argStr, const std::string& argUrl);

 protected:
  struct BrowserMapElem {
    CefRefPtr<CefBrowser> Browser;
    int Width = 640;
    int Height = 480;
    class CHostPipeServerConnectionThread* Connection = nullptr;

    BrowserMapElem(CefRefPtr<CefBrowser> browser) : Browser(browser) {}
  };

  class CHostPipeServerConnectionThread
      : public advancedfx::interop::CPipeServerConnectionThread {
   public:
    CHostPipeServerConnectionThread(HANDLE handle, SimpleHandler* host)
        : advancedfx::interop::CPipeServerConnectionThread(handle),
          m_Host(host) {}

    int GetWidth() { return m_Owner->Width; }

    int GetHeight() { return m_Owner->Height; }

    /**
     * @throws exception
     */
    void OnAfterClear(void* share_handle) {
      std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
      m_ClientConnection.WriteInt32(
          (int)advancedfx::interop::ClientMessage::OnAfterClear);
      m_ClientConnection.WriteHandle(share_handle);
      m_ClientConnection.Flush();
    }

    /**
     * @throws exception
     */
    void OnAfterRender(void* share_handle) {
      std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
      m_ClientConnection.WriteInt32(
          (int)advancedfx::interop::ClientMessage::OnAfterRender);
      m_ClientConnection.WriteHandle(share_handle);
      m_ClientConnection.Flush();
    }

    /**
     * @throws exception
     */
    void Message(int senderId, const std::string& message) {
      std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
      m_ClientConnection.WriteInt32(
          (int)advancedfx::interop::ClientMessage::Message);
      m_ClientConnection.WriteInt32(senderId);
      m_ClientConnection.WriteStringUTF8(message);
      m_ClientConnection.Flush();
    }

   /**
     * @throws exception
     */
    void RelaseShareHandle(HANDLE share_handle)
    {
      std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
      m_ClientConnection.WriteInt32(
          (int)advancedfx::interop::ClientMessage::ReleaseTextureHandle);
      m_ClientConnection.WriteHandle(share_handle);
      m_ClientConnection.Flush();
    }

    void DeleteExternal(std::unique_lock<std::mutex> &lock)
    {
        m_ExternalAbort = true;
        lock.unlock();
 
        m_Quit = true;
        Cancel();
        Join();

        delete this;

        lock.lock();
    }

   protected:

    virtual void ConnectionThread() override {
      DWORD processId;
      int browserId;

      processId = ReadUInt32();
      browserId = ReadInt32();

      if(browserId)
      {
        std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_client_");
        strPipeName.append(std::to_string(processId));
        strPipeName.append("_");
        strPipeName.append(std::to_string(browserId));

        while(true)
        {
          bool bError = false;
          try {
           m_ClientConnection.OpenPipe(strPipeName.c_str(), INFINITE);
            Sleep(100);
          }
          catch(const std::exception&)
          {
            bError = true;
          }

          if(!bError) break;
        }
      }

      {
        std::unique_lock<std::mutex> lock(m_Host->m_BrowserMutex);

        auto emplaced = m_Host->m_Browsers.emplace(
            std::piecewise_construct, std::forward_as_tuple(browserId),
            std::forward_as_tuple(nullptr));
        if (emplaced.second && 0 != browserId) {

        }
        m_Owner = &(emplaced.first->second);
      }

      m_Owner->Connection = this;

      try {
        while (!m_Quit) {
          advancedfx::interop::HostMessage message =
              (advancedfx::interop::HostMessage)ReadInt32();
          switch (message) {
            case advancedfx::interop::HostMessage::RenderFrame: {
              int width = ReadInt32();
              int height = ReadInt32();

              if (m_Owner->Width != width || m_Owner->Height != height) {
                m_Owner->Width = width;
                m_Owner->Height = height;

                CefPostTask(TID_UI,
                            base::Bind(&SimpleHandler::DoDrawingResized, m_Host,
                                       m_Owner->Browser, width, height));
              }

            CefPostTask(TID_UI, base::Bind(&SimpleHandler::DoRenderFrame,
                                            m_Host, m_Owner->Browser));
            } break;
            case advancedfx::interop::HostMessage::CreateDrawing: {
              std::string argUrl;
              ReadStringUTF8(argUrl);

              std::string argStr;
              ReadStringUTF8(argStr);

              CefPostTask(TID_UI,
                          base::Bind(&SimpleHandler::DoCreateDrawing,
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
            case advancedfx::interop::HostMessage::GpuRelaseShareHandle:
            {
              HANDLE shareHandle= ReadHandle();
              m_Host->ReleaseShareHandle(shareHandle);
            } break;
            case advancedfx::interop::HostMessage::OnAfterClear: {
              HANDLE sharedHandle = ReadHandle();
              bool bHandled = m_Host->OnAfterClear(sharedHandle);

              if (bHandled) {
                std::unique_lock<std::mutex> lock(m_Host->m_DrawingMutex);
                m_Host->m_DrawingDone = false;
                m_Host->m_DrawingCv.wait(lock, [this] { return m_Host->m_DrawingDone; });
              } else {
                m_Host->m_DrawingResult = false;
              }

              WriteBoolean(m_Host->m_DrawingResult);

            } break;
            case advancedfx::interop::HostMessage::OnAfterRender: {
              HANDLE sharedHandle = ReadHandle();
              bool bHandled = m_Host->OnAfterRender(sharedHandle);

              if (bHandled) {
                std::unique_lock<std::mutex> lock(m_Host->m_DrawingMutex);
                m_Host->m_DrawingDone = false;
                m_Host->m_DrawingCv.wait(lock, [this] { return m_Host->m_DrawingDone; });
              } else {
                m_Host->m_DrawingResult = false;
              }

              WriteBoolean(m_Host->m_DrawingResult);
            } break;
            case advancedfx::interop::HostMessage::OnAfterClearDone: {
              m_Host->m_DrawingResult = ReadBoolean();
              {
                std::unique_lock<std::mutex> lock(m_Host->m_DrawingMutex);
                m_Host->m_DrawingDone = true;
                m_Host->m_DrawingCv.notify_one();
              }
            } break;
            case advancedfx::interop::HostMessage::OnAfterRenderDone: {
              m_Host->m_DrawingResult = ReadBoolean();
              {
                std::unique_lock<std::mutex> lock(m_Host->m_DrawingMutex);
                m_Host->m_DrawingDone = true;
                m_Host->m_DrawingCv.notify_one();
              }

            } break;
            default:
              throw "CHostPipeServerConnectionThread::HandleConnection: Unknown message.";
          }
        }
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
        DebugBreak();
      }

      m_Owner->Connection = nullptr;

      try {
        m_ClientConnection.ClosePipe();
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
        DebugBreak();
      }

      ClosePipe();

      if (!m_ExternalAbort) {
        Detach();

        delete this;
      }
    }

   private:
    bool m_Quit = false;
    bool m_ExternalAbort = false;
    SimpleHandler* m_Host;
    BrowserMapElem* m_Owner = nullptr;
    advancedfx::interop::CPipeClient m_ClientConnection;
    std::mutex m_ClientConnectionMutex;

  };

  class SimpleApp * simple_app_;
  bool is_closing_;

  bool m_WaitConnectionQuit = false;
  std::thread m_WaitConnectionThread;
  void WaitConnectionThreadHandler(void);

  std::mutex m_BrowserMutex;
  std::map<int, BrowserMapElem> m_Browsers;
  std::map<HANDLE, int> m_HandleToBrowserId;
  int m_LastBrowserId = 0;

    std::mutex m_DrawingMutex;
  std::condition_variable m_DrawingCv;
  bool m_DrawingDone = false;
  bool m_DrawingResult = false;
  bool m_Creating = true;

  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  void DoPainted(int browserId, void* share_handle);

// Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
