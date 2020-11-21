#include "AfxInterop.h"

#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>

#include <d3d11.h>
#include <tchar.h>

#include <atomic>
#include <condition_variable>
#include <map>
#include <unordered_map>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <set>
#include <vector>
#include <functional>

namespace advancedfx {
namespace interop {

const char * g_szAlreadyReleased = "Already released.";
const char* g_szOutOfFlow = "Out of flow.";
const char* g_szConnectionError = "Connection error.";
const char* g_szInvalidArguments = "Invalid arguments.";

enum class GameEventFieldType : int {
  Local = 0,
  CString = 1,
  Float = 2,
  Long = 3,
  Short = 4,
  Byte = 5,
  Bool = 6,
  Uint64 = 7
};

enum class EngineMessage : unsigned int {
  Invalid = 0,
  LevelInitPreEntity = 1,
  LevelShutDown = 2,
  BeforeFrameStart = 3,
  OnRenderView = 4,
  OnRenderViewEnd = 5,
  BeforeFrameRenderStart = 6,
  AfterFrameRenderStart = 7,
  OnViewOverride = 8,
  BeforeTranslucentShadow = 9,
  AfterTranslucentShadow = 10,
  BeforeTranslucent = 11,
  AfterTranslucent = 12,
  BeforeHud = 13,
  AfterHud = 14,
  GameEvent = 15
};

enum class DrawingMessage : unsigned int {
  Invalid = 0,
  PreapareDraw = 1,
  BeforeTranslucentShadow = 2,
  AfterTranslucentShadow = 3,
  BeforeTranslucent = 4,
  AfterTranslucent = 5,
  BeforeHud = 6,
  AfterHud = 7,
  OnRenderViewEnd = 8,

  DeviceLost = 9,
  DeviceRestored = 10,
  NOP = 11
};

enum class PrepareDrawReply : unsigned int {
  Skip = 1,
  Retry = 2,
  Continue = 3
};

enum class DrawingReply : unsigned int {
  Skip = 1,
  Retry = 2,
  Continue = 3,
  Finished = 4,

  D3d9CreateVertexDeclaration = 5,
  ReleaseD3d9VertexDeclaration = 6,

  D3d9CreateIndexBuffer = 7,
  ReleaseD3d9IndexBuffer = 8,
  UpdateD3d9IndexBuffer = 9,

  D3d9CreateVertexBuffer = 10,
  ReleaseD3d9VertexBuffer = 11,
  UpdateD3d9VertexBuffer = 12,

  D3d9CreateTexture = 13,
  ReleaseD3d9Texture = 14,
  UpdateD3d9Texture = 15,

  D3d9CreateVertexShader = 16,
  ReleaseD3d9VertexShader = 17,

  D3d9CreatePixelShader = 18,
  ReleaseD3d9PixelShader = 19,

  D3d9SetViewport = 20,
  D3d9SetRenderState = 21,
  D3d9SetSamplerState = 22,
  D3d9SetTexture = 23,
  D3d9SetTextureStageState = 24,
  D3d9SetTransform = 25,
  D3d9SetIndices = 26,
  D3d9SetStreamSource = 27,
  D3d9SetStreamSourceFreq = 28,
  D3d9SetVertexDeclaration = 29,
  D3d9SetVertexShader = 30,
  D3d9SetVertexShaderConstantF = 31,
  D3d9SetVertexShaderConstantI = 32,
  D3d9SetVertexShaderConstantB = 33,
  D3d9SetPixelShader = 34,
  D3d9SetPixelShaderConstantB = 35,
  D3d9SetPixelShaderConstantF = 36,
  D3d9SetPixelShaderConstantI = 37,
  D3d9DrawPrimitive = 38,
  D3d9DrawIndexedPrimitive = 39,

  WaitForGpu = 40,
  BeginCleanState = 41,
  EndCleanState = 42,

  D3d9UpdateTexture = 43
};

enum class AfxUserDataType : int {
  AfxHandle,
  AfxData,
  AfxD3d9VertexDeclaration,
  AfxD3d9IndexBuffer,
  AfxD3d9VertexBuffer,
  AfxD3d9Texture,
  AfxD3d9PixelShader,
  AfxD3d9VertexShader,
  AfxD3d9Viewport
};

struct Matrix4x4_s {
  float M00;
  float M01;
  float M02;
  float M03;
  float M10;
  float M11;
  float M12;
  float M13;
  float M20;
  float M21;
  float M22;
  float M23;
  float M30;
  float M31;
  float M32;
  float M33;
};

enum RenderPassType_e {
  RenderPassType_BeforeTranslucentShadow = 2,
  RenderPassType_AfterTranslucentShadow = 3,
  RenderPassType_BeforeTranslucent = 4,
  RenderPassType_AfterTranslucent = 5
};

struct View_s {
  int X;
  int Y;
  int Width;
  int Height;
  struct Matrix4x4_s ViewMatrix;
  struct Matrix4x4_s ProjectionMatrix;
};

struct RenderInfo_s {
  struct View_s View;
  int FrameCount;
  float AbsoluteFrameTime;
  float CurTime;
  float FrameTime;
};

struct HandleCalcResult_s {
  int IntHandle = -1;
};

struct Vector_s {
  float X = 0;
  float Y = 0;
  float Z = 0;
};

struct QAngle_s {
  float Pitch = 0;
  float Yaw = 0;
  float Roll = 0;
};

struct VecAngCalcResult_s {
  struct Vector_s Vector;
  struct QAngle_s QAngle;
};

struct CamCalcResult_s {
  struct Vector_s Vector;
  struct QAngle_s QAngle;
  float Fov = 90.0f;
};

struct FovCalcResult_s {
  float Fov = 90.0f;
};

struct BoolCalcResult_s {
  bool Result = false;
};

struct IntCalcResult_s {
  int Result = 0;
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

CThreadedQueue::CThreadedQueue() {
  m_Thread = std::thread(&CThreadedQueue::QueueThreadHandler, this);
}

CThreadedQueue::~CThreadedQueue() {
  Abort();
}

void CThreadedQueue::Abort() {
  if (!m_Quit) {
    std::unique_lock<std::mutex> lock(m_Lock);
    m_Quit = true;
    lock.unlock();
    m_Cv.notify_all();

    if (m_Thread.joinable()) {
      m_Thread.join();
    }  
  }
}


void CThreadedQueue::Queue(const fp_t& op) {
  std::unique_lock<std::mutex> lock(m_Lock);
  m_Queue.push(op);

  lock.unlock();
  m_Cv.notify_one();
}

void CThreadedQueue::Queue(fp_t&& op) {
  std::unique_lock<std::mutex> lock(m_Lock);
  m_Queue.push(std::move(op));

  lock.unlock();
  m_Cv.notify_one();
}

void CThreadedQueue::QueueThreadHandler(void) {
  std::unique_lock<std::mutex> lock(m_Lock);

  do {
    m_Cv.wait(lock, [this] { return (m_Queue.size() || m_Quit); });

    if (!m_Quit && m_Queue.size()) {
      auto op = std::move(m_Queue.front());
      m_Queue.pop();

      lock.unlock();

      op();

      lock.lock();
    }
  } while (!m_Quit);
}


class CNamedPipeServer {
 public:
  enum State { State_Error, State_Waiting, State_Connected };

 private:
  HANDLE m_PipeHandle;

  OVERLAPPED m_OverlappedRead = {};
  OVERLAPPED m_OverlappedWrite = {};

  State m_State = State_Error;

  const DWORD m_ReadBufferSize = 512;
  const DWORD m_WriteBufferSize = 512;

  const DWORD m_ReadTimeoutMs = 5000;
  const DWORD m_WriteTimeoutMs = 5000;

 public:
  CNamedPipeServer(const char* pipeName) {
    m_OverlappedRead.hEvent = CreateEventA(NULL, true, true, NULL);
    m_OverlappedWrite.hEvent = CreateEventA(NULL, true, true, NULL);

    std::string strPipeName("\\\\.\\pipe\\");
    strPipeName.append(pipeName);

    m_PipeHandle = CreateNamedPipeA(
        strPipeName.c_str(),
        PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_READMODE_BYTE | PIPE_TYPE_BYTE | PIPE_WAIT |
            PIPE_REJECT_REMOTE_CLIENTS,
        1, m_ReadBufferSize, m_WriteBufferSize, 5000, NULL);

    if (INVALID_HANDLE_VALUE != m_OverlappedRead.hEvent &&
        INVALID_HANDLE_VALUE != m_OverlappedWrite.hEvent &&
        INVALID_HANDLE_VALUE != m_PipeHandle &&
        FALSE == ConnectNamedPipe(m_PipeHandle, &m_OverlappedRead)) {
      switch (GetLastError()) {
        case ERROR_IO_PENDING:
          m_State = State_Waiting;
          break;
        case ERROR_PIPE_CONNECTED:
          m_State = State_Connected;
          SetEvent(m_OverlappedRead.hEvent);
          break;
      }
    }
  }

  ~CNamedPipeServer() {
    if (INVALID_HANDLE_VALUE != m_PipeHandle)
      CloseHandle(m_PipeHandle);
    if (INVALID_HANDLE_VALUE != m_OverlappedWrite.hEvent)
      CloseHandle(m_OverlappedWrite.hEvent);
    if (INVALID_HANDLE_VALUE != m_OverlappedRead.hEvent)
      CloseHandle(m_OverlappedRead.hEvent);
  }

  State Connect() {
    if (State_Waiting == m_State) {
      DWORD waitResult = WaitForSingleObject(m_OverlappedRead.hEvent, 0);

      if (WAIT_OBJECT_0 == waitResult) {
        DWORD cb;

        if (!GetOverlappedResult(m_PipeHandle, &m_OverlappedRead, &cb, FALSE))
          m_State = State_Error;
        else
          m_State = State_Connected;
      }
    }

    return m_State;
  }

  bool ReadBytes(LPVOID bytes, DWORD offset, DWORD length) {
    while (true) {
      DWORD bytesRead = 0;

      if (!ReadFile(m_PipeHandle, (LPVOID) & (((char*)bytes)[offset]), length,
                    NULL, &m_OverlappedRead)) {
        if (ERROR_IO_PENDING == GetLastError()) {
          bool completed = false;

          while (!completed) {
            DWORD result =
                WaitForSingleObject(m_OverlappedRead.hEvent, m_ReadTimeoutMs);
            switch (result) {
              case WAIT_OBJECT_0:
                completed = true;
                break;
              case WAIT_TIMEOUT:
                return false;
              default:
                return false;
            }
          }
        } else {
          return false;
        }
      }

      if (!GetOverlappedResult(m_PipeHandle, &m_OverlappedRead, &bytesRead,
                               FALSE)) {
        return false;
      }

      offset += bytesRead;
      length -= bytesRead;

      if (0 == length)
        break;
    }

    return true;
  }

  bool ReadBoolean(bool& outValue) {
    BYTE tmp;

    if (ReadBytes(&tmp, 0, sizeof(tmp))) {
      outValue = 0 != tmp;
      return true;
    }

    return false;
  }

  bool ReadByte(BYTE& outValue) {
    return ReadBytes(&outValue, 0, (DWORD)sizeof(outValue));
  }

  bool ReadSByte(signed char& outValue) {
    return ReadBytes(&outValue, 0, (DWORD)sizeof(outValue));
  }

  bool ReadInt16(INT16& outValue) {
    return ReadBytes(&outValue, 0, (DWORD)sizeof(outValue));
  }

  bool ReadUInt32(UINT32& outValue) {
    return ReadBytes(&outValue, 0, (DWORD)sizeof(outValue));
  }

  bool ReadCompressedUInt32(UINT32& outValue) {
    BYTE value;

    if (!ReadByte(value))
      return false;

    if (value < 255) {
      outValue = value;
      return true;
    }

    return ReadUInt32(outValue);
  }

  bool ReadInt32(INT32& outValue) {
    return ReadBytes(&outValue, 0, (DWORD)sizeof(outValue));
  }

  bool ReadCompressedInt32(INT32& outValue) {
    signed char value;

    if (!ReadSByte(value))
      return false;

    if (value < 127) {
      outValue = value;
      return value;
    }

    return ReadInt32(outValue);
  }

  bool ReadStringUTF8(std::string& outValue) {
    UINT32 length;

    if (!ReadCompressedUInt32(length))
      return false;

    outValue.resize(length);

    if (!ReadBytes(&outValue[0], 0, length))
      return false;

    return true;
  }

  bool ReadHandle(HANDLE& outValue) {
    DWORD value32;

    if (ReadBytes(&value32, 0, (DWORD)sizeof(value32))) {
      outValue = ULongToHandle(value32);
      return true;
    }

    return false;
  }

  bool ReadUInt64(uint64_t& outValue) {
    return ReadBytes(&outValue, 0, (DWORD)sizeof(outValue));
  }

  bool ReadSingle(float& outValue) {
    return ReadBytes(&outValue, 0, (DWORD)sizeof(outValue));
  }

  bool WriteBytes(const LPVOID bytes, DWORD offset, DWORD length) {
    while (true) {
      DWORD bytesWritten = 0;
      DWORD bytesToWrite = length;

      if (!WriteFile(m_PipeHandle, (LPVOID) & (((char*)bytes)[offset]),
                     bytesToWrite, NULL, &m_OverlappedWrite)) {
        if (ERROR_IO_PENDING == GetLastError()) {
          bool completed = false;

          while (!completed) {
            DWORD result =
                WaitForSingleObject(m_OverlappedWrite.hEvent, m_WriteTimeoutMs);
            switch (result) {
              case WAIT_OBJECT_0:
                completed = true;
                break;
              case WAIT_TIMEOUT:
                return false;
              default:
                return false;
            }
          }
        } else {
          return false;
        }
      }

      if (!GetOverlappedResult(m_PipeHandle, &m_OverlappedWrite, &bytesWritten,
                               FALSE)) {
        return false;
      }

      offset += bytesWritten;
      length -= bytesWritten;

      if (0 == length)
        break;
    }

    return true;
  }

  bool Flush() {
    if (!FlushFileBuffers(m_PipeHandle))
      return false;

    return true;
  }

  bool WriteBoolean(bool value) {
    BYTE tmp = value ? 1 : 0;

    return WriteBytes(&tmp, 0, sizeof(tmp));
  }

  bool WriteByte(BYTE value) { return WriteBytes(&value, 0, sizeof(value)); }

  bool WriteSByte(signed char value) {
    return WriteBytes(&value, 0, sizeof(value));
  }

  bool WriteUInt32(UINT32 value) {
    return WriteBytes(&value, 0, sizeof(value));
  }

  bool WriteCompressedUInt32(UINT32 value) {
    if (0 <= value && value <= 255 - 1) {
      return WriteByte((BYTE)value);
    } else {
      return WriteByte(255) && WriteUInt32(value);
    }
  }

  bool WriteInt32(INT32 value) { return WriteBytes(&value, 0, sizeof(value)); }

  bool WriteCompressedInt32(INT32 value) {
    if (-128 <= value && value <= 127 - 1) {
      WriteSByte((signed char)value);
    } else {
      return WriteSByte(127) && WriteInt32(value);
    }
  }

  bool WriteUInt64(UINT64 value) {
    return WriteBytes(&value, 0, sizeof(value));
  }

  bool WriteHandle(HANDLE value) {
    DWORD value32 = HandleToULong(value);

    return WriteBytes(&value32, 0, sizeof(value32));
  }

  bool WriteSingle(FLOAT value) { return WriteBytes(&value, 0, sizeof(value)); }

  bool WriteStringUTF8(const std::string &value) {
    UINT32 length = (UINT32)value.length();

    return WriteCompressedUInt32(length) &&
           WriteBytes((LPVOID)value.c_str(), 0, length);
  }
};

class CAfxCommand {
 public:
  size_t GetArgs() const { return m_Args.size(); }

  const char* GetArg(size_t index) const {
    if (index >= m_Args.size())
      return "";

    return m_Args[index].c_str();
  }

  void SetArgs(size_t numArgs) { m_Args.resize(numArgs); }

  std::string& GetArgString(size_t index) { return m_Args[index]; }

 private:
  std::vector<std::string> m_Args;
};

typedef std::function<bool(const CefString& name,
                        CefRefPtr<CefV8Value> object,
                        const CefV8ValueList& arguments,
                        CefRefPtr<CefV8Value>& retval,
                        CefString& exception)>
AfxExecute_t;

typedef std::unordered_map<std::string, AfxExecute_t> AfxExecuteMap_t;

typedef std::function<bool(const CefString& name,
                           const CefRefPtr<CefV8Value> object,
                           CefRefPtr<CefV8Value>& retval,
                           CefString& exception)>
    AfxGet_t;

typedef std::unordered_map<std::string, AfxGet_t> AfxGetMap_t;


typedef std::function<bool(const CefString& name,
                           const CefRefPtr<CefV8Value> object,
                           const CefRefPtr<CefV8Value> value,
                           CefString& exception)>
    AfxSet_t;

typedef std::unordered_map<std::string, AfxSet_t> AfxSetMap_t;

class CAfxHandler : public CefV8Handler {
 public:
  void AddExecute(const char* name, AfxExecute_t execute) {
    m_ExecuteMap.emplace(name, execute);
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) OVERRIDE {
    auto it = m_ExecuteMap.find(name);
    if (it != m_ExecuteMap.end())
      return it->second(name, object, arguments, retval, exception);

    return false;
  }

