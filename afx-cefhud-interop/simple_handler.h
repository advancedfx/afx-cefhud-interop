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
#include <queue>

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

       DLOG(INFO) << "OnPaint: " << browser->GetIdentifier();

   }

   virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                   PaintElementType type,
                                   const RectList& dirtyRects,
                                   void* shared_handle) OVERRIDE {

     HANDLE clearHandle = INVALID_HANDLE_VALUE;
     int browserId = browser->GetIdentifier();

     {
       std::unique_lock<std::mutex> lock(m_BrowserMutex);

       if (0 != browserId) {
         auto it2 = m_Browsers.find(browserId);
         if (it2 != m_Browsers.end()) {
           clearHandle = it2->second.ClearHandle;
         }
       }
     }

     auto message = CefProcessMessage::Create("afx-paint");
     auto args = message->GetArgumentList();
     args->SetSize(4);
     args->SetInt(0, (int)(((unsigned __int64)shared_handle) & 0xFFFFFFFF));
     args->SetInt(1, (int)(((unsigned __int64)shared_handle) >> 32));
     args->SetInt(2, (int)(((unsigned __int64)clearHandle) & 0xFFFFFFFF));
     args->SetInt(3, (int)(((unsigned __int64)clearHandle) >> 32));

     //DLOG(INFO) << "OnAcceleratedPaint: " << browser->GetIdentifier();

     browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);
   }

     virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
                              CefScreenInfo& screen_info) OVERRIDE {
     m_NextBrowserId = browser->GetIdentifier();
     return false;
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
    //MessageBoxA(NULL, "A", "NEW", MB_OK);
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
      lock.unlock();
      connection->Message(senderId, message);
    }
    else {
      DLOG(INFO) << "No connection.";
      return;
    }
  }

  /**
   * @throws exception
   */
  bool OnAfterClear(INT32 browserId, HANDLE handle) {

    bool bResult = false;

    std::unique_lock<std::mutex> lock(m_BrowserMutex);

    if (0 != browserId) {
      auto it2 = m_Browsers.find(browserId);
      if (it2 != m_Browsers.end()) {
        if (CHostPipeServerConnectionThread* connection =
                it2->second.Connection) {
          lock.unlock();
          bResult = it2->second.Connection->OnAfterClear(handle);
          lock.lock();
        } else {
          DLOG(INFO) << "No connection.";
          return false;
        }
      }
    }

    return bResult;
  }

  /**
   * @throws exception
   */
  bool OnAfterRender(INT32 browserId) {
    bool bResult = false;

    std::unique_lock<std::mutex> lock(m_BrowserMutex);

    if (0 != browserId) {
      auto it2 = m_Browsers.find(browserId);
      if (it2 != m_Browsers.end()) {
        if (CHostPipeServerConnectionThread* connection =
                it2->second.Connection) {
          lock.unlock();
          bResult = it2->second.Connection->OnAfterRender();
          lock.lock();
        } else {
          DLOG(INFO) << "No connection.";
          return false;
        }
      }
    }

    return bResult;
  }

  void DoCreateDrawing(const std::string& argStr, const std::string& argUrl);

  void DoCreateEngine(const std::string& argStr, const std::string& argUrl);

 protected:
  struct BrowserMapElem {
    CefRefPtr<CefBrowser> Browser;
    int Width = 640;
    int Height = 480;
    class CHostPipeServerConnectionThread* Connection = nullptr;
    HANDLE ClearHandle = INVALID_HANDLE_VALUE;

    BrowserMapElem(CefRefPtr<CefBrowser> browser) : Browser(browser) {}
  };

  class CHostPipeServerConnectionThread
      : public advancedfx::interop::CPipeServerConnectionThread {
   public:
    CHostPipeServerConnectionThread(HANDLE handle, SimpleHandler* host)
        : advancedfx::interop::CPipeServerConnectionThread(handle),
          m_Host(host) {}

    void Lock() { m_SupressUpdates = true;
    }

    void Unlock(bool updateClearTexture) {
      m_UpdateClearTexture = updateClearTexture;
      m_SupressUpdates = false;
    }

    void SetCleared() {
        m_UpdateClearTexture = true; 
    }

    void SetSize(int width, int height) {
      m_Owner->Width = width;
      m_Owner->Height = height;
    }

    int GetWidth() { return m_Owner->Width; }

    int GetHeight() { return m_Owner->Height; }

    /**
     * @throws exception
     */
    bool OnAfterClear(HANDLE clearTextureHandle) {
        m_Owner->ClearHandle = clearTextureHandle;
      return m_UpdateClearTexture.exchange(false);
    }

    /**
     * @throws exception
     */
    bool OnAfterRender() {

      return m_SupressUpdates;
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

    std::atomic<bool> m_UpdateClearTexture = false;
    std::atomic<bool> m_SupressUpdates = false;
  };

  class CGpuPipeServer : public advancedfx::interop::CPipeServer {
   public:
    CGpuPipeServer(SimpleHandler* host) : m_Host(host) {
      m_GpuWaitConnectionThread =
          std::thread(&CGpuPipeServer::GpuWaitConnectionThreadHandler, this);
    }
      
    ~CGpuPipeServer() {
      m_GpuWaitConnectionQuit = true;
      CancelSynchronousIo(m_GpuWaitConnectionThread.native_handle());
      if (m_GpuWaitConnectionThread.joinable())
        m_GpuWaitConnectionThread.join();
    }
   protected:
    class CGpuPipeServerConnectionThread;

    advancedfx::interop::CPipeServerConnectionThread* OnNewConnection(
        HANDLE handle) override {
      //MessageBoxA(NULL, "B", "NEW", MB_OK);

      return new CGpuPipeServerConnectionThread(handle, this);
    }

  class CGpuPipeServerConnectionThread
         : public advancedfx::interop::CPipeServerConnectionThread {
      public:
    CGpuPipeServerConnectionThread(HANDLE handle, CGpuPipeServer* host)
           : advancedfx::interop::CPipeServerConnectionThread(handle),
             m_Host(host) {}

       void DeleteExternal(std::unique_lock<std::mutex>& lock) {
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

         processId = ReadUInt32();

         {
           std::unique_lock<std::mutex> lock(m_Host->m_ConnectionsMutex);

           auto emplaced = m_Host->m_Connections.emplace(
               std::piecewise_construct, std::forward_as_tuple(processId),
               std::forward_as_tuple(this));
         }

         try {
           while (!m_Quit) {
             advancedfx::interop::HostGpuMessage message =
                 (advancedfx::interop::HostGpuMessage)ReadInt32();
             switch (message) {
               case advancedfx::interop::HostGpuMessage::OnAfterClear: {
                 HANDLE handle = ReadHandle();
                 bool bResult = m_Host->m_Host->OnAfterClear(
                     m_Host->m_Host->m_NextBrowserId, handle);
                 WriteBoolean(bResult);
                 Flush();
               } break;
               case advancedfx::interop::HostGpuMessage::OnAfterRender: {
                 bool bResult = m_Host->m_Host->OnAfterRender(
                     m_Host->m_Host->m_NextBrowserId);
                 WriteBoolean(bResult);
                 Flush();
               } break;
               default:
                 throw "CGpuPipeServerConnectionThread::HandleConnection: Unknown message.";
             }
           }
         } catch (const std::exception& e) {
           DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                       << e.what();
           DebugBreak();
         }

         {
           std::unique_lock<std::mutex> lock(m_Host->m_ConnectionsMutex);

           m_Host->m_Connections.erase(processId);
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
       CGpuPipeServer* m_Host;
     };

     SimpleHandler* m_Host;
     
     std::map<INT32, CGpuPipeServerConnectionThread *> m_Connections;
     std::mutex m_ConnectionsMutex;

  bool m_GpuWaitConnectionQuit = false;
     std::thread m_GpuWaitConnectionThread;

     void GpuWaitConnectionThreadHandler(void) {
       std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_gpu_handler_");
       strPipeName.append(std::to_string(GetCurrentProcessId()));

       while (!m_GpuWaitConnectionQuit) {
         try {
           this->WaitForConnection(strPipeName.c_str(), 500);
         } catch (const std::exception& e) {
           DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                       << e.what();
           DebugBreak();
         }
       }
     }

  };

private:
  CGpuPipeServer g_GpuPipeServer;

  class SimpleApp * simple_app_;
  bool is_closing_;

  bool m_BrowserWaitConnectionQuit = false;
  std::thread m_BrowserWaitConnectionThread;
  void BrowserWaitConnectionThreadHandler(void);

  std::mutex m_BrowserMutex;
  std::map<int, BrowserMapElem> m_Browsers;
  int m_NextBrowserId = 0;

  bool m_Creating = true;

  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  void DoPainted(int browserId, void* share_handle);

// Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
