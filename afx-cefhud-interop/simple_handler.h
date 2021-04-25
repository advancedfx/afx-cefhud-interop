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


  // CefDisplayHandler methods:

  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;

   virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer,
                       int width,
                       int height) OVERRIDE {
/*
    if (PET_VIEW == type) {
      CefPostTask(TID_FILE_USER_BLOCKING, base::Bind(&SimpleHandler::DoPainted, this,
                                   browser->GetIdentifier(), m_ShareHandle));
    }*/
  }

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
  
  bool IsClosing() const { return is_closing_; }

 protected:
  class CHostPipeServerConnectionThread;

  advancedfx::interop::CPipeServerConnectionThread * OnNewConnection(HANDLE handle) override {
    return new CHostPipeServerConnectionThread(handle, this);
  }

 private:
  CefRefPtr<CefBrowser> AddBrowserConnection(
      int browserId,
      CHostPipeServerConnectionThread*
          connection) {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);
    auto it = m_Browsers.find(browserId);
    if (it == m_Browsers.end())
    {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << "AddBrowserConnection browser not found.";
      return nullptr;
    }

    it->second.Connection = connection;

    return it->second.Browser;
  }

  void RemoveBrowserConnection(int browserId) {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);
    auto it = m_Browsers.find(browserId);
    if (it == m_Browsers.end())
    {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << "RemoveBrowserConnection browser not found.";
      return;
    }

    it->second.Connection = nullptr;
  }

  void DoQuit();

  /**
   * @throws exception
   */
  void Message(int senderId, int targetId, const std::string& message) {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);
    auto it = m_Browsers.find(targetId);
    if (it == m_Browsers.end()) {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << "Message: browser not found.";
      return;
    }

    if (CHostPipeServerConnectionThread* connection = it->second.Connection)
    {
      connection->Message(senderId, message);
    }
    else {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << "Message: no connection.";
      return;
    }
  }

  bool OfferShareHandle(HANDLE share_handle)
  {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);    
    m_ShareHandle = share_handle;
    auto it = m_HandleToBrowserId.emplace(share_handle, 0);
    if(!it.second)
    {
      if(it.first->second)
      {
        return true;
      }
      return false;
    }
    return true;
  }

  
  void ReleaseShareHandle(HANDLE share_handle)
  {
    std::unique_lock<std::mutex> lock(m_BrowserMutex);    
    m_ShareHandle = share_handle;
    auto it = m_HandleToBrowserId.emplace(share_handle, 0);
    if(!it.second)
    {
      if(it.first->second)
      {
        auto it2 = m_Browsers.find(it.first->second);
        if(it2 != m_Browsers.end())
        it2->second.Connection->RelaseShareHandle(share_handle);
      }
    }
  }

  void DoDrawingResized(CefRefPtr<CefBrowser> browser) {
    browser->GetHost()->WasResized();
  }

  void DoRenderFrame(CefRefPtr<CefBrowser> browser) {
    browser->GetHost()->SendExternalBeginFrame();
  }

  void DoCreateDrawing(const std::string& argStr, const std::string& argUrl) {
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

    auto val = CefValue::Create();
    val->SetDictionary(extra_info);

    std::string prefix;
    std::string query;
    std::string suffix;

    size_t pos_hash = argUrl.find("#"); 
    if(std::string::npos != pos_hash)
    {
      prefix = argUrl.substr(0, pos_hash);
      suffix = argUrl.substr(pos_hash);
    }
    else {
      prefix = argUrl;
    }

    size_t pos_query = prefix.find("?");

    if(std::string::npos != pos_query)
    {
      query = prefix.substr(pos_query + 1);
      prefix = prefix.substr(0, pos_query);
    }

    if(0 < query.size()) query += "&";
    query += "afx="+CefURIEncode(CefWriteJSON(val, JSON_WRITER_DEFAULT), false).ToString();

    std::string url = prefix+"?"+query+suffix;

    CefBrowserHost::CreateBrowser(window_info, this, url, browser_settings, nullptr);
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

    auto val = CefValue::Create();
    val->SetDictionary(extra_info);

    std::string prefix;
    std::string query;
    std::string suffix;

    size_t pos_hash = argUrl.find("#"); 
    if(std::string::npos != pos_hash)
    {
      prefix = argUrl.substr(0, pos_hash);
      suffix = argUrl.substr(pos_hash);
    }
    else {
      prefix = argUrl;
    }

    size_t pos_query = prefix.find("?");

    if(std::string::npos != pos_query)
    {
      query = prefix.substr(pos_query + 1);
      prefix = prefix.substr(0, pos_query);
    }

    if(0 < query.size()) query += "&";
    query += "afx="+CefURIEncode(CefWriteJSON(val, JSON_WRITER_DEFAULT), false).ToString();

    std::string url = prefix+"?"+query+suffix;
    
    CefBrowserHost::CreateBrowser(window_info, this, url, browser_settings,
                                  nullptr);
  }

 protected:
  class CHostPipeServerConnectionThread
      : public advancedfx::interop::CPipeServerConnectionThread {
   public:
    CHostPipeServerConnectionThread(HANDLE handle, SimpleHandler* host)
        : advancedfx::interop::CPipeServerConnectionThread(handle),
          m_Host(host) {}

    int GetWidth() { return m_Width; }

    int GetHeight() { return m_Height; }

    /**
     * @throws exception
     */
    void OnPainted(void* share_handle) {
      std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
      m_ShareHandles.emplace(share_handle);
      m_ClientConnection.WriteInt32(
          (int)advancedfx::interop::ClientMessage::TextureHandle);
      m_ClientConnection.WriteHandle(share_handle);
      m_ClientConnection.Flush();
    }

    /**
     * @throws exception
     */
    void WaitedForGpu(bool bOk) {
      std::unique_lock<std::mutex> lock(m_ClientConnectionMutex);
      m_ClientConnection.WriteInt32(
          (int)advancedfx::interop::ClientMessage::WaitedForGpu);
      m_ClientConnection.WriteBoolean(bOk);
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
      m_ShareHandles.erase(share_handle);
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
      } else {
        std::unique_lock<std::mutex> lock(m_Host->m_BrowserMutex);
        m_Host->m_Connection0 = this;
      }

      try {
        m_Browser =
            browserId ? m_Host->AddBrowserConnection(
                            browserId, this)
                      : nullptr;

        while (!m_Quit) {
          advancedfx::interop::HostMessage message =
              (advancedfx::interop::HostMessage)ReadInt32();
          switch (message) {
            case advancedfx::interop::HostMessage::DrawingResized: {
              m_Width = ReadInt32();
              m_Height = ReadInt32();

              CefPostTask(TID_UI, base::Bind(&SimpleHandler::DoDrawingResized,
                                             m_Host, m_Browser));
            } break;
            case advancedfx::interop::HostMessage::RenderFrame: {
              CefPostTask(TID_UI, base::Bind(&SimpleHandler::DoRenderFrame,
                                             m_Host, m_Browser));
            } break;
            case advancedfx::interop::HostMessage::CreateDrawing: {
              std::string argUrl;
              ReadStringUTF8(argUrl);

              std::string argStr;
              ReadStringUTF8(argStr);

              CefPostTask(TID_PROCESS_LAUNCHER,
                          base::Bind(&SimpleHandler::DoCreateDrawing,
                                             m_Host, argStr, argUrl));
            } break;
            case advancedfx::interop::HostMessage::CreateEngine: {
              std::string argUrl;
              ReadStringUTF8(argUrl);

              std::string argStr;
              ReadStringUTF8(argStr);

              CefPostTask(TID_PROCESS_LAUNCHER,
                          base::Bind(&SimpleHandler::DoCreateEngine,
                                             m_Host, argStr, argUrl));
            } break;
            case advancedfx::interop::HostMessage::Message: {
              int targetId = ReadInt32();
              std::string argMessage;
              ReadStringUTF8(argMessage);
              m_Host->Message(browserId, targetId, argMessage);
            } break;
            case advancedfx::interop::HostMessage::GpuOfferShareHandle: {
              HANDLE shareHandle= ReadHandle();
              bool bWaitForGpu = m_Host->OfferShareHandle(shareHandle);
              WriteBoolean(bWaitForGpu);
              Flush();
              if(bWaitForGpu)
              {
                ReadBoolean();
              }
            } break;
            case advancedfx::interop::HostMessage::GpuRelaseShareHandle:
            {
              HANDLE shareHandle= ReadHandle();
              m_Host->ReleaseShareHandle(shareHandle);
            } break;
            default:
              throw "CHostPipeServerConnectionThread::HandleConnection: Unknown message.";
          }
        }
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
      }

      {
        std::unique_lock<std::mutex> lock(m_Host->m_BrowserMutex);

        for (auto it = m_ShareHandles.begin(); it != m_ShareHandles.end();
             ++it) {
          m_Host->m_HandleToBrowserId.erase(*it);
        }
      }

      if (m_Browser)
        m_Host->RemoveBrowserConnection(browserId);

      if (0 == browserId) {
        std::unique_lock<std::mutex> lock(m_Host->m_BrowserMutex);
        m_Host->m_Connection0 = nullptr;
      }

      try {
        m_ClientConnection.ClosePipe();
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
      }

      m_Browser = nullptr; // !

      ClosePipe();

      if (!m_ExternalAbort) {
        Detach();

        delete this;
      }
    }

   private:
    SimpleHandler* m_Host;
    bool m_Quit = false;
    bool m_ExternalAbort = false;
    CefRefPtr<CefBrowser> m_Browser;
    advancedfx::interop::CPipeClient m_ClientConnection;
    std::mutex m_ClientConnectionMutex;
    int m_Width = 640;
    int m_Height = 480;
    std::set<HANDLE> m_ShareHandles;
  };

  bool is_closing_;

  bool m_WaitConnectionQuit = false;
  std::thread m_WaitConnectionThread;
  void WaitConnectionThreadHandler(void);

  struct BrowserMapElem {
    CefRefPtr<CefBrowser> Browser;
    CHostPipeServerConnectionThread* Connection = nullptr;

    BrowserMapElem(CefRefPtr<CefBrowser> browser) : Browser(browser) {}
  };

  std::mutex m_BrowserMutex;
  std::map<int, BrowserMapElem> m_Browsers;
  HANDLE m_ShareHandle = INVALID_HANDLE_VALUE;
  std::map<HANDLE, int> m_HandleToBrowserId;
  CHostPipeServerConnectionThread* m_Connection0 = nullptr;

   // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  void DoPainted(int browserId, void* share_handle);

// Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