 private:
  AfxExecuteMap_t m_ExecuteMap;

  IMPLEMENT_REFCOUNTING(CAfxHandler);
};

class CAfxAccessor : public CefV8Accessor {
 public:
  void AddGet(const char* name, AfxGet_t get) { m_GetMap.emplace(name, get); }

  void AddSet(const char* name, AfxSet_t set) { m_SetMap.emplace(name, set); }

  virtual bool Get(const CefString& name,
                   const CefRefPtr<CefV8Value> object,
                   CefRefPtr<CefV8Value>& retval,
                   CefString& exception) OVERRIDE {
    auto it = m_GetMap.find(name);

    if (it != m_GetMap.end())
      return it->second(name, object, retval, exception);

    return false;
  }

  virtual bool Set(const CefString& name,
                   const CefRefPtr<CefV8Value> object,
                   const CefRefPtr<CefV8Value> value,
                   CefString& exception) OVERRIDE {
    auto it = m_SetMap.find(name);

    if (it != m_SetMap.end())
      return it->second(name, object, value, exception);

    return false;
  }

 private:
  AfxSetMap_t m_SetMap;
  AfxGetMap_t m_GetMap;

  IMPLEMENT_REFCOUNTING(CAfxAccessor);
};

class CAfxCallback : public CefBaseRefCounted {
 public:
  CAfxCallback(CefRefPtr<CefV8Context> context)
      : m_CallbackContext(context) {
  }

  CAfxCallback(CefRefPtr<CefV8Value> func, CefRefPtr<CefV8Context> context)
      : m_CallbackFunc(func), m_CallbackContext(context) {}


  CefRefPtr<CefV8Context> GetContext() {
      return m_CallbackContext;
  }

  void SetContext(CefRefPtr<CefV8Context> value) { m_CallbackContext = value;
  }

  CefRefPtr<CefV8Value> GetFunc() {
      return m_CallbackFunc;
  }

  void SetFunc(CefRefPtr<CefV8Value> value) {
      m_CallbackFunc = value;
  }

  bool IsValid() { return m_CallbackFunc != nullptr; }

  CefRefPtr<CefV8Value> ExecuteCallback(const CefV8ValueList& arguments) {
    return m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext, NULL,
                                                      arguments);
  }

 private:
  CefRefPtr<CefV8Value> m_CallbackFunc;
  CefRefPtr<CefV8Context> m_CallbackContext;

  IMPLEMENT_REFCOUNTING(CAfxCallback);
};

class CAfxObject : public CefBaseRefCounted {
 public:
  CAfxObject() {
      m_Handler = new CAfxHandler();
      m_Accessor = new CAfxAccessor();
      m_Object = CefV8Value::CreateObject(m_Accessor, nullptr);
  }

  CefRefPtr<CefV8Value> GetObj() { return m_Object; }

  void AddFunction(const char* name, AfxExecute_t execute) {
    m_Handler->AddExecute(name, execute);
    CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(name, m_Handler);
    m_Object->SetValue(name, func, V8_PROPERTY_ATTRIBUTE_NONE);
    m_Accessor->AddGet(
        name, [func](const CefString& name, const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& exception) { return func; });
  }

  void AddGetter(const char* name, AfxGet_t get) {
    m_Accessor->AddGet(name, get);
    m_Object->SetValue(name, V8_ACCESS_CONTROL_DEFAULT,
                  V8_PROPERTY_ATTRIBUTE_NONE);
  }

  void AddSetter(const char* name, AfxSet_t set) {
    m_Accessor->AddSet(name, set);
    m_Object->SetValue(name, V8_ACCESS_CONTROL_DEFAULT,
                       V8_PROPERTY_ATTRIBUTE_NONE);
  }

    CefRefPtr<CAfxCallback> AddCallback(const char* name,
                                      CefRefPtr<CefV8Context> context) {
    {
      CefRefPtr<CAfxCallback> callback = new CAfxCallback(context);

      m_Accessor->AddSet(
          name,
          [callback](const CefString& name, const CefRefPtr<CefV8Value> object,
                     const CefRefPtr<CefV8Value> value, CefString& exception) {
            if (value) {
              if (value->IsNull()) {
                callback->SetFunc(nullptr);
                return true;
              } else if (value->IsFunction()) {
                callback->SetFunc(value);
                return true;
              }
            }

            exception = "Callbacks can only be null or function.";
            return true;
          });

      m_Accessor->AddGet(
          name,
          [callback](const CefString& name, const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval, CefString& exception) {
            CefRefPtr<CefV8Value> func = callback->GetFunc();
            if (nullptr == func)
              retval = CefV8Value::CreateNull();
            else
              retval = func;

            return true;
          });

       m_Object->SetValue(name, V8_ACCESS_CONTROL_DEFAULT,
                         V8_PROPERTY_ATTRIBUTE_NONE);

      return callback;
    }
  }

    void SetUserData(CefRefPtr<CefBaseRefCounted> user_data) { m_Object->SetUserData(user_data);
    }

 private:
  CefRefPtr<CefV8Value> m_Object;
  CefRefPtr<CAfxHandler> m_Handler;
  CefRefPtr<CAfxAccessor> m_Accessor;

  IMPLEMENT_REFCOUNTING(CAfxObject);
};

class CCalcCallbacksGuts {
public:
  ~CCalcCallbacksGuts() {
    while (!m_Map.empty()) {
      while (!m_Map.begin()->second.empty()) {
        CAfxCallback* callback = (*m_Map.begin()->second.begin());
        callback->Release();
        m_Map.begin()->second.erase(m_Map.begin()->second.begin());
      }
      m_Map.erase(m_Map.begin());
    }
  }

  void Add(const char* name, CAfxCallback* callback) {
    callback->AddRef();
    m_Map[name].emplace(callback);
  }

  void Remove(const char* name, CAfxCallback* callback) {
    auto it1 = m_Map.find(name);
    if (it1 != m_Map.end()) {
      auto it2 = it1->second.find(callback);
      if (it2 != it1->second.end()) {
        it1->second.erase(it2);
        if (it1->second.empty())
          m_Map.erase(it1);
        callback->Release();
      }
    }
  }

 protected:
  typename typedef std::set<CAfxCallback*> Callbacks_t;
  typedef std::map<std::string, Callbacks_t> NameToCallbacks_t;
  NameToCallbacks_t m_Map;
};

template <typename R>
class CCalcCallbacks abstract : public CCalcCallbacksGuts {
 public:
  bool BatchUpdateRequest(CNamedPipeServer* pipeServer) {
    if (!pipeServer->WriteCompressedUInt32((UINT32)m_Map.size()))
      return false;
    for (typename std::map<std::string, std::set<CAfxCallback*>>::iterator it =
             m_Map.begin();
         it != m_Map.end(); ++it) {
      if (!pipeServer->WriteStringUTF8(it->first))
        return false;
    }

    return true;
  }

  bool BatchUpdateResult(CNamedPipeServer* pipeServer) {
    R result;

    for (typename std::map<std::string, std::set<CAfxCallback*>>::iterator it =
             m_Map.begin();
         it != m_Map.end(); ++it) {
      R* resultPtr = nullptr;
      bool hasResult;

      if (!pipeServer->ReadBoolean(hasResult))
        return false;

      if (hasResult) {
        if (!ReadResult(pipeServer, result))
          return false;
        resultPtr = &result;
      }

      for (typename std::set<CAfxCallback*>::iterator setIt =
               (*it).second.begin();
           setIt != (*it).second.end(); ++setIt) {
        CallResult(*setIt, resultPtr);
      }
    }

    return true;
  }

 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer, R& outResult) = 0;
  virtual void CallResult(CAfxCallback* callback, R* result) = 0;
};

class CHandleCalcCallbacks
    : public CCalcCallbacks<struct HandleCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          HandleCalcResult_s& outResult) override {
    if (!pipeServer->ReadInt32(outResult.IntHandle))
      return false;

    return true;
  }

  virtual void CallResult(CAfxCallback* callback,
                          struct HandleCalcResult_s* result) override {

    CefV8ValueList args;

    if (result) {
      CefRefPtr<CefV8Value> v8Obj = CefV8Value::CreateObject(nullptr, nullptr);

      v8Obj->SetValue("intHandle", CefV8Value::CreateInt(result->IntHandle),
                      V8_PROPERTY_ATTRIBUTE_NONE);

      args.push_back(v8Obj);
    } else {
      args.push_back(CefV8Value::CreateNull());
    }

    callback->ExecuteCallback(args);
  }
};

