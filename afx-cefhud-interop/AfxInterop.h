#pragma once

#include <include/cef_base.h>
#include <include/cef_v8.h>

#include <queue>
#include <list>
#include <string>
#include <functional>
#include <mutex>

#include <malloc.h>

#include <windows.h>
#include <d3d9types.h>

#define TEN_MINUTES_IN_MILLISECONDS (10*60*60*1000)

namespace advancedfx {
namespace interop {

class CPipeHandle {
 public:
  HANDLE GetHandle() { return m_Handle; }

 protected:
  HANDLE m_Handle = INVALID_HANDLE_VALUE;
};

struct CPipeException : public std::exception {
 public:
  DWORD LastError;

  CPipeException(DWORD lastError) : LastError(lastError) {}

  const char* what() const throw() { return "PipeIOException"; }
};

class CThreadedQueue {
  typedef std::function<void(void)> fp_t;

 public:
  CThreadedQueue();
  ~CThreadedQueue();

  void Abort();

  void Queue(const fp_t& op);
  void Queue(fp_t&& op);

  CThreadedQueue(const CThreadedQueue& rhs) = delete;
  CThreadedQueue& operator=(const CThreadedQueue& rhs) = delete;
  CThreadedQueue(CThreadedQueue&& rhs) = delete;
  CThreadedQueue& operator=(CThreadedQueue&& rhs) = delete;

 private:
  std::mutex m_Lock;
  std::thread m_Thread;
  std::queue<fp_t> m_Queue;
  std::condition_variable m_Cv;
  bool m_Quit = false;

  void QueueThreadHandler(void);
};

class CPipeReader : public virtual CPipeHandle {
 public:
  /**
   * @throws exception
   */
  void ReadBytes(LPVOID bytes, DWORD offset, DWORD length);

  /**
   * @throws exception
   */
  bool ReadBoolean();

  /**
   * @throws exception
   */
  BYTE ReadByte();

  /**
   * @throws exception
   */
  signed char ReadSByte();

  /**
   * @throws exception
   */
  INT16 ReadInt16();

  /**
   * @throws exception
   */
  UINT32 ReadUInt32();

  /**
   * @throws exception
   */
  UINT32 ReadCompressedUInt32();

  /**
   * @throws exception
   */
  INT32 ReadInt32();

  /**
   * @throws exception
   */
  INT32 ReadCompressedInt32();

  /**
   * @throws exception
   */
  void ReadStringUTF8(std::string& outValue);

  /**
   * @throws exception
   */
  HANDLE ReadHandle();

  /**
   * @throws exception
   */
  UINT64 ReadUInt64();

  /**
   * @throws exception
   */
  FLOAT ReadSingle();
};

class CPipeWriter : public virtual CPipeHandle {
 public:
  /**
   * @throws exception
   */
  void Flush();

  /**
   * @throws exception
   */
  void WriteBytes(const LPVOID bytes, DWORD offset, DWORD length);

  /**
   * @throws exception
   */
  void WriteBoolean(bool value);

  /**
   * @throws exception
   */
  void WriteByte(BYTE value);

  /**
   * @throws exception
   */
  void WriteSByte(signed char value);

  /**
   * @throws exception
   */
  void WriteUInt32(UINT32 value);

  /**
   * @throws exception
   */
  void WriteCompressedUInt32(UINT32 value);

  /**
   * @throws exception
   */
  void WriteInt32(INT32 value);

  /**
   * @throws exception
   */
  void WriteCompressedInt32(INT32 value);

  /**
   * @throws exception
   */
  void WriteUInt64(UINT64 value);

  /**
   * @throws exception
   */
  void WriteHandle(HANDLE value);

  /**
   * @throws exception
   */
  void WriteSingle(FLOAT value);

  /**
   * @throws exception
   */
  void WriteStringUTF8(const std::string& value);
};

class CPipeReaderWriter : public CPipeReader, public CPipeWriter {

};

class CPipeServerConnectionThread : public CPipeReaderWriter {

public:
  CPipeServerConnectionThread(HANDLE handle)
  {
      m_Handle = handle;
  }

  virtual ~CPipeServerConnectionThread() {}

  /**
   * @throws exception
   */
  virtual void HandleConnection() = 0;
};

struct CWinApiException : public std::exception {
 public:
  std::string What;
  DWORD LastError;

  CWinApiException(const char * what, DWORD lastError) : What(what), LastError(lastError) {}

  const char* what() const throw() { return What.c_str(); }
};

class CPipeServer {

public:
  /**
   * @throws exception
   */
 void WaitForConnection(const char* pipeName, int timeOut);

protected:
  virtual CPipeServerConnectionThread* OnNewConnection(HANDLE handle) = 0;

private:
  static DWORD WINAPI InstanceThread(LPVOID lpvParam);

};

class CPipeClient : public CPipeReaderWriter {

public:
  virtual ~CPipeClient() {
   try {
      ClosePipe();   
   }
   catch(...) {
   }
  }

  /**
    * @throws exception
    */
  void OpenPipe(const char* pipeName, int timeOut);

  /**
    * @throws exception
    */
  void ClosePipe();
};

enum class ClientMessage : int {
  Quit = 0,
  Message = 1,
  TextureHandle = 2,
  WaitedForGpu = 3,
  ReleaseTextureHandle = 4
};

enum class HostMessage : int {
  Quit = 0,
  DrawingResized = 1,
  RenderFrame = 2,
  CreateDrawing = 3,
  CreateEngine = 4,
  Message = 5,
  WaitForGpu = 6,
  GpuOfferShareHandle = 7,
  GpuRelaseShareHandle = 8
};


class CInterop : public CefBaseRefCounted {
  public:
    virtual void CloseInterop() = 0;

    // Must be called on TID_RENDERER.
    virtual bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) = 0;

  protected:
    CThreadedQueue m_InteropQueue;
};

CefRefPtr<CefV8Value> CreateInterop(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context,
                                    const CefString& argStr,
                                    DWORD handlerId,
                                    CefRefPtr<CInterop>* out = nullptr);

CefRefPtr<CefV8Value> CreateEngineInterop(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefV8Context> context,
                                          const CefString& argStr,
                                          DWORD handlerId,
                                          CefRefPtr<CInterop>* out = nullptr);

CefRefPtr<CefV8Value> CreateDrawingInterop(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context,
                                           const CefString& argStr,
                                           DWORD handlerId,
                                           CefRefPtr<CInterop>* out = nullptr);

}  // namespace interop
}  // namespace advancedfx