class CVecAngCalcCallbacks
    : public CCalcCallbacks<struct VecAngCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                         struct VecAngCalcResult_s& outResult) override {
    if (!pipeServer->ReadSingle(outResult.Vector.X))
      return false;
    if (!pipeServer->ReadSingle(outResult.Vector.Y))
      return false;
    if (!pipeServer->ReadSingle(outResult.Vector.Z))
      return false;

    if (!pipeServer->ReadSingle(outResult.QAngle.Pitch))
      return false;
    if (!pipeServer->ReadSingle(outResult.QAngle.Yaw))
      return false;
    if (!pipeServer->ReadSingle(outResult.QAngle.Roll))
      return false;

    return true;
  }

  virtual void CallResult(CAfxCallback* callback,
                          struct VecAngCalcResult_s* result) override {

    CefV8ValueList args;

    if (result) {
      CefRefPtr<CefV8Value> v8Obj = CefV8Value::CreateObject(nullptr, nullptr);

      CefRefPtr<CefV8Value> v8Vector =
          CefV8Value::CreateObject(nullptr, nullptr);
      v8Vector->SetValue("x", CefV8Value::CreateDouble(result->Vector.X),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("y", CefV8Value::CreateDouble(result->Vector.Y),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("z", CefV8Value::CreateDouble(result->Vector.Z),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Obj->SetValue("vector", v8Vector, V8_PROPERTY_ATTRIBUTE_NONE);

      CefRefPtr<CefV8Value> v8QAngle =
          CefV8Value::CreateObject(nullptr, nullptr);
      v8Vector->SetValue("pitch",
                         CefV8Value::CreateDouble(result->QAngle.Pitch),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("yaw", CefV8Value::CreateDouble(result->QAngle.Yaw),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("roll", CefV8Value::CreateDouble(result->QAngle.Roll),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Obj->SetValue("qAngle", v8QAngle, V8_PROPERTY_ATTRIBUTE_NONE);

      args.push_back(v8Obj);
    } else {
      args.push_back(CefV8Value::CreateNull());
    }

    callback->ExecuteCallback(args);
  }
};

class CCamCalcCallbacks : public CCalcCallbacks<struct CamCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          struct CamCalcResult_s& outResult) override {
    if (!pipeServer->ReadSingle(outResult.Vector.X))
      return false;
    if (!pipeServer->ReadSingle(outResult.Vector.Y))
      return false;
    if (!pipeServer->ReadSingle(outResult.Vector.Z))
      return false;

    if (!pipeServer->ReadSingle(outResult.QAngle.Pitch))
      return false;
    if (!pipeServer->ReadSingle(outResult.QAngle.Yaw))
      return false;
    if (!pipeServer->ReadSingle(outResult.QAngle.Roll))
      return false;

    if (!pipeServer->ReadSingle(outResult.Fov))
      return false;

    return true;
  }

  virtual void CallResult(CAfxCallback* callback,
                          struct CamCalcResult_s* result) override {
    CefV8ValueList args;

    if (result) {
      CefRefPtr<CefV8Value> v8Obj = CefV8Value::CreateObject(nullptr, nullptr);

      CefRefPtr<CefV8Value> v8Vector =
          CefV8Value::CreateObject(nullptr, nullptr);
      v8Vector->SetValue("x", CefV8Value::CreateDouble(result->Vector.X),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("y", CefV8Value::CreateDouble(result->Vector.Y),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("z", CefV8Value::CreateDouble(result->Vector.Z),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Obj->SetValue("vector", v8Vector, V8_PROPERTY_ATTRIBUTE_NONE);

      CefRefPtr<CefV8Value> v8QAngle =
          CefV8Value::CreateObject(nullptr, nullptr);
      v8Vector->SetValue("pitch",
                         CefV8Value::CreateDouble(result->QAngle.Pitch),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("yaw", CefV8Value::CreateDouble(result->QAngle.Yaw),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Vector->SetValue("roll", CefV8Value::CreateDouble(result->QAngle.Roll),
                         V8_PROPERTY_ATTRIBUTE_NONE);
      v8Obj->SetValue("qAngle", v8QAngle, V8_PROPERTY_ATTRIBUTE_NONE);

      v8Obj->SetValue("fov", CefV8Value::CreateDouble(result->Fov),
                      V8_PROPERTY_ATTRIBUTE_NONE);

      args.push_back(v8Obj);
    } else {
      args.push_back(CefV8Value::CreateNull());
    }

    callback->ExecuteCallback(args);
  }
};

class CFovCalcCallbacks
    : public CCalcCallbacks<struct FovCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          struct FovCalcResult_s& outResult) override {
    if (!pipeServer->ReadSingle(outResult.Fov))
      return false;

    return true;
  }

  virtual void CallResult(CAfxCallback* callback,
                          struct FovCalcResult_s* result) override {
    CefV8ValueList args;

    if (result) {
      CefRefPtr<CefV8Value> v8Obj = CefV8Value::CreateObject(nullptr, nullptr);

      v8Obj->SetValue("fov", CefV8Value::CreateDouble(result->Fov),
                      V8_PROPERTY_ATTRIBUTE_NONE);

      args.push_back(v8Obj);
    } else {
      args.push_back(CefV8Value::CreateNull());
    }

    callback->ExecuteCallback(args);
  }
};

class CBoolCalcCallbacks
    : public CCalcCallbacks<struct BoolCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          struct BoolCalcResult_s& outResult) override {
    bool result;
    if (!pipeServer->ReadBoolean(result))
      return false;

    outResult.Result = result;

    return true;
  }

  virtual void CallResult(CAfxCallback* callback,
                          struct BoolCalcResult_s* result) override {
    CefV8ValueList args;

    if (result) {
      CefRefPtr<CefV8Value> v8Obj = CefV8Value::CreateObject(nullptr, nullptr);

      v8Obj->SetValue("result", CefV8Value::CreateBool(result->Result),
                      V8_PROPERTY_ATTRIBUTE_NONE);

      args.push_back(v8Obj);
    } else {
      args.push_back(CefV8Value::CreateNull());
    }

    callback->ExecuteCallback(args);
  }
};

class CIntCalcCallbacks
    : public CCalcCallbacks<struct IntCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          struct IntCalcResult_s& outResult) override {
    INT32 result;
    if (!pipeServer->ReadInt32(result))
      return false;

    outResult.Result = result;

    return true;
  }

  virtual void CallResult(CAfxCallback* callback,
                          struct IntCalcResult_s* result) override {
    CefV8ValueList args;

    if (result) {
      CefRefPtr<CefV8Value> v8Obj = CefV8Value::CreateObject(nullptr, nullptr);

      v8Obj->SetValue("result", CefV8Value::CreateInt(result->Result),
                      V8_PROPERTY_ATTRIBUTE_NONE);

      args.push_back(v8Obj);
    } else {
      args.push_back(CefV8Value::CreateNull());
    }

    callback->ExecuteCallback(args);
  }
};

class CAfxInterop {
 public:
  const INT32 Version = 7;

  CAfxInterop(const char* pipeName) : m_PipeName(pipeName) {}

  virtual bool Connection(void) {
    if (nullptr == m_PipeServer) {
      std::string pipeName(m_PipeName);

      m_PipeServer = new CNamedPipeServer(pipeName.c_str());
      m_Connecting = true;
    }

    CNamedPipeServer::State state = m_PipeServer->Connect();

    switch (state) {
      case CNamedPipeServer::State_Connected:
        if (m_Connecting) {
          m_Connecting = false;
          if (!OnNewConnection())
            goto locked_error;
        }
        if (!OnConnection())
          goto locked_error;
        break;
      case CNamedPipeServer::State_Error:
        goto locked_error;
      default:
        return true;
    }
    return true;

  locked_error:
    Close();
    return false;
  }

  virtual bool Connected() {
    return nullptr != m_PipeServer && !m_Connecting;
  }

  virtual void Close() {
    OnClosing();

    if (nullptr != m_PipeServer) {
      delete m_PipeServer;
      m_PipeServer = nullptr;
    }
  }

  virtual const char* GetPipeName() const { return m_PipeName.c_str(); }

  virtual void SetPipeName(const char* value) { m_PipeName = value; }

 protected:
  std::string m_PipeName;
  CNamedPipeServer* m_PipeServer = nullptr;

  virtual ~CAfxInterop() { Close(); }

  virtual bool OnNewConnection() { return true; }

  virtual bool OnConnection() { return true; }

  virtual void OnClosing() {}

 private:
  bool m_Connecting = false;
};

class CAfxUserData : public CefBaseRefCounted {
 public:
  CAfxUserData(AfxUserDataType userDataType, void* userData)
      : m_UserDataType(userDataType) , m_UserData(userData) {}

  AfxUserDataType GetUserDataType() const { return m_UserDataType; }
  void* GetUserData() const { return m_UserData; }

 protected:
  ~CAfxUserData() {}

 private:
  AfxUserDataType m_UserDataType;
  void* m_UserData;

  IMPLEMENT_REFCOUNTING(CAfxUserData);
};

template <AfxUserDataType type, class T>
CefRefPtr<T> GetAfxUserData(CefRefPtr<CefV8Value> value) {
  if(nullptr == value || !value->IsObject()) return nullptr;
  CefRefPtr<CefBaseRefCounted> user_data = value->GetUserData();
  if(nullptr == user_data) return nullptr;
  CAfxUserData* afxUserData = static_cast<CAfxUserData *>(user_data.get());
  if(type != afxUserData->GetUserDataType()) return nullptr;
  return (T*)afxUserData->GetUserData();
}

class CDrawingInteropImpl : public CInterop, public CAfxInterop {
  IMPLEMENT_REFCOUNTING(CDrawingInteropImpl);

 public:
  static CefRefPtr<CefV8Value> Create(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context,
                                      const CefString& argStr,
                                      CefRefPtr<CInterop>* out = nullptr) {
    CefRefPtr<CDrawingInteropImpl> self = new CDrawingInteropImpl();

    CefRefPtr<CAfxObject> afxObject = new CAfxObject();

    //

    afxObject->AddGetter(
        "id",
        [browser](const CefString& name, const CefRefPtr<CefV8Value> object,
                  CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateInt(browser->GetIdentifier());
          return true;
        });

    afxObject->AddFunction(
        "init", [context, obj = afxObject->GetObj(), argStr](const CefString& name,
                                             CefRefPtr<CefV8Value> object,
                                             const CefV8ValueList& arguments,
                                             CefRefPtr<CefV8Value>& retval,
                                             CefString& exceptionoverride) {
          if (1 == arguments.size()) {
            auto argInitFn = arguments[0];

            if (argInitFn->IsFunction()) {
              CefV8ValueList args;
              args.push_back(obj);
              args.push_back(CefV8Value::CreateString(argStr));

              argInitFn->ExecuteFunctionWithContext(context, NULL, args);

              return true;
            }
          }

          return true;
        });

    afxObject->AddFunction(
        "sendMessage",
        [frame](const CefString& name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exceptionoverride) {
          if (arguments.size() == 2) {
            auto argId = arguments[0];
            auto argStr = arguments[1];

            if (argId->IsInt() && argStr->IsString()) {
              auto message = CefProcessMessage::Create("afx-send-message");
              auto args = message->GetArgumentList();
              args->SetSize(2);
              args->SetInt(0, argId->GetIntValue());
              args->SetString(1, argStr->GetStringValue());

              frame->SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    self->m_OnMessage = afxObject->AddCallback("onMessage", context);

    self->m_OnError = afxObject->AddCallback("onError", context);

    afxObject->AddGetter(
        "pipeName",
        [self](const CefString& name, const CefRefPtr<CefV8Value> object,
               CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateString(self->GetPipeName());

          return true;
        });
    afxObject->AddSetter(
        "pipeName",
        [self](const CefString& name, const CefRefPtr<CefV8Value> object,
               const CefRefPtr<CefV8Value> value, CefString& exception) {
          if (value && value->IsString()) {
            std::string pipeName(value->GetStringValue().ToString());

            self->SetPipeName(pipeName.c_str());
            return true;
          }

          exception = g_szInvalidArguments;
          return true;
        });

    self->m_OnDeviceLost = afxObject->AddCallback("onDeviceLost", context);

    self->m_OnDeviceReset = afxObject->AddCallback("onDeviceReset", context);

    afxObject->AddFunction("connect", [self](const CefString& name,
                                             CefRefPtr<CefV8Value> object,
                                             const CefV8ValueList& arguments,
                                             CefRefPtr<CefV8Value>& retval,
                                             CefString& exceptionoverride) {
      retval = CefV8Value::CreateBool(self->Connection() && self->Connected());
      return true;
    });
    afxObject->AddFunction(
        "close",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          self->Close();
          return true;
        });
    afxObject->AddFunction("pumpBegin", [self](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exceptionoverride) {
      if (2 <= arguments.size() &&
          arguments[0]->IsInt() && arguments[1]->IsUInt()) {
        if (!self->Connected()) {
          exceptionoverride = "Not connected.";
          return true;
        }

        if (!self->DoPumpBegin(arguments[0]->GetIntValue(),
                                      arguments[1]->GetUIntValue())) {
          self->Close();
          exceptionoverride = g_szConnectionError;
          return true;
        }

        retval = CefV8Value::CreateBool(self->m_InFlow);
        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });

    afxObject->AddFunction(
        "pumpEnd",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (!self->m_PipeServer->WriteUInt32((UINT32)DrawingReply::Finished))
            goto error;

          if (!self->m_PipeServer->Flush())
            goto error;

          return true;

        error:
          exceptionoverride = g_szConnectionError;
          return true;
        });
    afxObject->AddFunction(
        "setSize",
        [self, frame](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          if (2 <= arguments.size() &&
              arguments[0]->IsInt() && arguments[1]->IsInt()) {
            int width = arguments[0]->GetIntValue();
            int height = arguments[1]->GetIntValue();

            if (width != self->m_Width || height != self->m_Height) {
              self->m_Width = width;
              self->m_Height = height;

              auto message = CefProcessMessage::Create("afx-drawing-resized");
              auto args = message->GetArgumentList();
              args->SetSize(2);
              args->SetInt(0, width);
              args->SetInt(1, height);
              frame->SendProcessMessage(PID_BROWSER, message);
              return true;
            }
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "beginFrame",
        [frame](const CefString& name, CefRefPtr<CefV8Value> object,
                const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                CefString& exceptionoverride) {
          auto message = CefProcessMessage::Create("afx-drawing-begin-frame");
          frame->SendProcessMessage(PID_BROWSER, message);
          return true;
        });
    afxObject->AddFunction(
        "d3d9CreateVertexDeclaration",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (1 <= arguments.size()) {
            auto data = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(
                arguments[0]);

            if (nullptr != data) {
              CefRefPtr<CAfxD3d9VertexDeclaration> val;
              retval = CAfxD3d9VertexDeclaration::Create(self, &val);

              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreateVertexDeclaration))
                goto error;
              if (!self->m_PipeServer->WriteUInt64((UINT64)val->GetIndex()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)data->GetSize()))
                goto error;
              if (!self->m_PipeServer->WriteBytes(data->GetData(), 0,
                                                  (DWORD)data->GetSize()))
                goto error;

              return true;

            error:
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }
          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9CreateIndexBuffer",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (5 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt()) {
            auto drawingHandle =
                GetAfxUserData<AfxUserDataType::AfxData, CAfxHandle>(
                    arguments[4]);

            CefRefPtr<CAfxD3d9IndexBuffer> val;
            retval = CAfxD3d9IndexBuffer::Create(self, &val);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9CreateIndexBuffer))
              goto error;
            if (!self->m_PipeServer->WriteUInt32((UINT64)val->GetIndex()))
              goto error;
            // length:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[0]->GetUIntValue()))
              goto error;
            // usage:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[1]->GetUIntValue()))
              goto error;
            // format:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[2]->GetUIntValue()))
              goto error;
            // pool:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[3]->GetUIntValue()))
              goto error;
            if (nullptr == drawingHandle) {
              if (!self->m_PipeServer->WriteBoolean(false))
                goto error;
            } else {
              if (!self->m_PipeServer->WriteBoolean(true))
                goto error;
              if (!self->m_PipeServer->WriteHandle(drawingHandle->GetHandle()))
                goto error;
            }

            return true;

          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9CreateVertexBuffer",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (5 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt()) {
            auto drawingHandle =
                GetAfxUserData<AfxUserDataType::AfxHandle, CAfxHandle>(
                    arguments[4]);

            CefRefPtr<CAfxD3d9VertexBuffer> val;
            retval = CAfxD3d9VertexBuffer::Create(self, &val);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9CreateVertexBuffer))
              goto error;
            if (!self->m_PipeServer->WriteUInt64((UINT64)val->GetIndex()))
              goto error;
            // length
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[0]->GetUIntValue()))
              goto error;
            // usage
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[1]->GetUIntValue()))
              goto error;
            // fvf
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[2]->GetUIntValue()))
              goto error;
            // pool
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[3]->GetUIntValue()))
              goto error;
            if (nullptr == drawingHandle) {
              if (!self->m_PipeServer->WriteBoolean(false))
                goto error;
            } else {
              if (!self->m_PipeServer->WriteBoolean(true))
                goto error;
              if (!self->m_PipeServer->WriteHandle(drawingHandle->GetHandle()))
                goto error;
            }

            return true;

          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9CreateTexture",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (7 <= arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt() &&
              arguments[4]->IsUInt() && arguments[5]->IsUInt()) {
            CefRefPtr<CAfxHandle> drawingHandle =
                GetAfxUserData<AfxUserDataType::AfxHandle, CAfxHandle>(
                    arguments[6]);

            CefRefPtr<CAfxD3d9Texture> val;
            retval = CAfxD3d9Texture::Create(self, &val);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9CreateTexture))
              goto error;
            if (!self->m_PipeServer->WriteUInt64((UINT64)val->GetIndex()))
              goto error;
            // width:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[0]->GetUIntValue()))
              goto error;
            // height:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[1]->GetUIntValue()))
              goto error;
            // levels:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[2]->GetUIntValue()))
              goto error;
            // usage:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[3]->GetUIntValue()))
              goto error;
            // format:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[4]->GetUIntValue()))
              goto error;
            // pool:
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[5]->GetUIntValue()))
              goto error;
            if (nullptr == drawingHandle) {
              if (!self->m_PipeServer->WriteBoolean(false))
                goto error;
            } else {
              if (!self->m_PipeServer->WriteBoolean(true))
                goto error;
              if (!self->m_PipeServer->WriteHandle(drawingHandle->GetHandle()))
                goto error;
            }

            return true;

          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9CreateVertexShader",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (1 <= arguments.size()) {
            auto data = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(
                arguments[0]);

            if (nullptr != data) {
              CefRefPtr<CAfxD3d9VertexShader> val;
              retval = CAfxD3d9VertexShader::Create(self, &val);

              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreateVertexShader))
                goto error;
              if (!self->m_PipeServer->WriteUInt64((UINT64)val->GetIndex()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)data->GetSize()))
                goto error;
              if (!self->m_PipeServer->WriteBytes(data->GetData(), 0,
                                                  (DWORD)data->GetSize()))
                goto error;

              return true;

            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9CreatePixelShader",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (1 == arguments.size()) {
            auto data = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(
                arguments[0]);

            if (data) {
              CefRefPtr<CAfxD3d9PixelShader> val;
              retval = CAfxD3d9PixelShader::Create(self, &val);

              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreatePixelShader))
                goto error;
              if (!self->m_PipeServer->WriteUInt64((UINT64)val->GetIndex()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)data->GetSize()))
                goto error;
              if (!self->m_PipeServer->WriteBytes(data->GetData(), 0,
                                                  (DWORD)data->GetSize()))
                goto error;

              return true;
            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9UpdateTexture",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 <= arguments.size()) {
            CefRefPtr<CAfxD3d9Texture> val =
                GetAfxUserData<AfxUserDataType::AfxD3d9Texture,
                               CAfxD3d9Texture>(arguments[0]);
            CefRefPtr<CAfxD3d9Texture> val2 =
                GetAfxUserData<AfxUserDataType::AfxD3d9Texture,
                               CAfxD3d9Texture>(arguments[1]);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9UpdateTexture))
              goto error;
            if (!self->m_PipeServer->WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto error;
            if (!self->m_PipeServer->WriteUInt64(val2 ? (UINT64)val2->GetIndex()
                                                      : 0))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetViewport",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (0 == arguments.size()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetViewport))
              goto error_1;
            if (!self->m_PipeServer->WriteBoolean(false))
              goto error_1;

            return true;
          error_1:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          } else if (1 == arguments.size() &&
                     arguments[0]->IsObject()) {
            auto x = arguments[0]->GetValue("x");
            auto y = arguments[0]->GetValue("y");
            auto width = arguments[0]->GetValue("width");
            auto height = arguments[0]->GetValue("height");
            auto minZ = arguments[0]->GetValue("minZ");
            auto maxZ = arguments[0]->GetValue("maxZ");

            if (x && y && width && height && minZ && maxZ && x->IsUInt() &&
                y->IsUInt() && width->IsUInt() && height->IsUInt() &&
                minZ->IsDouble() && maxZ->IsDouble()) {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetViewport))
                goto error_2;
              if (!self->m_PipeServer->WriteBoolean(true))
                goto error_2;
              if (!self->m_PipeServer->WriteUInt32(x->GetUIntValue()))
                goto error_2;
              if (!self->m_PipeServer->WriteUInt32(y->GetUIntValue()))
                goto error_2;
              if (!self->m_PipeServer->WriteUInt32(width->GetUIntValue()))
                goto error_2;
              if (!self->m_PipeServer->WriteUInt32(height->GetUIntValue()))
                goto error_2;
              if (!self->m_PipeServer->WriteSingle(minZ->GetUIntValue()))
                goto error_2;
              if (!self->m_PipeServer->WriteSingle(maxZ->GetUIntValue()))
                goto error_2;

              return true;

            error_2:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetRenderState",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetRenderState))
              goto error;
            // state
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[0]->GetUIntValue()))
              goto error;
            // value
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[1]->GetUIntValue()))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetSamplerState",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (3 == arguments.size() && arguments[0]->IsUInt() &&
              arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetSamplerState))
              goto error;
            // sampler
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[0]->GetUIntValue()))
              goto error;
            // type
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[1]->GetUIntValue()))
              goto error;
            // value
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[2]->GetUIntValue()))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetTexture",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt()) {
            CefRefPtr<CAfxD3d9Texture> val =
                GetAfxUserData<AfxUserDataType::AfxD3d9Texture,
                               CAfxD3d9Texture>(arguments[0]);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetTexture))
              goto error;

            // sampler
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)arguments[0]->GetUIntValue()))
              goto error;
            // textureIndex
            if (!self->m_PipeServer->WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetTextureStageState",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (3 == arguments.size() && arguments[0]->IsUInt() &&
              arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetTextureStageState))
              goto error;
            // stage
            if (!self->m_PipeServer->WriteUInt32(arguments[0]->GetUIntValue()))
              goto error;
            // type
            if (!self->m_PipeServer->WriteUInt32(arguments[1]->GetUIntValue()))
              goto error;
            // value
            if (!self->m_PipeServer->WriteUInt32(arguments[2]->GetUIntValue()))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("d3d9SetTransform",
        [self](const CefString& name,
   CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt()) {
            if (arguments[1]->IsArray() &&
                arguments[1]->GetArrayLength() == 16) {
              bool bOk = true;
              for (int i = 0; i < 16; ++i) {
                auto arrVal = arguments[1]->GetValue(i);

                if (arrVal && arrVal->IsDouble()) {
                } else {
                  bOk = false;
                  break;
                }
              }
              if (bOk) {
                if (!self->m_PipeServer->WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetTransform))
                  goto error_1;
                // state
                if (!self->m_PipeServer->WriteUInt32(
                        (UINT32)arguments[0]->GetUIntValue()))
                  goto error_1;
                if (!self->m_PipeServer->WriteBoolean(true))
                  goto error_1;

                for (int i = 0; i < 16; ++i) {
                  auto arrVal = arguments[1]->GetValue(i);

                  if (!self->m_PipeServer->WriteSingle(
                          arrVal->GetDoubleValue()))
                    goto error_1;
                }

                return true;
              error_1:
                self->Close();
                exceptionoverride = g_szConnectionError;
                return true;
              }
            } else {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetTransform))
                goto error_2;

              // state:
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)arguments[0]->GetUIntValue()))
                goto error_2;
              if (!self->m_PipeServer->WriteBoolean(false))
                goto error_2;

              return true;
            error_2:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetIndices",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (1 == arguments.size()) {
            CefRefPtr<CAfxD3d9IndexBuffer> val =
                GetAfxUserData<AfxUserDataType::AfxD3d9IndexBuffer,
                               CAfxD3d9IndexBuffer>(arguments[0]);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetIndices))
              goto error;
            if (!self->m_PipeServer->WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetStreamSource",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (4 == arguments.size() && arguments[0]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt()) {
            CefRefPtr<CAfxD3d9VertexBuffer> val =
                GetAfxUserData<AfxUserDataType::AfxD3d9VertexBuffer,
                               CAfxD3d9VertexBuffer>(arguments[1]);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetStreamSource))
              goto error;
            // streamNumber
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT64)arguments[0]->GetUIntValue()))
              goto error;
            if (!self->m_PipeServer->WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto error;
            // offsetInBytes
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT64)arguments[2]->GetUIntValue()))
              goto error;
            // stride
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT64)arguments[3]->GetUIntValue()))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetStreamSourceFreq",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetStreamSourceFreq))
              goto error;
            // streamNumber
            if (!self->m_PipeServer->WriteUInt32(arguments[0]->GetUIntValue()))
              goto error;
            // setting
            if (!self->m_PipeServer->WriteUInt32(arguments[1]->GetUIntValue()))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetVertexDeclaration",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (1 == arguments.size()) {
            CefRefPtr<CAfxD3d9VertexDeclaration> val =
                GetAfxUserData<AfxUserDataType::AfxD3d9VertexDeclaration,
                               CAfxD3d9VertexDeclaration>(arguments[0]);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetVertexDeclaration))
              goto error;
            if (!self->m_PipeServer->WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto error;

            return true;

          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetVertexShader",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (1 == arguments.size()) {
            CefRefPtr<CAfxD3d9VertexShader> val =
                GetAfxUserData<AfxUserDataType::AfxD3d9VertexShader,
                               CAfxD3d9VertexShader>(arguments[0]);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetVertexShader))
              goto error;
            if (!self->m_PipeServer->WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetVertexShaderConstantB",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsArray()) {
            size_t arrLen = arguments[1]->GetArrayLength();

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[1]->GetValue(i);

              if (arrVal && arrVal->IsBool()) {
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShaderConstantB))
                goto error;
              // startRegister
              if (!self->m_PipeServer->WriteUInt32(
                      arguments[0]->GetUIntValue()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)arrLen))
                goto error;
              for (int i = 0; i < arrLen; ++i) {
                auto arrVal = arguments[1]->GetValue(i);
                if (!self->m_PipeServer->WriteBoolean(arrVal->GetBoolValue()))
                  goto error;
              }

              return true;
            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("d3d9SetVertexShaderConstantF",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsArray()) {
            size_t arrLen = arguments[1]->GetArrayLength();

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[1]->GetValue(i);

              if (arrVal && arrVal->IsDouble()) {
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShaderConstantF))
                goto error;
              // startRegister
              if (!self->m_PipeServer->WriteUInt32(
                      arguments[0]->GetUIntValue()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)arrLen))
                goto error;
              for (int i = 0; i < arrLen; ++i) {
                auto arrVal = arguments[1]->GetValue(i);
                if (!self->m_PipeServer->WriteSingle(arrVal->GetDoubleValue()))
                  goto error;
              }

              return true;
            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("d3d9SetVertexShaderConstantI",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsArray()) {
            size_t arrLen = arguments[1]->GetArrayLength();

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[1]->GetValue(i);

              if (arrVal && arrVal->IsInt()) {
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShaderConstantI))
                goto error;
              // startRegister
              if (!self->m_PipeServer->WriteUInt32(
                      arguments[0]->GetUIntValue()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)arrLen))
                goto error;
              for (int i = 0; i < arrLen; ++i) {
                auto arrVal = arguments[1]->GetValue(i);
                if (!self->m_PipeServer->WriteInt32(arrVal->GetIntValue()))
                  goto error;
              }

              return true;
            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("d3d9SetPixelShader",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (1 == arguments.size()) {
            CefRefPtr<CAfxD3d9PixelShader> val =
                GetAfxUserData<AfxUserDataType::AfxD3d9PixelShader,
                               CAfxD3d9PixelShader>(arguments[0]);

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetPixelShader))
              goto error;
            if (!self->m_PipeServer->WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetPixelShaderConstantB",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsArray()) {
            size_t arrLen = arguments[1]->GetArrayLength();

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[1]->GetValue(i);

              if (arrVal && arrVal->IsBool()) {
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShaderConstantB))
                goto error;
              // startRegister
              if (!self->m_PipeServer->WriteUInt32(
                      arguments[0]->GetUIntValue()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)arrLen))
                goto error;
              for (int i = 0; i < arrLen; ++i) {
                auto arrVal = arguments[1]->GetValue(i);
                if (!self->m_PipeServer->WriteBoolean(arrVal->GetBoolValue()))
                  goto error;
              }

              return true;
            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("d3d9SetPixelShaderConstantF",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsArray()) {
            size_t arrLen = arguments[1]->GetArrayLength();

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[1]->GetValue(i);

              if (arrVal && arrVal->IsDouble()) {
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShaderConstantF))
                goto error;
              // startRegister
              if (!self->m_PipeServer->WriteUInt32(
                      arguments[0]->GetUIntValue()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)arrLen))
                goto error;
              for (int i = 0; i < arrLen; ++i) {
                auto arrVal = arguments[1]->GetValue(i);
                if (!self->m_PipeServer->WriteSingle(arrVal->GetDoubleValue()))
                  goto error;
              }

              return true;
            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9SetPixelShaderConstantI",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (2 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsArray()) {
            size_t arrLen = arguments[1]->GetArrayLength();

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[1]->GetValue(i);

              if (arrVal && arrVal->IsInt()) {
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              if (!self->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShaderConstantI))
                goto error;
              // startRegister
              if (!self->m_PipeServer->WriteUInt32(
                      arguments[0]->GetUIntValue()))
                goto error;
              if (!self->m_PipeServer->WriteUInt32((UINT32)arrLen))
                goto error;
              for (int i = 0; i < arrLen; ++i) {
                auto arrVal = arguments[1]->GetValue(i);
                if (!self->m_PipeServer->WriteInt32(arrVal->GetIntValue()))
                  goto error;
              }

              return true;
            error:
              self->Close();
              exceptionoverride = g_szConnectionError;
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9DrawPrimitive",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (3 == arguments.size() && arguments[0]->IsUInt() &&
              arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9DrawPrimitive))
              goto error;
            // primitiveType
            if (!self->m_PipeServer->WriteUInt32(arguments[0]->GetUIntValue()))
              goto error;
            // startVertex
            if (!self->m_PipeServer->WriteUInt32(arguments[1]->GetUIntValue()))
              goto error;
            // primitiveCount
            if (!self->m_PipeServer->WriteUInt32(arguments[2]->GetUIntValue()))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "d3d9DrawIndexedPrimitive",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (6 == arguments.size() &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt() &&
              arguments[4]->IsUInt() && arguments[5]->IsUInt()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::D3d9DrawIndexedPrimitive))
              goto error;
            // primitiveType
            if (!self->m_PipeServer->WriteUInt32(arguments[0]->GetUIntValue()))
              goto error;
            // baseVertexIndex
            if (!self->m_PipeServer->WriteUInt32(arguments[1]->GetUIntValue()))
              goto error;
            // minVertexIndex
            if (!self->m_PipeServer->WriteUInt32(arguments[2]->GetUIntValue()))
              goto error;
            // numVertices
            if (!self->m_PipeServer->WriteUInt32(arguments[3]->GetUIntValue()))
              goto error;
            // startIndex
            if (!self->m_PipeServer->WriteUInt32(arguments[4]->GetUIntValue()))
              goto error;
            // primCount
            if (!self->m_PipeServer->WriteUInt32(arguments[5]->GetUIntValue()))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
   afxObject->AddFunction("waitForClientGpu",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (0 == arguments.size()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::WaitForGpu))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("beginCleanState",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
         if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (0 == arguments.size()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::BeginCleanState))
              goto error;

            return true;
          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("endCleanState",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (!self->m_InFlow) {
            exceptionoverride = g_szOutOfFlow;
            return true;
          }

          if (0 == arguments.size()) {
            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::EndCleanState))
              goto error;

            return true;

          error:
            self->Close();
            exceptionoverride = g_szConnectionError;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction("createDrawingData",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0]->IsUInt()) {
            unsigned int size = arguments[0]->GetUIntValue();
            if (void* pData = malloc(size)) {
              retval = CAfxData::Create(size, pData);
              return true;
            }

            exceptionoverride = "Memory allocation failed.";
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    afxObject->AddFunction("waitForCefFrame",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
  
          std::unique_lock<std::mutex> lock(self->m_PaintedMutex);
          self->m_PaintedCv.wait(lock, [self] { return self->m_Painted; });

          retval = CAfxHandle::Create(self->m_ShareHandle, nullptr);
          return true;
        });


    //

    if (out)
      *out = self;
    
    return afxObject->GetObj();
  }

  virtual void CloseInterop() override { Close(); }

 private:
  void ProcessAfxMessage(int senderId, const CefString& message) {
    CefV8ValueList execArgs;
    execArgs.push_back(CefV8Value::CreateInt(senderId));
    execArgs.push_back(CefV8Value::CreateString(message));

    if (m_OnMessage->IsValid())
      m_OnMessage->ExecuteCallback(execArgs);
  }
  void ProcessAfxError(int senderId, const CefString& message) {
    CefV8ValueList execArgs;
    execArgs.push_back(CefV8Value::CreateInt(senderId));
    execArgs.push_back(CefV8Value::CreateString(message));

    if (m_OnError->IsValid())
      m_OnError->ExecuteCallback(execArgs);
  }

 public:


  virtual bool OnProcessMessageReceived(
          CefRefPtr<CefBrowser> browser,
          CefRefPtr<CefFrame> frame,
          CefProcessId source_process,
          CefRefPtr<CefProcessMessage> message) override {
        auto const name = message->GetName();

        if (name == "afx-message") {
          auto const args = message->GetArgumentList();

          CefPostTask(TID_RENDERER,
                      base::Bind(&CDrawingInteropImpl::ProcessAfxMessage, this, args->GetInt(0),
                                 args->GetString(1)));

          return true;
        } else if (name == "afx-error") {
          auto const args = message->GetArgumentList();

          CefPostTask(TID_RENDERER,
                      base::Bind(&CDrawingInteropImpl::ProcessAfxError, this,
                                 args->GetInt(0),
                                 args->GetString(1)));

          return true;
        } else if (name == "afx-painted") {
          auto const args = message->GetArgumentList();
          {
            std::lock_guard<std::mutex> lock(m_PaintedMutex);
            m_Painted = true;
            m_ShareHandle =
                (HANDLE)(
                (unsigned __int64)(unsigned int)args->GetInt(0) |
                ((unsigned __int64)(unsigned int)args->GetInt(1) << 32));
          }
          m_PaintedCv.notify_one();

          return true;
        }

        return false;
      }

 protected:
  CDrawingInteropImpl() : CAfxInterop("advancedfxInterop_drawing") {}

  virtual void OnClosing() override { m_InFlow = false; }

 private:

  class CAfxInteropImpl : public CefBaseRefCounted {
   public:
    UINT64 GetIndex() const { return (UINT64)this; }
  };

class CAfxHandle : public CAfxInteropImpl {
    IMPLEMENT_REFCOUNTING(CAfxHandle);

   public:
    static CefRefPtr<CefV8Value> Create(HANDLE handle, CefRefPtr<CAfxHandle>* out = nullptr) {
      CefRefPtr<CAfxHandle> val = new CAfxHandle();

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddGetter(
          "lo", [val](const CefString& name, const CefRefPtr<CefV8Value> object,
                      CefRefPtr<CefV8Value>& retval, CefString& exception) {
                retval =  CefV8Value::CreateUInt(val->GetLo());
                return true;
          });
      afxObject->AddGetter(
          "hi", [val](const CefString& name, const CefRefPtr<CefV8Value> object,
                      CefRefPtr<CefV8Value>& retval, CefString& exception) {
            retval = CefV8Value::CreateUInt(val->GetHi());
            return true;
          });

      if (out)
        *out = val;

      afxObject->SetUserData(new CAfxUserData(
          AfxUserDataType::AfxHandle, val.get()));
      return afxObject->GetObj();
    }

    CAfxHandle() : CAfxInteropImpl() ,m_Handle(INVALID_HANDLE_VALUE) {     
    }

    CAfxHandle(HANDLE value) : CAfxInteropImpl(), m_Handle(value) {}

    CAfxHandle(unsigned int lo, unsigned int hi) : CAfxInteropImpl(), m_Handle((void *)((unsigned __int64)lo | ((unsigned __int64)hi << 32))) {}

    HANDLE GetHandle() { return m_Handle;
    }

    unsigned int GetLo() {        
        return (unsigned int)((unsigned __int64)m_Handle & 0xffffffff);
    }

    unsigned int GetHi() {
      return (unsigned int)(((unsigned __int64)m_Handle >> 32) & 0xffffffff);
    }

   private:
       HANDLE m_Handle;
  };

  class CAfxData : public CAfxInteropImpl,
                          public CefV8ArrayBufferReleaseCallback {
    IMPLEMENT_REFCOUNTING(CAfxData);

   public:
    static CefRefPtr<CefV8Value> Create(unsigned int size,
                                        void* data,
                                        CefRefPtr<CAfxData> *out = nullptr) {
      CefRefPtr<CAfxData> val = new CAfxData(size, data);
      CefRefPtr<CefV8Value> buffer =
          CefV8Value::CreateArrayBuffer(data, size, val);

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddGetter(
          "buffer",
          [buffer](const CefString& name, const CefRefPtr<CefV8Value> object,
                   CefRefPtr<CefV8Value>& retval, CefString& exception) {
            retval = buffer;
            return true;
          });

      if (out)
        *out = val;

      afxObject->SetUserData(
          new CAfxUserData(AfxUserDataType::AfxData, val.get()));
      return afxObject->GetObj();
    }

    virtual void ReleaseBuffer(void* buffer) override { free(buffer); }

    size_t GetSize() { return m_Size; }

    void* GetData() { return m_Data; }


   private:

    CAfxData(unsigned int size, void* data)
        : CAfxInteropImpl(),
          m_Size(size),
          m_Data(data) {
    }

    size_t m_Size;
    void* m_Data;
  };

  class CAfxD3d9VertexDeclaration : public CAfxInteropImpl {
    IMPLEMENT_REFCOUNTING(CAfxD3d9VertexDeclaration);

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> drawingInteropImpl,
        CefRefPtr<CAfxD3d9VertexDeclaration>* out = nullptr) {
      CefRefPtr<CAfxD3d9VertexDeclaration> val =
          new CAfxD3d9VertexDeclaration();

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddFunction(
          "release", [val, drawingInteropImpl](
                         const CefString& name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) {
            if (val->m_DoReleased) {
              exception = g_szAlreadyReleased;
              return true;
            }

            if (!drawingInteropImpl->m_InFlow) {
              exception = g_szOutOfFlow;
              return true;
            }

            if (name == "release") {
              if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::ReleaseD3d9VertexDeclaration))
                goto error;
              if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                      (UINT64)val->GetIndex()))
                goto error;

              val->m_DoReleased = true;
              return true;
            error:
              drawingInteropImpl->Close();
              exception = g_szConnectionError;
              return true;
            }

            return false;
          });

      if (out)
        *out = val;

      afxObject->SetUserData(
          new CAfxUserData(AfxUserDataType::AfxD3d9VertexDeclaration, val.get()));
      return afxObject->GetObj();
    }

    CAfxD3d9VertexDeclaration()
        : CAfxInteropImpl() {
    }

  private:
    bool m_DoReleased = false;
  };

  class CAfxD3d9IndexBuffer : public CAfxInteropImpl {
    IMPLEMENT_REFCOUNTING(CAfxD3d9IndexBuffer);

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> drawingInteropImpl,
        CefRefPtr<CAfxD3d9IndexBuffer>* out = nullptr) {
      CefRefPtr<CAfxD3d9IndexBuffer> val = new CAfxD3d9IndexBuffer();

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddFunction("release", [val, drawingInteropImpl](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exception) {
        if (val->m_DoReleased) {
          exception = g_szAlreadyReleased;
          return true;
        }

        if (!drawingInteropImpl->m_InFlow) {
          exception = g_szOutOfFlow;
          return true;
        }

        if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                (UINT32)DrawingReply::ReleaseD3d9IndexBuffer))
          goto error_1;
        if (!drawingInteropImpl->m_PipeServer->WriteUInt64((UINT64)val->GetIndex()))
          goto error_1;

        val->m_DoReleased = true;
        return true;
      error_1:
        drawingInteropImpl->Close();
        exception = g_szConnectionError;
        return true;
      });
      afxObject->AddFunction("update", [val, drawingInteropImpl](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exception) {
        if (val->m_DoReleased) {
          exception = g_szAlreadyReleased;
          return true;
        }

        if (!drawingInteropImpl->m_InFlow) {
          exception = g_szOutOfFlow;
          return true;
        }
      
          if (3 == arguments.size() &&
            arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
          auto data = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(arguments[0]);

          if (nullptr != data) {
            size_t offsetToLock = (int)arguments[0]->GetUIntValue();
            size_t sizeToLock = (int)arguments[1]->GetUIntValue();

            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::UpdateD3d9IndexBuffer))
              goto error_2;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                    (UINT64)val->GetIndex()))
              goto error_2;
            // offsetToLock
            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (UINT32)offsetToLock))
              goto error_2;
            // sizeToLock
            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (UINT32)sizeToLock))
              goto error_2;
            if (!drawingInteropImpl->m_PipeServer->WriteBytes(
                    (unsigned char*)data->GetData() + offsetToLock,
                    (DWORD)offsetToLock, (DWORD)sizeToLock))
              goto error_2;

          error_2:
            drawingInteropImpl->Close();
            exception = g_szConnectionError;
            return true;
          }
        }

          exception = g_szInvalidArguments;
          return true;
      });

      if (out)
        *out = val;

      afxObject->SetUserData(
          new CAfxUserData(AfxUserDataType::AfxD3d9IndexBuffer, val.get()));
      return afxObject->GetObj();
    }

    CAfxD3d9IndexBuffer()
        : CAfxInteropImpl() {
    }
      
   private:
    bool m_DoReleased = false;
  };

  class CAfxD3d9VertexBuffer : public CAfxInteropImpl {
    IMPLEMENT_REFCOUNTING(CAfxD3d9VertexBuffer);

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> drawingInteropImpl,
        CefRefPtr<CAfxD3d9VertexBuffer>* out = nullptr) {
      CefRefPtr<CAfxD3d9VertexBuffer> val = new CAfxD3d9VertexBuffer();

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddFunction(
          "release", [val, drawingInteropImpl](
                         const CefString& name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) {
            if (val->m_DoReleased) {
              exception = g_szAlreadyReleased;
              return true;
            }

            if (!drawingInteropImpl->m_InFlow) {
              exception = g_szOutOfFlow;
              return true;
            }

            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9VertexBuffer))
              goto error_1;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                    (UINT64)val->GetIndex()))
              goto error_1;

            val->m_DoReleased = true;
            return true;
          error_1:
            drawingInteropImpl->Close();
            exception = g_szConnectionError;
            return true;
          });
      afxObject->AddFunction(
          "update", [val, drawingInteropImpl](
                        const CefString& name, CefRefPtr<CefV8Value> object,
                        const CefV8ValueList& arguments,
                        CefRefPtr<CefV8Value>& retval, CefString& exception) {
            if (val->m_DoReleased) {
              exception = g_szAlreadyReleased;
              return true;
            }

            if (!drawingInteropImpl->m_InFlow) {
              exception = g_szOutOfFlow;
              return true;
            }

            if (3 == arguments.size() &&
                arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
              auto data = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(
                  arguments[0]);

              if (nullptr != data) {
                size_t offsetToLock = (int)arguments[0]->GetUIntValue();
                size_t sizeToLock = (int)arguments[1]->GetUIntValue();

                if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                        (UINT32)DrawingReply::UpdateD3d9VertexBuffer))
                  goto error_2;
                if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                        (UINT64)val->GetIndex()))
                  goto error_2;
                // offsetToLock
                if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                        (UINT32)offsetToLock))
                  goto error_2;
                // sizeToLock
                if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                        (UINT32)sizeToLock))
                  goto error_2;
                if (!drawingInteropImpl->m_PipeServer->WriteBytes(
                        (unsigned char*)data->GetData() + offsetToLock,
                        (DWORD)offsetToLock, (DWORD)sizeToLock))
                  goto error_2;

              error_2:
                drawingInteropImpl->Close();
                exception = g_szConnectionError;
                return true;
              }
            }

            exception = g_szInvalidArguments;
            return true;
          });

      if (out)
        *out = val;

      afxObject->SetUserData(
          new CAfxUserData(AfxUserDataType::AfxD3d9VertexBuffer, val.get()));
      return afxObject->GetObj();
    }

    CAfxD3d9VertexBuffer()
        : CAfxInteropImpl() {
    }


   private:
    bool m_DoReleased = false;
  };

  class CAfxD3d9Texture : public CAfxInteropImpl {
    IMPLEMENT_REFCOUNTING(CAfxD3d9Texture);

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> drawingInteropImpl,
        CefRefPtr<CAfxD3d9Texture>* out = nullptr) {
      CefRefPtr<CAfxD3d9Texture> val = new CAfxD3d9Texture();

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddFunction(
          "release", [val, drawingInteropImpl](
                         const CefString& name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) {
            if (val->m_DoReleased) {
              exception = g_szAlreadyReleased;
              return true;
            }

            if (!drawingInteropImpl->m_InFlow) {
              exception = g_szOutOfFlow;
              return true;
            }

            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9Texture))
              goto error_1;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                    (UINT64)val->GetIndex()))
              goto error_1;

            val->m_DoReleased = true;
            return true;
          error_1:
            drawingInteropImpl->Close();
            exception = g_szConnectionError;
            return true;
          });
      afxObject->AddFunction(
          "update", [val, drawingInteropImpl](
                        const CefString& name, CefRefPtr<CefV8Value> object,
                        const CefV8ValueList& arguments,
                        CefRefPtr<CefV8Value>& retval, CefString& exception) {
            if (val->m_DoReleased) {
              exception = g_szAlreadyReleased;
              return true;
            }

            if (!drawingInteropImpl->m_InFlow) {
              exception = g_szOutOfFlow;
              return true;
            }

            if (8 == arguments.size() && arguments[0]->IsUInt() &&
            arguments[1]->IsObject() &&
            arguments[3]->IsUInt() && arguments[4]->IsUInt() &&
            arguments[5]->IsUInt() && arguments[6]->IsUInt() &&
            arguments[7]->IsUInt()) {
              auto data = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(
                  arguments[2]);

          auto rectLeft = arguments[1]->GetValue("left");
          auto rectTop = arguments[1]->GetValue("top");
          auto rectRight = arguments[1]->GetValue("right");
          auto rectBottom = arguments[1]->GetValue("bottom");

          UINT32 rowOffsetBytes = arguments[3]->GetUIntValue();
          UINT32 columnOffsetBytes = arguments[4]->GetUIntValue();
          UINT32 dataBytesPerRow = arguments[5]->GetUIntValue();
          UINT32 totalBytesPerRow = arguments[6]->GetUIntValue();
          UINT32 numRows = arguments[7]->GetUIntValue();

          if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                  (UINT32)DrawingReply::UpdateD3d9Texture))
            goto error_2;
          if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                  (UINT64)val->GetIndex()))
            goto error_2;
          if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                  (UINT32)arguments[0]->GetUIntValue()))
            goto error_2;
          if (nullptr != rectLeft && nullptr != rectTop &&
              nullptr != rectRight && nullptr != rectBottom &&
              rectLeft->IsInt() && rectTop->IsInt() && rectRight->IsInt() &&
              rectBottom->IsInt()) {
            if (!drawingInteropImpl->m_PipeServer->WriteBoolean(true))
              goto error_2;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (INT32)rectLeft->GetIntValue()))
              goto error_2;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (INT32)rectTop->GetIntValue()))
              goto error_2;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (INT32)rectRight->GetIntValue()))
              goto error_2;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (INT32)rectBottom->GetIntValue()))
              goto error_2;
          } else {
            if (!drawingInteropImpl->m_PipeServer->WriteBoolean(false))
              goto error_2;
          }
          if (!drawingInteropImpl->m_PipeServer->WriteUInt32((UINT32)numRows))
            goto error_2;
          if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                  (UINT32)(dataBytesPerRow - columnOffsetBytes)))
            goto error_2;

          void* pData = (unsigned char*)data->GetData() + rowOffsetBytes;

          for (unsigned int i = 0; i < numRows; ++i) {
            if (!drawingInteropImpl->m_PipeServer->WriteBytes(
                    (unsigned char*)pData + columnOffsetBytes, 0,
                    dataBytesPerRow - columnOffsetBytes))
              goto error_2;

            pData = (unsigned char*)pData + totalBytesPerRow;
          }
          return true;
        error_2:
              drawingInteropImpl->Close();
          exception = g_szConnectionError;
          return true;
        }

        exception = g_szInvalidArguments;
        return true;
          });

      if (out)
        *out = val;

      afxObject->SetUserData(
          new CAfxUserData(AfxUserDataType::AfxD3d9Texture, val.get()));
      return afxObject->GetObj();
    }


    CAfxD3d9Texture()
        : CAfxInteropImpl() {
    }

   private:
    bool m_DoReleased = false;
  };

  class CAfxD3d9PixelShader : public CAfxInteropImpl {
   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> drawingInteropImpl,
        CefRefPtr<CAfxD3d9PixelShader>* out = nullptr) {
      CefRefPtr<CAfxD3d9PixelShader> val = new CAfxD3d9PixelShader();

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddFunction(
          "release", [val, drawingInteropImpl](
                         const CefString& name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) {
            if (val->m_DoReleased) {
              exception = g_szAlreadyReleased;
              return true;
            }

            if (!drawingInteropImpl->m_InFlow) {
              exception = g_szOutOfFlow;
              return true;
            }

            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9PixelShader))
              goto error_1;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                    (UINT64)val->GetIndex()))
              goto error_1;

            val->m_DoReleased = true;
            return true;
          error_1:
            drawingInteropImpl->Close();
            exception = g_szConnectionError;
            return true;
          });
 
      if (out)
        *out = val;

      afxObject->SetUserData(
          new CAfxUserData(AfxUserDataType::AfxD3d9PixelShader, val.get()));
      return afxObject->GetObj();
    }

    CAfxD3d9PixelShader()
        : CAfxInteropImpl() {
    }


   private:
    bool m_DoReleased = false;

     IMPLEMENT_REFCOUNTING(CAfxD3d9PixelShader);
  };

  class CAfxD3d9VertexShader : public CAfxInteropImpl {
    IMPLEMENT_REFCOUNTING(CAfxD3d9VertexShader);

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> drawingInteropImpl,
        CefRefPtr<CAfxD3d9VertexShader>* out = nullptr) {
      CefRefPtr<CAfxD3d9VertexShader> val = new CAfxD3d9VertexShader();

      CefRefPtr<CAfxObject> afxObject = new CAfxObject();
      afxObject->AddFunction(
          "release", [val, drawingInteropImpl](
                         const CefString& name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) {
            if (val->m_DoReleased) {
              exception = g_szAlreadyReleased;
              return true;
            }

            if (!drawingInteropImpl->m_InFlow) {
              exception = g_szOutOfFlow;
              return true;
            }

            if (!drawingInteropImpl->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9VertexShader))
              goto error_1;
            if (!drawingInteropImpl->m_PipeServer->WriteUInt64(
                    (UINT64)val->GetIndex()))
              goto error_1;

            val->m_DoReleased = true;
            return true;
          error_1:
            drawingInteropImpl->Close();
            exception = g_szConnectionError;
            return true;
          });

      if (out)
        *out = val;

      afxObject->SetUserData(
          new CAfxUserData(AfxUserDataType::AfxD3d9VertexShader, val.get()));
      return afxObject->GetObj();
    }

    CAfxD3d9VertexShader()
        : CAfxInteropImpl() {

    }

   private:
    bool m_DoReleased = false;
  };

  bool m_InFlow = false;

  HANDLE m_ShareHandle = INVALID_HANDLE_VALUE;
  int m_Width = 640;
  int m_Height = 480;

  bool m_Painted;
  std::mutex m_PaintedMutex;
  std::condition_variable m_PaintedCv;

  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;
  CefRefPtr<CAfxCallback> m_OnDeviceLost;
  CefRefPtr<CAfxCallback> m_OnDeviceReset;

  bool DoPumpBegin(int frameCount, unsigned int pass) {
       m_InFlow = false;

    while (true) {
      INT32 drawingMessage;
      if (!m_PipeServer->ReadInt32(drawingMessage))
        return false;

      bool bContinue = false;

      switch ((DrawingMessage)drawingMessage) {
        case DrawingMessage::BeforeTranslucentShadow:
        case DrawingMessage::AfterTranslucentShadow:
        case DrawingMessage::BeforeTranslucent:
        case DrawingMessage::AfterTranslucent:
        case DrawingMessage::BeforeHud:
        case DrawingMessage::AfterHud:
        case DrawingMessage::OnRenderViewEnd:
        case DrawingMessage::NOP:
          break;
        case DrawingMessage::DeviceLost: {
          CefV8ValueList execArgs;
          if (m_OnDeviceLost->IsValid())
            m_OnDeviceLost->ExecuteCallback(execArgs);
        }
          bContinue = true;
          break;
        case DrawingMessage::DeviceRestored: {
          CefV8ValueList execArgs;
          if (m_OnDeviceReset->IsValid())
            m_OnDeviceReset->ExecuteCallback(execArgs);
        }
          bContinue = true;
          break;

        default:
          return false;
      }

      if (bContinue)
        continue;

      INT32 clientFrameCount;
      if(!m_PipeServer->ReadInt32(clientFrameCount))
          return false;

      UINT32 clientPass;
      if (!m_PipeServer->ReadUInt32(clientPass))
        return false;

      INT32 frameDiff = frameCount - clientFrameCount;

      if (frameDiff < 0 || frameDiff == 0 && pass < clientPass) {
        // Error: client is ahead, otherwise we would have correct
        // data by now.

        if (!m_PipeServer->WriteInt32((INT32)DrawingReply::Retry))
          return false;

        if(!m_PipeServer->Flush())
            return false;

        // OutputDebugStringA("DrawingReply::Retry\n");

        return true;

      } else if (frameDiff > 0 || frameDiff == 0 && pass > clientPass) {
        // client is behind.

        if (!m_PipeServer->WriteInt32((INT32)DrawingReply::Skip))
          return false;

        if(!m_PipeServer->Flush())
            return false;

        // OutputDebugStringA("DrawingReply::Skip\n");
      } else {
        // we are right on.

        m_InFlow = true;
        return true;
      }
    }
  }



  struct CBinaryData {
    void* Data;
    size_t Size;

    CBinaryData(const CefRefPtr<CefBinaryValue> & data) {
      Size = data->GetSize();
      Data = malloc(Size);
      if(Data) {
        data->GetData(Data, Size, 0);
      }
    }

    ~CBinaryData() { free(Data);
    }
  };

  std::vector<bool> BoolVecFromCefArgs(const CefRefPtr<CefListValue>& args,
                                         size_t ofs,
                                         size_t count) {
    std::vector<bool> result(count);

    for (size_t i = 0; i < count; ++i) {
      result[i] = (bool)args->GetBool(ofs + i);
    }

    return result;
  }

  std::vector<float> FloatVecFromCefArgs(const CefRefPtr<CefListValue>& args, size_t ofs, size_t count) {
    std::vector<float> result(count);

    for (size_t i = 0; i < count; ++i) {
      result[i] = (float)args->GetDouble(ofs + i);
    }

    return result;
  }

  std::vector<int> IntVecFromCefArgs(const CefRefPtr<CefListValue>& args,
                                       size_t ofs,
                                       size_t count) {
    std::vector<int> result(count);

    for (size_t i = 0; i < count; ++i) {
      result[i] = (int)args->GetInt(ofs + i);
    }

    return result;
  }
};

CefRefPtr<CefV8Value> CreateDrawingInterop(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context,
                                           const CefString& argStr,
                                           CefRefPtr<CInterop>* out) {
  return CDrawingInteropImpl::Create(browser,frame,context,argStr, out);
}

class CCreateInterop {
 public:
  static void FreeAll() {
    while (!m_Lingering.empty()) {
      auto it = m_Lingering.begin();
      delete (*it);
      m_Lingering.erase(it);
    }
  }

  static void Resolve(unsigned int lo, unsigned int hi, int interopId) {
    CCreateInterop* val =
        (CCreateInterop*)((unsigned __int64)lo | ((unsigned __int64)hi << 32));

    auto it = m_Lingering.find(val);
    if (it != m_Lingering.end()) {
      m_Lingering.erase(it);
      val->DoResolve(interopId);
      delete val;
    }
  }

  static void Reject(unsigned int lo,
                     unsigned int hi,
                     const CefString& message) {
    CCreateInterop* val =
        (CCreateInterop*)((unsigned __int64)lo | ((unsigned __int64)hi << 32));

    auto it = m_Lingering.find(val);
    if (it != m_Lingering.end()) {
      m_Lingering.erase(it);
      val->DoReject(message);
      delete val;
    }
  }

  CCreateInterop(CefRefPtr<CefV8Context> context,
                 CefRefPtr<CefV8Value> fnResolve,
                 CefRefPtr<CefV8Value> fnReject)
      : m_Context(context), m_FnResolve(fnResolve), m_FnReject(fnReject) {
    m_Lingering.insert(this);
  }

  unsigned int GetLo() {
    return (unsigned int)((unsigned __int64)this & 0xffffffff);
  }

  unsigned int GetHi() {
    return (unsigned int)(((unsigned __int64)this >> 32) & 0xffffffff);
  }

 private:
  static std::set<CCreateInterop*> m_Lingering;
  CefRefPtr<CefV8Context> m_Context;
  CefRefPtr<CefV8Value> m_FnResolve;
  CefRefPtr<CefV8Value> m_FnReject;

  void DoResolve(int interopId) {
    CefV8ValueList arguments;
    arguments.push_back(CefV8Value::CreateInt(interopId));
    // TODO: Handle exceptions.
    m_FnResolve->ExecuteFunctionWithContext(m_Context, NULL, arguments);
  }

  void DoReject(const CefString& message) {
    CefV8ValueList arguments;
    arguments.push_back(CefV8Value::CreateString(message));
    // TODO: Handle exceptions.
    m_FnResolve->ExecuteFunctionWithContext(m_Context, NULL, arguments);
  }
};

std::set<CCreateInterop*> CCreateInterop::m_Lingering;

class CEngineInteropImpl : public CInterop,
                           public CAfxInterop {
  IMPLEMENT_REFCOUNTING(CEngineInteropImpl);

public:

  static CefRefPtr<CefV8Value> Create(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context,
                                     const CefString& argStr,
                                     CefRefPtr<CInterop>* out = nullptr) {
   CefRefPtr<CEngineInteropImpl> self = new CEngineInteropImpl();

   CefRefPtr<CAfxObject> afxObject = new CAfxObject();

   //

   afxObject->AddGetter("id",
       [browser](const CefString& name, const CefRefPtr<CefV8Value> object,
                 CefRefPtr<CefV8Value>& retval, CefString& exception) {
         retval = CefV8Value::CreateInt(browser->GetIdentifier());

         return true;
       });

   afxObject->AddFunction("init",
       [context, obj = afxObject->GetObj(), argStr](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (1 == arguments.size()) {
                 auto argInitFn = arguments[0];

                 if (argInitFn->IsFunction()) {
                   CefV8ValueList args;
                   args.push_back(obj);
                   args.push_back(CefV8Value::CreateString(argStr));

                   argInitFn->ExecuteFunctionWithContext(context, NULL, args);

                   return true;
                 }
               }

               return false;
             });

   afxObject->AddFunction("sendMessage",
             [frame](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (arguments.size() == 2) {
                 auto argId = arguments[0];
                 auto argStr = arguments[1];

                 if (argId && argStr && argId->IsInt() && argStr->IsString()) {
                   auto message = CefProcessMessage::Create("afx-send-message");
                   auto args = message->GetArgumentList();
                   args->SetSize(2);
                   args->SetInt(0, argId->GetIntValue());
                   args->SetString(1, argStr->GetStringValue());

                   frame->SendProcessMessage(PID_BROWSER, message);
                   return true;
                 }
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });

   self->m_OnMessage = afxObject->AddCallback("onMessage", context);

   self->m_OnError = afxObject->AddCallback("onError", context);

  afxObject->AddGetter("pipeName",
       [self](const CefString& name, const CefRefPtr<CefV8Value> object,
              CefRefPtr<CefV8Value>& retval, CefString& exception) {
         retval = CefV8Value::CreateString(self->GetPipeName());

         return true;
       });
   afxObject->AddSetter(
       "pipeName",
       [self](const CefString& name, const CefRefPtr<CefV8Value> object,
              const CefRefPtr<CefV8Value> value, CefString& exception) {
         if (value && value->IsString()) {
           std::string pipeName(value->GetStringValue().ToString());

           self->SetPipeName(pipeName.c_str());
           return true;
         }

         exception = g_szInvalidArguments;
         return true;
       });

   afxObject->AddFunction(
       "connect",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         retval = CefV8Value::CreateBool(self->Connection() &&
                                         self->Connected());
         return true;
       });

   afxObject->AddFunction(
       "close",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         self->Close();
         return true;
       });

   self->m_OnNewConnection = afxObject->AddCallback("onNewConnection", context);

   self->m_OnCommands = afxObject->AddCallback("onCommands", context);

   afxObject->AddFunction("scheduleCommand",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 <= arguments.size()) {
           const CefRefPtr<CefV8Value>& arg0 = arguments[0];
           if (arg0->IsString()) {
             self->m_Commands.emplace(arg0->GetStringValue().ToString().c_str());
             return true;
           }
         }

         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   self->m_OnRenderViewBegin = afxObject->AddCallback("onRenderViewBegin", context);

   self->m_OnViewOverride = afxObject->AddCallback("onViewOverride", context);

   self->m_OnRenderPass = afxObject->AddCallback("onRenderPass", context);

   self->m_OnHudBegin = afxObject->AddCallback("onHudBegin", context);

   self->m_OnHudEnd = afxObject->AddCallback("onHudEnd", context);

   self->m_OnRenderViewEnd = afxObject->AddCallback("onRenderViewEnd", context);

   afxObject->AddFunction("addCalcHandle",
             [self, context](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() && arguments[0]->IsString() && arguments[1]->IsFunction()) {
           retval = CCalcResult::Create(
                          &self->m_HandleCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CAfxCallback(arguments[1], context));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   afxObject->AddFunction("addCalcVecAng",
             [self, context](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                    CCalcResult::Create(
                          &self->m_VecAngCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1], context));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   afxObject->AddFunction("addCalcCam",
             [self, context](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                     CCalcResult::Create(
                          &self->m_CamCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CAfxCallback(arguments[1], context));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   afxObject->AddFunction("addCalcFov",
             [self, context](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                     CCalcResult::Create(
                          &self->m_FovCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CAfxCallback(arguments[1], context));

                 return true;
               }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction(
       "addCalcBool",
       [self, context](const CefString& name, CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exceptionoverride) {
         if (2 == arguments.size() &&
             arguments[0]->IsString() && arguments[1]->IsFunction()) {
           std::string valName = arguments[0]->GetStringValue().ToString();

           retval = CCalcResult::Create(
                         &self->m_BoolCalcCallbacks,
                         arguments[0]->GetStringValue().ToString().c_str(),
                         new CAfxCallback(arguments[1], context));

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction(
       "addCalcInt",
       [self, context](const CefString& name, CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exceptionoverride) {
         if (2 == arguments.size() &&
             arguments[0]->IsString() && arguments[1]->IsFunction()) {
           std::string valName = arguments[0]->GetStringValue().ToString();

           retval = CCalcResult::Create(
                         &self->m_IntCalcCallbacks,
                         arguments[0]->GetStringValue().ToString().c_str(),
                         new CAfxCallback(arguments[1], context));

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   afxObject->AddFunction(
       "gameEventAllowAdd",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 == arguments.size() &&
             arguments[0]->IsString()) {
           std::string val = arguments[0]->GetStringValue().ToString();

           self->m_GameEventsAllowDeletions.erase(val);
           self->m_GameEventsAllowAdditions.insert(val);

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction(
       "gameEventAllowRemove",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 == arguments.size() &&
             arguments[0]->IsString()) {
           std::string val = arguments[0]->GetStringValue().ToString();

           self->m_GameEventsAllowAdditions.erase(val);
           self->m_GameEventsAllowDeletions.insert(val);

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction(
       "gameEventDenyAdd",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 == arguments.size() &&
             arguments[0]->IsString()) {
           std::string val = arguments[0]->GetStringValue().ToString();

           self->m_GameEventsDenyDeletions.erase(val);
           self->m_GameEventsDenyAdditions.insert(val);

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction("gameEventDenyRemove",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 == arguments.size() &&
             arguments[0]->IsString()) {
           std::string val = arguments[0]->GetStringValue().ToString();

           self->m_GameEventsDenyAdditions.erase(val);
           self->m_GameEventsDenyDeletions.insert(val);

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction(
       "gameEventSetEnrichment",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (3 == arguments.size() && arguments[0]->IsString() &&
             arguments[1]->IsString() && arguments[2]->IsUInt()) {
           std::string strEvent = arguments[0]->GetStringValue().ToString();
           std::string strProperty = arguments[1]->GetStringValue().ToString();
           unsigned int uiEnrichments = arguments[2]->GetUIntValue();

           self->m_GameEventsEnrichmentsChanges[GameEventEnrichmentKey_s(
               strEvent.c_str(), strProperty.c_str())] = uiEnrichments;

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   self->m_OnGameEvent = afxObject->AddCallback("onGameEvent", context);

   afxObject->AddFunction("gameEventSetTransmitClientTime",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 == arguments.size() && arguments[0]->IsBool()) {
           bool value = arguments[0]->GetBoolValue();

           self->m_GameEventsTransmitChanged =
               self->m_GameEventsTransmitChanged ||
               value != self->m_GameEventsTransmitClientTime;
           self->m_GameEventsTransmitClientTime = value;

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   afxObject->AddFunction(
       "gameEventSetTransmitTick",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 == arguments.size() && arguments[0]->IsBool()) {
           bool value = arguments[0]->GetBoolValue();

           self->m_GameEventsTransmitChanged =
               self->m_GameEventsTransmitChanged ||
               value != self->m_GameEventsTransmitTick;
           self->m_GameEventsTransmitTick = value;

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   afxObject->AddFunction(
       "gameEventSetTransmitSystemTime",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 == arguments.size() && arguments[0]->IsBool()) {
           bool value = arguments[0]->GetBoolValue();

           self->m_GameEventsTransmitChanged =
               self->m_GameEventsTransmitChanged ||
               value != self->m_GameEventsTransmitSystemTime;
           self->m_GameEventsTransmitSystemTime = value;

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   //

   if (out)
     *out = self;

   return afxObject->GetObj();
}

    virtual void CloseInterop() override { Close(); }

 private:
    void ProcessAfxMessage(int senderId, const CefString& message) {
        CefV8ValueList execArgs;
        execArgs.push_back(CefV8Value::CreateInt(senderId));
        execArgs.push_back(CefV8Value::CreateString(message));

        if (m_OnMessage->IsValid())
          m_OnMessage->ExecuteCallback(execArgs);
  }
    void ProcessAfxError(int senderId, const CefString& message) {
      CefV8ValueList execArgs;
      execArgs.push_back(CefV8Value::CreateInt(senderId));
      execArgs.push_back(CefV8Value::CreateString(message));

      if (m_OnError->IsValid())
        m_OnError->ExecuteCallback(execArgs);
    }

 public:

virtual bool OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) override {


    auto const name = message->GetName();

  if (name == "afx-message") {
    auto const args = message->GetArgumentList();

    CefPostTask(TID_RENDERER, base::Bind(&CEngineInteropImpl::ProcessAfxMessage, this, args->GetInt(0),
                                         args->GetString(1)));

    return true;

  } else if (name == "afx-error") {
    auto const args = message->GetArgumentList();
    auto const argC = args->GetSize();

    CefPostTask(TID_RENDERER,
                base::Bind(&CEngineInteropImpl::ProcessAfxError, this,
                                         args->GetInt(0), args->GetString(1)));
    return true;

  }

  return false;
}

 protected:

  CEngineInteropImpl() : CAfxInterop("advancedfxInterop") {}


  virtual bool OnConnection(void) override {
    bool done = false;

    bool outBeforeTranslucentShadow;
    bool outAfterTranslucentShadow;
    bool outBeforeTranslucent;
    bool outAfterTranslucent;
    bool outBeforeHud;
    bool outAfterHud;
    bool outAfterRenderView;

    while (!done) {
      UINT32 engineMessage;
      if (!m_PipeServer->ReadUInt32(engineMessage))
        return false;

      switch ((EngineMessage)engineMessage) {
        case EngineMessage::BeforeFrameStart: {
          // Read incoming commands from client:
          {
            CefV8ValueList args;

            UINT32 commandIndex = 0;
            UINT32 commandCount;
            if (!m_PipeServer->ReadCompressedUInt32(commandCount))
              return false;

            CefRefPtr<CefV8Value> objCommands =
                CefV8Value::CreateArray((int)commandCount);

            args.push_back(objCommands);

            while (0 < commandCount) {
              UINT32 argIndex = 0;
              UINT32 argCount;
              if (!m_PipeServer->ReadCompressedUInt32(argCount))
                return false;

              CefRefPtr<CefV8Value> objArgs =
                  CefV8Value::CreateArray((int)argCount);

              objCommands->SetValue((int)commandIndex, objArgs);

              while (0 < argCount) {
                std::string str;

                if (!m_PipeServer->ReadStringUTF8(str))
                  return false;

                objArgs->SetValue((int)argIndex, CefV8Value::CreateString(str));

                --argCount;
                ++argIndex;
              }

              --commandCount;
              ++commandIndex;
            }

            if (m_OnCommands->IsValid())
              m_OnCommands->ExecuteCallback(args);
          }

          if (!m_PipeServer->WriteCompressedUInt32((UINT32)m_Commands.size()))
            return false;

          while (!m_Commands.empty()) {
            if (!m_PipeServer->WriteStringUTF8(m_Commands.front().c_str()))
              return false;
            m_Commands.pop();
          }

          if (!m_PipeServer->Flush())
            return false;
        } break;

        case EngineMessage::BeforeFrameRenderStart: {
          if (!WriteGameEventSettings(true))
            return false;
          if (!m_PipeServer->Flush())
            return false;
        } break;

        case EngineMessage::AfterFrameRenderStart: {
          if (!m_HandleCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            return false;
          if (!m_VecAngCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            return false;
          if (!m_CamCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            return false;
          if (!m_FovCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            return false;
          if (!m_BoolCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            return false;
          if (!m_IntCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            return false;

          if (!m_PipeServer->Flush())
            return false;

          if (!m_HandleCalcCallbacks.BatchUpdateResult(m_PipeServer))
            return false;
          if (!m_VecAngCalcCallbacks.BatchUpdateResult(m_PipeServer))
            return false;
          if (!m_CamCalcCallbacks.BatchUpdateResult(m_PipeServer))
            return false;
          if (!m_FovCalcCallbacks.BatchUpdateResult(m_PipeServer))
            return false;
          if (!m_BoolCalcCallbacks.BatchUpdateResult(m_PipeServer))
            return false;
          if (!m_IntCalcCallbacks.BatchUpdateResult(m_PipeServer))
            return false;
        } break;

        case EngineMessage::OnRenderView: {
          struct RenderInfo_s renderInfo;

          if (!m_PipeServer->ReadInt32(renderInfo.FrameCount))
            return false;

          if (!m_PipeServer->ReadSingle(renderInfo.AbsoluteFrameTime))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.CurTime))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.FrameTime))
            return false;

          if (!m_PipeServer->ReadInt32(renderInfo.View.X))
            return false;
          if (!m_PipeServer->ReadInt32(renderInfo.View.Y))
            return false;
          if (!m_PipeServer->ReadInt32(renderInfo.View.Width))
            return false;
          if (!m_PipeServer->ReadInt32(renderInfo.View.Height))
            return false;

          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M00))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M01))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M02))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M03))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M10))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M11))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M12))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M13))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M20))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M21))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M22))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M23))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M30))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M31))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M32))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M33))
            return false;

          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M00))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M01))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M02))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M03))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M10))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M11))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M12))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M13))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M20))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M21))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M22))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M23))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M30))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M31))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M32))
            return false;
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M33))
            return false;

          outBeforeTranslucentShadow = false;
          outAfterTranslucentShadow = false;
          outBeforeTranslucent = false;
          outAfterTranslucent = false;
          outBeforeHud = false;
          outAfterHud = false;
          outAfterRenderView = false;

          if (m_OnRenderViewBegin->IsValid()) {
            CefV8ValueList args;
            args.push_back(CreateAfxRenderInfo(renderInfo));

            CefRefPtr<CefV8Value> retval =
                m_OnRenderViewBegin->ExecuteCallback(args);

            if (NULL != retval && retval->IsObject()) {
              CefRefPtr<CefV8Value> beforeTranslucentShadow =
                  retval->GetValue("beforeTranslucentShadow");
              if (beforeTranslucentShadow && beforeTranslucentShadow->IsBool())
                outBeforeTranslucentShadow =
                    beforeTranslucentShadow->GetBoolValue();

              CefRefPtr<CefV8Value> afterTranslucentShadow =
                  retval->GetValue("afterTranslucentShadow");
              if (afterTranslucentShadow && afterTranslucentShadow->IsBool())
                outAfterTranslucentShadow =
                    afterTranslucentShadow->GetBoolValue();

              CefRefPtr<CefV8Value> beforeTranslucent =
                  retval->GetValue("beforeTranslucent");
              if (beforeTranslucent && beforeTranslucent->IsBool())
                outBeforeTranslucent = beforeTranslucent->GetBoolValue();

              CefRefPtr<CefV8Value> afterTranslucent =
                  retval->GetValue("afterTranslucent");
              if (afterTranslucent && afterTranslucent->IsBool())
                outAfterTranslucent = afterTranslucent->GetBoolValue();

              CefRefPtr<CefV8Value> beforeHud = retval->GetValue("beforeHud");
              if (beforeHud && beforeHud->IsBool())
                outBeforeHud = beforeHud->GetBoolValue();

              CefRefPtr<CefV8Value> afterHud = retval->GetValue("afterHud");
              if (afterHud && afterHud->IsBool())
                outAfterHud = afterHud->GetBoolValue();

              CefRefPtr<CefV8Value> afterRenderView =
                  retval->GetValue("afterRenderView");
              if (afterRenderView && afterRenderView->IsBool())
                outAfterRenderView = afterRenderView->GetBoolValue();
            }
          }

          if (!m_PipeServer->WriteBoolean(outBeforeTranslucentShadow))
            return false;
          if (!m_PipeServer->WriteBoolean(outAfterTranslucentShadow))
            return false;
          if (!m_PipeServer->WriteBoolean(outBeforeTranslucent))
            return false;
          if (!m_PipeServer->WriteBoolean(outAfterTranslucent))
            return false;
          if (!m_PipeServer->WriteBoolean(outBeforeHud))
            return false;
          if (!m_PipeServer->WriteBoolean(outAfterHud))
            return false;
          if (!m_PipeServer->WriteBoolean(outAfterRenderView))
            return false;

          if (!(outBeforeTranslucentShadow || outAfterTranslucentShadow ||
                outBeforeTranslucent || outAfterTranslucent || outBeforeHud ||
                outAfterHud || outAfterRenderView))
            done = true;

          if (!done) {
            // Handle render pass here if needed ...
          }
        } break;

        case EngineMessage::OnRenderViewEnd:
          if (m_OnRenderViewEnd->IsValid())
            m_OnRenderViewEnd->ExecuteCallback(CefV8ValueList());
          done = true;
          break;

        case EngineMessage::BeforeHud:
          if (m_OnHudBegin->IsValid())
            m_OnHudBegin->ExecuteCallback(CefV8ValueList());
          break;

        case EngineMessage::AfterHud:
          if (m_OnHudEnd->IsValid())
            m_OnHudEnd->ExecuteCallback(CefV8ValueList());
          break;

        case EngineMessage::BeforeTranslucentShadow:
          if (!DoRenderPass(RenderPassType_BeforeTranslucentShadow))
            return false;
          break;
        case EngineMessage::AfterTranslucentShadow:
          if (!DoRenderPass(RenderPassType_AfterTranslucentShadow))
            return false;
          break;
        case EngineMessage::BeforeTranslucent:
          if (!DoRenderPass(RenderPassType_BeforeTranslucent))
            return false;
          break;
        case EngineMessage::AfterTranslucent:
          if (!DoRenderPass(RenderPassType_AfterTranslucent))
            return false;
          break;

        case EngineMessage::OnViewOverride: {
          float Tx, Ty, Tz, Rx, Ry, Rz, Fov;

          if (!m_PipeServer->ReadSingle(Tx))
            return false;
          if (!m_PipeServer->ReadSingle(Ty))
            return false;
          if (!m_PipeServer->ReadSingle(Tz))
            return false;
          if (!m_PipeServer->ReadSingle(Rx))
            return false;
          if (!m_PipeServer->ReadSingle(Ry))
            return false;
          if (!m_PipeServer->ReadSingle(Rz))
            return false;
          if (!m_PipeServer->ReadSingle(Fov))
            return false;

          if (m_OnViewOverride->IsValid()) {

        CefRefPtr<CefV8Value> obj =
                CefV8Value::CreateObject(nullptr, nullptr);

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
            CefRefPtr<CefV8Value> retval =
                m_OnViewOverride->ExecuteCallback(args);

            bool overriden = false;

            if (retval && retval->IsObject()) {
              CefRefPtr<CefV8Value> v8Tx = retval->GetValue("tX");
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

            if (overriden) {
              if (!m_PipeServer->WriteBoolean(true))
                return false;

              if (!m_PipeServer->WriteSingle(Tx))
                return false;
              if (!m_PipeServer->WriteSingle(Ty))
                return false;
              if (!m_PipeServer->WriteSingle(Tz))
                return false;
              if (!m_PipeServer->WriteSingle(Rx))
                return false;
              if (!m_PipeServer->WriteSingle(Ry))
                return false;
              if (!m_PipeServer->WriteSingle(Rz))
                return false;
              if (!m_PipeServer->WriteSingle(Fov))
                return false;
            } else {
              if (!m_PipeServer->WriteBoolean(false))
                return false;
            }
          } else {
            if (!m_PipeServer->WriteBoolean(false))
              return false;
          }

          if (!m_PipeServer->Flush())
            return false;  // client is waiting
        } break;

        case EngineMessage::GameEvent:
          if (!ReadGameEvent())
            return false;
          break;
      }
    }

    return true;
  }


  virtual bool OnNewConnection() override {
    // Check if our version is supported by client:

    if (!m_PipeServer->WriteInt32(Version))
      return false;

      if (!m_PipeServer->Flush())
      return false;

      bool versionSupported;
    if (!m_PipeServer->ReadBoolean(versionSupported))
      return false;

    if (!versionSupported)
      return false;

      // Supply server info required by client:

#if _WIN64
    if (!m_PipeServer->WriteBoolean(true))
      return false;
#else
    if (!m_PipeServer->WriteBoolean(false))
      return false;
#endif

    if (!WriteGameEventSettings(false))
        return false;

    if (!m_PipeServer->Flush())
      return false;

    if (m_OnNewConnection->IsValid())
      m_OnNewConnection->ExecuteCallback(CefV8ValueList());
    return true;
  }

 private:
  static CefRefPtr<CefV8Value> CreateAfxMatrix4x4(
      const struct advancedfx::interop::Matrix4x4_s& value) {
    CefRefPtr<CefV8Value> obj = CefV8Value::CreateArray(16);

    obj->SetValue(0,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M00)));
    obj->SetValue(1,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M01)));
    obj->SetValue(2,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M02)));
    obj->SetValue(3,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M03)));

    obj->SetValue(4,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M10)));
    obj->SetValue(5,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M11)));
    obj->SetValue(6,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M12)));
    obj->SetValue(7,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M13)));

    obj->SetValue(8,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M20)));
    obj->SetValue(9,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M21)));
    obj->SetValue(10,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M22)));
    obj->SetValue(11,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M23)));

    obj->SetValue(12,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M30)));
    obj->SetValue(13,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M31)));
    obj->SetValue(14,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M32)));
    obj->SetValue(15,
                  CefRefPtr<CefV8Value>(CefV8Value::CreateDouble(value.M33)));

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

    obj->SetValue("view", CreateAfxView(value.View),
                  V8_PROPERTY_ATTRIBUTE_NONE);

    obj->SetValue("frameCount", CefV8Value::CreateInt(value.FrameCount),
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("absoluteFrameTime",
                  CefV8Value::CreateDouble(value.AbsoluteFrameTime),
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("curTime", CefV8Value::CreateDouble(value.CurTime),
                  V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("frameTime", CefV8Value::CreateDouble(value.FrameTime),
                  V8_PROPERTY_ATTRIBUTE_NONE);

    return obj;
  }

  std::queue<std::string> m_Commands;

  CHandleCalcCallbacks m_HandleCalcCallbacks;
  CVecAngCalcCallbacks m_VecAngCalcCallbacks;
  CCamCalcCallbacks m_CamCalcCallbacks;
  CFovCalcCallbacks m_FovCalcCallbacks;
  CBoolCalcCallbacks m_BoolCalcCallbacks;
  CIntCalcCallbacks m_IntCalcCallbacks;

  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;
  CefRefPtr<CAfxCallback> m_OnNewConnection;
  CefRefPtr<CAfxCallback> m_OnCommands;
  CefRefPtr<CAfxCallback> m_OnRenderViewBegin;
  CefRefPtr<CAfxCallback> m_OnViewOverride;
  CefRefPtr<CAfxCallback> m_OnRenderPass;
  CefRefPtr<CAfxCallback> m_OnHudBegin;
  CefRefPtr<CAfxCallback> m_OnHudEnd;
  CefRefPtr<CAfxCallback> m_OnRenderViewEnd;
  CefRefPtr<CAfxCallback> m_OnGameEvent;


  bool DoRenderPass(enum RenderPassType_e pass) {
    View_s view;

    if (!m_PipeServer->ReadInt32(view.X))
      return false;
    if (!m_PipeServer->ReadInt32(view.Y))
      return false;
    if (!m_PipeServer->ReadInt32(view.Width))
      return false;
    if (!m_PipeServer->ReadInt32(view.Height))
      return false;

    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M00))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M01))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M02))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M03))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M10))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M11))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M12))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M13))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M20))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M21))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M22))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M23))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M30))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M31))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M32))
      return false;
    if (!m_PipeServer->ReadSingle(view.ViewMatrix.M33))
      return false;

    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M00))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M01))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M02))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M03))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M10))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M11))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M12))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M13))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M20))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M21))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M22))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M23))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M30))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M31))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M32))
      return false;
    if (!m_PipeServer->ReadSingle(view.ProjectionMatrix.M33))
      return false;

    if (m_OnRenderPass->IsValid()) {

      CefV8ValueList args;
      args.push_back(CefV8Value::CreateInt((int)pass));
      args.push_back(CreateAfxView(view));

      m_OnRenderPass->ExecuteCallback(args);
    }

    return true;
  }

  struct GameEventEnrichmentKey_s {
    std::string EventName;
    std::string EventPropertyName;

    GameEventEnrichmentKey_s(const std::string& eventName,
        const std::string& evenPropertyName) : EventName(eventName), EventPropertyName(evenPropertyName){

    }

    bool operator<(const struct GameEventEnrichmentKey_s& other) const {
      int cmp = EventName.compare(other.EventName);
      if (cmp < 0)
        return true;
      else if (0 == cmp)
         return EventPropertyName.compare(other.EventPropertyName) < 0;
      return false;
    }
  };

  std::set<std::string> m_GameEventsAllow;
  std::set<std::string> m_GameEventsAllowDeletions;
  std::set<std::string> m_GameEventsAllowAdditions;
  std::set<std::string> m_GameEventsDeny;
  std::set<std::string> m_GameEventsDenyDeletions;
  std::set<std::string> m_GameEventsDenyAdditions;
  std::map<GameEventEnrichmentKey_s, unsigned int>
      m_GameEventsEnrichments;
  std::map<GameEventEnrichmentKey_s, unsigned int>
      m_GameEventsEnrichmentsChanges;

  bool m_GameEventsTransmitChanged = false;
  bool m_GameEventsTransmitClientTime = false;
  bool m_GameEventsTransmitTick = false;
  bool m_GameEventsTransmitSystemTime = false;

  struct KnownGameEventKey_s {
    std::string Key;
    GameEventFieldType Type;

    KnownGameEventKey_s(const std::string& key, GameEventFieldType fieldType)
        : Key(key), Type(fieldType) {}
  };

  struct KnownGameEvent_s {
    std::string Name;
    std::list<KnownGameEventKey_s> Keys;

    KnownGameEvent_s() {}
  };

  std::map<int, KnownGameEvent_s> m_KnownGameEvents;

  bool ReadGameEvent() {
    int iEventId;
    if (!m_PipeServer->ReadInt32(iEventId))
      return false;

    std::map<int, KnownGameEvent_s>::iterator itKnown;
    if (0 == iEventId) {
      if (!m_PipeServer->ReadInt32(iEventId))
        return false;

      auto resultEmplace = m_KnownGameEvents.emplace(std::piecewise_construct,
                                                     std::make_tuple(iEventId),
                                                     std::make_tuple());

      if (!resultEmplace.second)
        return false;

      itKnown = resultEmplace.first;

      std::string strName;
      if (!m_PipeServer->ReadStringUTF8(itKnown->second.Name))
        return false;

      while (true) {
        bool bHasNext;
        if (!m_PipeServer->ReadBoolean(bHasNext))
          return false;
        if (!bHasNext)
          break;

        std::string strKey;
        if (!m_PipeServer->ReadStringUTF8(strKey))
          return false;

        int iEventType;
        if (!m_PipeServer->ReadInt32(iEventType))
          return false;

        itKnown->second.Keys.emplace_back(strKey,
                                          (GameEventFieldType)iEventType);
      }
    } else {
      itKnown = m_KnownGameEvents.find(iEventId);
    }

    if (itKnown == m_KnownGameEvents.end())
      return false;

    CefRefPtr<CefV8Value> objEvent =
        CefV8Value::CreateObject(nullptr, nullptr);

    objEvent->SetValue("name", CefV8Value::CreateString(itKnown->second.Name),
                        V8_PROPERTY_ATTRIBUTE_NONE);

    if (m_GameEventsTransmitClientTime) {
      float clientTime;
      if (!m_PipeServer->ReadSingle(clientTime))
        return false;
      objEvent->SetValue("clientTime", CefV8Value::CreateDouble(clientTime),
                          V8_PROPERTY_ATTRIBUTE_NONE);
    }

    if (m_GameEventsTransmitTick) {
      int tick;
      if (!m_PipeServer->ReadInt32(tick))
        return false;
      objEvent->SetValue("tick", CefV8Value::CreateInt(tick),
                          V8_PROPERTY_ATTRIBUTE_NONE);
    }

    if (m_GameEventsTransmitSystemTime) {
      uint64_t systemTime;
      if (!m_PipeServer->ReadUInt64(systemTime))
        return false;
      CefTime time((time_t)systemTime);
      objEvent->SetValue("systemTime", CefV8Value::CreateDate(time),
                          V8_PROPERTY_ATTRIBUTE_NONE);
    }

    std::string tmpString;
    float tmpFloat;
    int tmpLong;
    short tmpShort;
    unsigned char tmpByte;
    bool tmpBool;
    unsigned __int64 tmpUint64;

   CefRefPtr<CefV8Value> objKeys = CefV8Value::CreateObject(nullptr, nullptr);

    for (auto itKey = itKnown->second.Keys.begin();
         itKey != itKnown->second.Keys.end(); ++itKey) {
      CefRefPtr<CefV8Value> objKey = CefV8Value::CreateObject(nullptr, nullptr);

      objKey->SetValue("type", CefV8Value::CreateInt((int)itKey->Type),
                          V8_PROPERTY_ATTRIBUTE_NONE);

      switch (itKey->Type) {
        case GameEventFieldType::CString:
          if (!m_PipeServer->ReadStringUTF8(tmpString))
            return false;
          objKey->SetValue("value", CefV8Value::CreateString(tmpString),
                           V8_PROPERTY_ATTRIBUTE_NONE);
          break;
        case GameEventFieldType::Float:
          if (!m_PipeServer->ReadSingle(tmpFloat))
            return false;
          objKey->SetValue("value", CefV8Value::CreateDouble(tmpFloat),
                           V8_PROPERTY_ATTRIBUTE_NONE);
          break;
        case GameEventFieldType::Long:
          if (!m_PipeServer->ReadInt32(tmpLong))
            return false;
          objKey->SetValue("value", CefV8Value::CreateInt(tmpLong),
                           V8_PROPERTY_ATTRIBUTE_NONE);
          break;
        case GameEventFieldType::Short:
          if (!m_PipeServer->ReadInt16(tmpShort))
            return false;
          objKey->SetValue("value", CefV8Value::CreateInt(tmpShort),
                           V8_PROPERTY_ATTRIBUTE_NONE);
          break;
        case GameEventFieldType::Byte:
          if (!m_PipeServer->ReadByte(tmpByte))
            return false;
          objKey->SetValue("value", CefV8Value::CreateUInt(tmpByte),
                           V8_PROPERTY_ATTRIBUTE_NONE);
          break;
        case GameEventFieldType::Bool:
          if (!m_PipeServer->ReadBoolean(tmpBool))
            return false;
          objKey->SetValue("value", CefV8Value::CreateBool(tmpBool),
                           V8_PROPERTY_ATTRIBUTE_NONE);
          break;
        case GameEventFieldType::Uint64:
          if (!m_PipeServer->ReadUInt64(tmpUint64))
            return false;
          CefRefPtr<CefV8Value> objValue = CefV8Value::CreateArray(2);
          objValue->SetValue(0, CefV8Value::CreateUInt(
                                    (unsigned int)(tmpUint64 & 0x0ffffffff)));
          objValue->SetValue(1, CefV8Value::CreateUInt(
                                    (unsigned int)(tmpUint64 & 0x0ffffffff)));
          objKey->SetValue("value", objValue, V8_PROPERTY_ATTRIBUTE_NONE);
          break;
      }

      auto itEnrichment = m_GameEventsEnrichments.find(
          GameEventEnrichmentKey_s(itKnown->second.Name, itKey->Key));
      if (itEnrichment != m_GameEventsEnrichments.end()) {
        int enrichmentType = itEnrichment->second;

        CefRefPtr<CefV8Value> objEnrichments =
            CefV8Value::CreateObject(nullptr, nullptr);

        if (enrichmentType & (1 << 0)) {
          uint64_t value;
          if (!m_PipeServer->ReadUInt64(value))
            return false;

          CefRefPtr<CefV8Value> objValue = CefV8Value::CreateArray(2);
          objValue->SetValue(
              0, CefV8Value::CreateUInt((unsigned int)(value & 0x0ffffffff)));
          objValue->SetValue(1,
                             CefV8Value::CreateUInt(
                                 (unsigned int)((value >> 32) & 0x0ffffffff)));

          objEnrichments->SetValue("userIdWithSteamId", objValue,
                                   V8_PROPERTY_ATTRIBUTE_NONE);
        }

        if (enrichmentType & (1 << 1)) {
          Vector_s value;
          if (!m_PipeServer->ReadSingle(value.X))
            return false;
          if (!m_PipeServer->ReadSingle(value.Y))
            return false;
          if (!m_PipeServer->ReadSingle(value.Z))
            return false;

          CefRefPtr<CefV8Value> objValue =
              CefV8Value::CreateObject(nullptr, nullptr);
          objValue->SetValue("x", CefV8Value::CreateDouble(value.X),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("y", CefV8Value::CreateDouble(value.Y),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("z", CefV8Value::CreateDouble(value.Z),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objEnrichments->SetValue("entnumWithOrigin", objValue,
                                   V8_PROPERTY_ATTRIBUTE_NONE);
        }

        if (enrichmentType & (1 << 2)) {
          QAngle_s value;
          if (!m_PipeServer->ReadSingle(value.Pitch))
            return false;
          if (!m_PipeServer->ReadSingle(value.Yaw))
            return false;
          if (!m_PipeServer->ReadSingle(value.Roll))
            return false;
          CefRefPtr<CefV8Value> objValue =
              CefV8Value::CreateObject(nullptr, nullptr);
          objValue->SetValue("pitch", CefV8Value::CreateDouble(value.Pitch),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("yaw", CefV8Value::CreateDouble(value.Yaw),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("roll", CefV8Value::CreateDouble(value.Roll),
                             V8_PROPERTY_ATTRIBUTE_NONE);

          objEnrichments->SetValue("entnumWithAngles", objValue,
                                   V8_PROPERTY_ATTRIBUTE_NONE);
        }

        if (enrichmentType & (1 << 3)) {
          Vector_s value;
          if (!m_PipeServer->ReadSingle(value.X))
            return false;
          if (!m_PipeServer->ReadSingle(value.Y))
            return false;
          if (!m_PipeServer->ReadSingle(value.Z))
            return false;

          CefRefPtr<CefV8Value> objValue =
              CefV8Value::CreateObject(nullptr, nullptr);
          objValue->SetValue("x", CefV8Value::CreateDouble(value.X),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("y", CefV8Value::CreateDouble(value.Y),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("z", CefV8Value::CreateDouble(value.Z),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objEnrichments->SetValue("useridWithEyePosition", objValue,
                                   V8_PROPERTY_ATTRIBUTE_NONE);
        }

        if (enrichmentType & (1 << 4)) {
          QAngle_s value;
          if (!m_PipeServer->ReadSingle(value.Pitch))
            return false;
          if (!m_PipeServer->ReadSingle(value.Yaw))
            return false;
          if (!m_PipeServer->ReadSingle(value.Roll))
            return false;
          CefRefPtr<CefV8Value> objValue =
              CefV8Value::CreateObject(nullptr, nullptr);
          objValue->SetValue("pitch", CefV8Value::CreateDouble(value.Pitch),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("yaw", CefV8Value::CreateDouble(value.Yaw),
                             V8_PROPERTY_ATTRIBUTE_NONE);
          objValue->SetValue("roll", CefV8Value::CreateDouble(value.Roll),
                             V8_PROPERTY_ATTRIBUTE_NONE);

          objEnrichments->SetValue("useridWithEyeAngels", objValue,
                                   V8_PROPERTY_ATTRIBUTE_NONE);
        }

        objKey->SetValue("enrichments", objEnrichments,
                         V8_PROPERTY_ATTRIBUTE_NONE);
      }

      objKeys->SetValue(itKey->Key, objKey, V8_PROPERTY_ATTRIBUTE_NONE);
    }

    objEvent->SetValue("keys", objKeys, V8_PROPERTY_ATTRIBUTE_NONE);

    if (m_OnGameEvent->IsValid()) {
      CefV8ValueList args;
      args.push_back(objEvent);
      m_OnGameEvent->ExecuteCallback(args);
    }

    return true;
  }

  bool WriteGameEventSettings(bool delta) {
    if (!m_PipeServer->WriteBoolean(m_OnGameEvent->IsValid() ? true
                                                                   : false))
      return false;

    if (!m_OnGameEvent->IsValid())
      return true;

    if (!delta) {
      m_KnownGameEvents.clear();
    }

    if(delta) {
      bool bChanged = !(m_GameEventsAllowDeletions.empty() &&
                        m_GameEventsAllowAdditions.empty() &&
                        m_GameEventsDenyDeletions.empty() &&
                        m_GameEventsDenyAdditions.empty() &&
                        m_GameEventsEnrichmentsChanges.empty()) ||
                      m_GameEventsTransmitChanged;

      if (!m_PipeServer->WriteBoolean(bChanged))
        return false;

      if (!bChanged)
        return true;
    }

    if (!m_PipeServer->WriteBoolean(m_GameEventsTransmitClientTime))
      return false;
    if (!m_PipeServer->WriteBoolean(m_GameEventsTransmitTick))
      return false;
    if (!m_PipeServer->WriteBoolean(m_GameEventsTransmitSystemTime))
      return false;

    m_GameEventsTransmitChanged = false;

    // Allow removals:
    if (delta) {
      if (!m_PipeServer->WriteCompressedUInt32(
              (UINT32)m_GameEventsAllowDeletions.size()))
        return false;
    }
    while (!m_GameEventsAllowDeletions.empty()) {
      auto it = m_GameEventsAllowDeletions.begin();
      if (!(!delta || m_PipeServer->WriteStringUTF8(*it)))
        return false;
      m_GameEventsAllow.erase(*it);
      m_GameEventsAllowDeletions.erase(it);
    }

    // Allow additions:
    if (!m_PipeServer->WriteCompressedUInt32(
            (UINT32)m_GameEventsAllowAdditions.size() +
            (UINT32)(delta ? 0 : m_GameEventsAllow.size())))
      return false;
    if (!delta) {
      for (auto it = m_GameEventsAllow.begin(); it != m_GameEventsAllow.end(); ++it) {
        if (!m_PipeServer->WriteStringUTF8(*it))
          return false;
      }
    }
    while (!m_GameEventsAllowAdditions.empty()) {
      auto it = m_GameEventsAllowAdditions.begin();
      if (!m_PipeServer->WriteStringUTF8(*it))
        return false;
      m_GameEventsAllow.insert(*it);
      m_GameEventsAllowAdditions.erase(it);
    }

   // Deny removals:
    if (delta) {
      if (!m_PipeServer->WriteCompressedUInt32(
              (UINT32)m_GameEventsDenyDeletions.size()))
        return false;
    }
    while (!m_GameEventsDenyDeletions.empty()) {
      auto it = m_GameEventsDenyDeletions.begin();
      if (!(!delta || m_PipeServer->WriteStringUTF8(*it)))
        return false;
      m_GameEventsDeny.erase(*it);
      m_GameEventsDenyDeletions.erase(it);
    }

    // Deny additions:
    if (!m_PipeServer->WriteCompressedUInt32(
            (UINT32)(m_GameEventsDenyAdditions.size() +
            (UINT32)(delta ? 0 : m_GameEventsDeny.size()))))
      return false;
    if (!delta) {
      for (auto it = m_GameEventsDeny.begin();
           it != m_GameEventsDeny.end(); ++it) {
        if (!m_PipeServer->WriteStringUTF8(*it))
          return false;
      }
    }
    while (!m_GameEventsDenyAdditions.empty()) {
      auto it = m_GameEventsDenyAdditions.begin();
      if (!m_PipeServer->WriteStringUTF8(*it))
        return false;
      m_GameEventsDeny.insert(*it);
      m_GameEventsDenyAdditions.erase(it);
    }

    // Write enrichments:
    if (!m_PipeServer->WriteCompressedUInt32(
            (UINT32)(m_GameEventsEnrichmentsChanges.size() +
                     (delta ? 0 : m_GameEventsEnrichments.size()))))
      return false;
    if (!delta) {
      for (auto itEnrichment = m_GameEventsEnrichments.begin();
           itEnrichment != m_GameEventsEnrichments.end(); ++itEnrichment) {
        if (!m_PipeServer->WriteStringUTF8(itEnrichment->first.EventName.c_str()))
          return false;
        if (!m_PipeServer->WriteStringUTF8(itEnrichment->first.EventPropertyName.c_str()))
          return false;
        if (!m_PipeServer->WriteUInt32(itEnrichment->second))
          return false;
      }
    }
    while (!m_GameEventsEnrichmentsChanges.empty()) {
      auto itEnrichment = m_GameEventsEnrichmentsChanges.begin();

      unsigned int enrichmentType = itEnrichment->second;

      if (!m_PipeServer->WriteStringUTF8(itEnrichment->first.EventName.c_str()))
        return false;
      if (!m_PipeServer->WriteStringUTF8(
              itEnrichment->first.EventPropertyName.c_str()))
        return false;
      if (!m_PipeServer->WriteUInt32(itEnrichment->second))
        return false;

      if (0 == enrichmentType) {
        auto targetEnrichment =
            m_GameEventsEnrichments.find(itEnrichment->first);
        if (targetEnrichment != m_GameEventsEnrichments.end())
          m_GameEventsEnrichments.erase(targetEnrichment);
      } else {
        m_GameEventsEnrichments[itEnrichment->first] = enrichmentType;
      }

      m_GameEventsEnrichmentsChanges.erase(itEnrichment);
    }

    return true;
  }

  class CCalcResult : public CefV8Accessor, public CefV8Handler {
    IMPLEMENT_REFCOUNTING(CCalcResult);

   public:
       static CefRefPtr<CefV8Value> Create(CCalcCallbacksGuts* guts,
           const char* name,
           CefRefPtr<CAfxCallback> callback) {

         CefRefPtr<CCalcResult> self = new CCalcResult(guts, name, callback);

         auto obj = CefV8Value::CreateObject(self, nullptr);
         obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                         V8_PROPERTY_ATTRIBUTE_NONE);

         return obj;
    }

 
    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
      if (name == "release") {
        retval = CefV8Value::CreateBool(ReleaseCalc());
        return true;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }

      return false;
    }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      return false;
    }

   private:
    CCalcCallbacksGuts* m_Guts;
    std::string m_Name;
    CefRefPtr<CAfxCallback> m_Callback;

   CCalcResult(CCalcCallbacksGuts* guts,
                const char* name,
                CefRefPtr<CAfxCallback> callback)
        : m_Guts(guts), m_Name(name), m_Callback(callback) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);

      m_Guts->Add(name, callback);
    }

    virtual ~CCalcResult() { ReleaseCalc(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    bool m_ReleasedCalc = false;

    bool ReleaseCalc() {
      if (m_ReleasedCalc)
        return false;

      m_ReleasedCalc = true;
      m_Guts->Remove(m_Name.c_str(), m_Callback.get());
      m_Callback = nullptr;

      return true;
    }
  };
};

CefRefPtr<CefV8Value> CreateEngineInterop(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefV8Context> context,
                                   const CefString& argStr,
                                   CefRefPtr<CInterop>* out) {
  return CEngineInteropImpl::Create(browser,frame,context, argStr);
}


class CInteropImpl : public CInterop {
  IMPLEMENT_REFCOUNTING(CInteropImpl);

 public:
  static CefRefPtr<CefV8Value> Create(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context,
                                      const CefString& argStr,
                                      CefRefPtr<CInterop>* out = nullptr) {
    CefRefPtr<CInteropImpl> self = new CInteropImpl();

    CefRefPtr<CAfxObject> afxObject = new CAfxObject();

    //

    afxObject->AddGetter(
        "id",
        [browser](const CefString& name, const CefRefPtr<CefV8Value> object,
                  CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateInt(browser->GetIdentifier());

          return true;
        });

    afxObject->AddFunction("init",
        [context, obj = afxObject->GetObj(), argStr](
                  const CefString& name, CefRefPtr<CefV8Value> object,
                  const CefV8ValueList& arguments,
                  CefRefPtr<CefV8Value>& retval, CefString& exceptionoverride) {
                if (1 == arguments.size()) {
                  auto argInitFn = arguments[0];

                  if (argInitFn->IsFunction()) {
                    CefV8ValueList args;
                    args.push_back(obj);
                    args.push_back(CefV8Value::CreateString(argStr));

                    argInitFn->ExecuteFunctionWithContext(context, NULL, args);

                    return true;
                  }
                }

               exceptionoverride = g_szInvalidArguments;
          return true;

              });

    afxObject->AddFunction(
        "createDrawingInterop",
              [frame,context](
                  const CefString& name, CefRefPtr<CefV8Value> object,
                  const CefV8ValueList& arguments,
                  CefRefPtr<CefV8Value>& retval, CefString& exceptionoverride) {
                if (4 == arguments.size()) {
                  auto argUrl = arguments[0];
                  auto argStr = arguments[1];
                  auto argFnResolve = arguments[2];
                  auto argFnReject = arguments[3];

                  if (argUrl->IsString() && argStr->IsString() &&
                      argFnResolve->IsFunction() && argFnReject->IsFunction()) {
                    CCreateInterop* createInterop =
                        new CCreateInterop(context, argFnResolve, argFnReject);

                    auto message =
                        CefProcessMessage::Create("afx-create-drawing");
                    auto args = message->GetArgumentList();
                    args->SetSize(4);
                    args->SetInt(0, (int)createInterop->GetLo());
                    args->SetInt(1, (int)createInterop->GetHi());
                    args->SetString(2, argUrl->GetStringValue());
                    args->SetString(3, argStr->GetStringValue());

                    frame->SendProcessMessage(PID_BROWSER, message);
                    return true;
                  }
                }

                exceptionoverride = g_szInvalidArguments;
                return true;
              });

    afxObject->AddFunction(
        "createEngineInterop",
              [frame, context](
                  const CefString& name, CefRefPtr<CefV8Value> object,
                  const CefV8ValueList& arguments,
                  CefRefPtr<CefV8Value>& retval, CefString& exceptionoverride) {
                if (4 == arguments.size()) {
                  auto argUrl = arguments[0];
                  auto argStr = arguments[1];
                  auto argFnResolve = arguments[2];
                  auto argFnReject = arguments[3];

                  if (argUrl->IsString() && argStr->IsString() &&
                      argFnResolve->IsFunction() && argFnReject->IsFunction()) {
                    CCreateInterop* createInterop =
                        new CCreateInterop(context, argFnResolve, argFnReject);

                    auto message =
                        CefProcessMessage::Create("afx-create-engine");
                    auto args = message->GetArgumentList();
                    args->SetSize(4);
                    args->SetInt(0, (int)createInterop->GetLo());
                    args->SetInt(1, (int)createInterop->GetHi());
                    args->SetString(2, argUrl->GetStringValue());
                    args->SetString(3, argStr->GetStringValue());

                    frame->SendProcessMessage(PID_BROWSER, message);
                    return true;
                  }
                }

                exceptionoverride = g_szInvalidArguments;
                return true;
              });

    afxObject->AddFunction("sendMessage",
              [frame](
                  const CefString& name, CefRefPtr<CefV8Value> object,
                  const CefV8ValueList& arguments,
                  CefRefPtr<CefV8Value>& retval, CefString& exceptionoverride) {
                if (arguments.size() == 2) {
                  auto argId = arguments[0];
                  auto argStr = arguments[1];

                  if (argId->IsInt() && argStr->IsString()) {
                    auto message =
                        CefProcessMessage::Create("afx-send-message");
                    auto args = message->GetArgumentList();
                    args->SetSize(2);
                    args->SetInt(0, argId->GetIntValue());
                    args->SetString(1, argStr->GetStringValue());

                    frame->SendProcessMessage(PID_BROWSER, message);
                    return true;
                  }
                }

                exceptionoverride = g_szInvalidArguments;
                return true;
              });

    self->m_OnMessage = afxObject->AddCallback("onMessage", context);

    self->m_OnError = afxObject->AddCallback("onError", context);

    //

    if (out)
      *out = self;

    return afxObject->GetObj();
  }

  CInteropImpl() {

  }

private:
  void ProcessAfxMessage(int senderId, const CefString& message) {
    CefV8ValueList execArgs;
    execArgs.push_back(CefV8Value::CreateInt(senderId));
    execArgs.push_back(CefV8Value::CreateString(message));

    if (m_OnMessage->IsValid())
      m_OnMessage->ExecuteCallback(execArgs);
  }
  void ProcessAfxError(int senderId, const CefString& message) {
    CefV8ValueList execArgs;
    execArgs.push_back(CefV8Value::CreateInt(senderId));
    execArgs.push_back(CefV8Value::CreateString(message));

    if (m_OnError->IsValid())
      m_OnError->ExecuteCallback(execArgs);
  }

  void ProcessAfxResolve(unsigned int lo, unsigned int hi, int interopId) {
    CCreateInterop::Resolve(lo, hi, interopId);
  }

  void ProcessAfxReject(unsigned int lo, unsigned int hi, const CefString & message) {
    CCreateInterop::Reject(lo, hi, message);
  }


 public:

  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override {

    auto const name = message->GetName();

    if (name == "afx-message") {
      auto const args = message->GetArgumentList();

      CefPostTask(TID_RENDERER,
                  base::Bind(&CInteropImpl::ProcessAfxMessage, this, args->GetInt(0),
                             args->GetString(1)));

      return true;

    } else if (name == "afx-error") {
      auto const args = message->GetArgumentList();

      CefPostTask(TID_RENDERER,
                  base::Bind(&CInteropImpl::ProcessAfxError, this,
                             args->GetInt(0),
                             args->GetString(1)));

      return true;
    } else if (name == "afx-interop-resolve") {
      auto const args = message->GetArgumentList();
      

      CefPostTask(TID_RENDERER, base::Bind(&CInteropImpl::ProcessAfxResolve,
                                           this, (unsigned int)args->GetInt(0),
                                           (unsigned int)args->GetInt(1),
                                           args->GetInt(2)));

      return true;
    } else if (name == "afx-interop-reject") {
      auto const args = message->GetArgumentList();

      CefPostTask(TID_RENDERER, base::Bind(&CInteropImpl::ProcessAfxReject,
                                           this, (unsigned int)args->GetInt(0),
                                           (unsigned int)args->GetInt(1),
                                           args->GetString(2)));

      return true;
    }

    return false;
  }

  virtual void CloseInterop() override {}


 private:
  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;

  virtual ~CInteropImpl() { CCreateInterop::FreeAll(); }
};

CefRefPtr<CefV8Value> CreateInterop(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context,
    const CefString& argStr,
    CefRefPtr<CInterop>* out) {
  return CInteropImpl::Create(browser, frame, context, argStr, out);
}

}  // namespace interop
}  // namespace advancedfx
