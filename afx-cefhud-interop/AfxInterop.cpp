#include "AfxInterop.h"

#include <include/cef_base.h>
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

enum class CefEngineInteropMessage : int {
  DeviceReset
};

enum class CefDrawingInteropMessage : int {
  SetPipeName,
  Close,

  Connect,
  PumpBegin,
  PumpEnd,
  BeginFrame,
	
  D3d9CreateVertexDecalaration,
	
  D3d9CreateIndexBuffer,
  D3d9CreateVertexBuffer,
  D3d9CreateTexture,
	
  D3d9CreateVertexShader,
  D3d9CreatePixelShader,
	
  D3d9UpdateTexture,
  D3d9SetViewport,
  D3d9SetRenderState,
  D3d9SetSamplerState,
  D3d9SetTexture,
  D3d9SetTextureStageState,
  D3d9SetTransform,
  D3d9SetIndices,
  D3d9SetStreamSource,
  D3d9SetStreamSourceFreq,
  D3d9SetVertexDeclaration,
  D3d9SetVertexShader,
  D3d9SetVertexShaderConstantB,
  D3d9SetVertexShaderConstantF,
  D3d9SetVertexShaderConstantI,
  D3d9SetPixelShader,
  D3d9SetPixelShaderConstantB,
  D3d9SetPixelShaderConstantF,
  D3d9SetPixelShaderConstantI,
  D3d9DrawPrimitive,
  D3d9DrawIndexedPrimitive,
	
  WaitForGpu,
  BeginCleanState,
  EndCleanState,

  CreateCefWindowTextureSharedHandle,

  CreateDrawingData,

  AfxDrawingHandle_Release,

  AfxDrawingData_Release,
  AfxDrawingData_Update,

  AfxD3d9VertexDeclaration_Release,

  AfxD3d9IndexBuffer_Release,
  AfxD3d9IndexBuffer_Update,

  AfxD3d9VertexBuffer_Release,
  AfxD3d9VertexBuffer_Update,

  AfxD3d9Texture_Release,
  AfxD3d9Texture_Update,

  AfxD3d9PixelShader_Release,
  AfxD3d9VertexShader_Release
};

enum class AfxUserDataType : int {
  AfxDrawingHandle,
  AfxDrawingData,
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

class CCalcCallback : public CefBaseRefCounted
{
  IMPLEMENT_REFCOUNTING(CCalcCallback);

public:
  CCalcCallback(const CefRefPtr<CefV8Value>& callbackFunc,
                const CefRefPtr<CefV8Context>& callbackContext)
      : m_CallbackFunc(callbackFunc), m_CallbackContext(callbackContext){
  }

  CefRefPtr<CefV8Value> Call(const CefV8ValueList arguments) {
    return m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext, NULL,
                                               arguments);
  }

private:
  CefRefPtr<CefV8Value> m_CallbackFunc;
  CefRefPtr<CefV8Context> m_CallbackContext;
};

class CCalcCallbacksGuts {
public:
  ~CCalcCallbacksGuts() {
    while (!m_Map.empty()) {
      while (!m_Map.begin()->second.empty()) {
        CCalcCallback* callback = (*m_Map.begin()->second.begin());
        callback->Release();
        m_Map.begin()->second.erase(m_Map.begin()->second.begin());
      }
      m_Map.erase(m_Map.begin());
    }
  }

  void Add(const char* name, CCalcCallback* callback) {
    callback->AddRef();
    m_Map[name].emplace(callback);
  }

  void Remove(const char* name, CCalcCallback* callback) {
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
  typename typedef std::set<CCalcCallback*> Callbacks_t;
  typedef std::map<std::string, Callbacks_t> NameToCallbacks_t;
  NameToCallbacks_t m_Map;
};

template <typename R>
class CCalcCallbacks abstract : public CCalcCallbacksGuts {
 public:
  bool BatchUpdateRequest(CNamedPipeServer* pipeServer) {
    if (!pipeServer->WriteCompressedUInt32((UINT32)m_Map.size()))
      return false;
    for (typename std::map<std::string, std::set<CCalcCallback*>>::iterator it =
             m_Map.begin();
         it != m_Map.end(); ++it) {
      if (!pipeServer->WriteStringUTF8(it->first))
        return false;
    }

    return true;
  }

  bool BatchUpdateResult(CNamedPipeServer* pipeServer) {
    R result;

    for (typename std::map<std::string, std::set<CCalcCallback*>>::iterator it =
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

      for (typename std::set<CCalcCallback*>::iterator setIt =
               (*it).second.begin();
           setIt != (*it).second.end(); ++setIt) {
        CallResult(*setIt, resultPtr);
      }
    }

    return true;
  }

 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer, R& outResult) = 0;
  virtual void CallResult(CCalcCallback* callback, R* result) = 0;
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

  virtual void CallResult(CCalcCallback* callback,
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

    callback->Call(args);
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

  virtual void CallResult(CCalcCallback* callback,
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

    callback->Call(args);
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

  virtual void CallResult(CCalcCallback* callback,
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

    callback->Call(args);
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

  virtual void CallResult(CCalcCallback* callback,
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

    callback->Call(args);
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

  virtual void CallResult(CCalcCallback* callback,
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

    callback->Call(args);
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

  virtual void CallResult(CCalcCallback* callback,
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

    callback->Call(args);
  }
};

class CInterop {
 public:
  const INT32 Version = 7;

  CInterop(const char* pipeName) : m_PipeName(pipeName) {}

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

  virtual ~CInterop() { Close(); }

  virtual bool OnNewConnection() { return true; }

  virtual bool OnConnection() { return true; }

  virtual void OnClosing() {}

 private:
  bool m_Connecting = false;
};

class CDrawingInteropImpl : public CDrawingInterop, public CInterop {
  IMPLEMENT_REFCOUNTING(CDrawingInteropImpl);

 public:

  virtual void CloseInterop() override { 
      m_DrawingThreadedQueue.Abort();
      CInterop::Close();
  }

  CDrawingInteropImpl(CefRefPtr<CefBrowser> const& browser)
      : CInterop("advancedfxInterop_drawing"), m_Browser(browser) {
  
  }
  
  virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message) override {

    auto const name = message->GetName().ToString();

    if (name == "afx-interop") {
      auto const args = message->GetArgumentList();
      auto const argC = args->GetSize();

      if (1 <= argC) {
        int msg_id = args->GetInt(0);

        switch ((CefDrawingInteropMessage)msg_id) {
          case CefDrawingInteropMessage::SetPipeName:
            m_DrawingThreadedQueue.Queue(
                [this, pipeName = args->GetString(1).ToString()] {
                  CInterop::SetPipeName(pipeName.c_str());
                });
            return true;
          case CefDrawingInteropMessage::Close:
            m_DrawingThreadedQueue.Queue([this] { CInterop::Close(); });
            return true;
          case CefDrawingInteropMessage::Connect:
            m_DrawingThreadedQueue.Queue([this] { CInterop::Connection(); });
            return true;
          case CefDrawingInteropMessage::PumpBegin:
            m_DrawingThreadedQueue.Queue(
                [this, frameCount = args->GetInt(1),
                 pass = (unsigned int)args->GetInt(2)] {
              if (!Connected())
                return;

              if (!DoThePumping(frameCount, pass))
                Close();
            });
            return true;
          case CefDrawingInteropMessage::PumpEnd:
            m_DrawingThreadedQueue.Queue([this] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32((UINT32)DrawingReply::Finished))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::BeginFrame:
            if (1 == argC) {
              m_DrawingThreadedQueue.Queue(
                  [this] {
                       m_Browser->GetHost()->SendExternalBeginFrame();
                  });
            } else {
              m_DrawingThreadedQueue.Queue(
                  [this, width = args->GetInt(1), height = args->GetInt(2)] {
                if (width != m_Width || height != m_Height) {
                  m_Width = width;
                  m_Height = height;

                  m_Browser->GetHost()->WasResized();
                }
                m_Browser->GetHost()->SendExternalBeginFrame();
              });
            }
            return true;
          case CefDrawingInteropMessage::D3d9CreateVertexDecalaration:
            m_DrawingThreadedQueue.Queue(
                [this,
                 index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 drawingDataIndex = (unsigned __int64)args->GetInt(3) |
                                    ((unsigned __int64)args->GetInt(4) << 32)] {
              if (!m_InFlow)
                return;

              auto it = m_DrawingData.find(drawingDataIndex);

              if (m_DrawingData.end() == it)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreateVertexDeclaration))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)index))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)it->second.GetSize()))
                goto error;
              if (!m_PipeServer->WriteBytes(it->second.GetData(), 0,
                                            it->second.GetSize()))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9CreateIndexBuffer: {
            m_DrawingThreadedQueue.Queue(
                [this,
                 index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 length = (unsigned int)args->GetInt(3),
                 usage = (unsigned int)args->GetInt(4),
                 format = (unsigned int)args->GetInt(5),
                 pool = (unsigned int)args->GetInt(6),
                 sharedHandleIndex =
                     (unsigned __int64)args->GetInt(7) |
                     ((unsigned __int64)args->GetInt(8) << 32)] {
                  if (!m_InFlow)
                    return;

                  if (!m_PipeServer->WriteUInt32(
                          (UINT32)DrawingReply::D3d9CreateIndexBuffer))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT64)index))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)length))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)usage))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)format))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)pool))
                    goto error;
                  if (0 == sharedHandleIndex) {
                    if (!m_PipeServer->WriteBoolean(false))
                      goto error;
                  } else {
                    if (!m_PipeServer->WriteBoolean(true))
                      goto error;
                    if (!m_PipeServer->WriteHandle(m_ShareHandle))
                      goto error;
                  }

                  return;

                error:
                  Close();
                });
          }
            return true;
          case CefDrawingInteropMessage::D3d9CreateVertexBuffer:
            m_DrawingThreadedQueue.Queue(
                [this,
                 index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 length = (unsigned int)args->GetInt(3),
                 usage = (unsigned int)args->GetInt(4),
                 fvf = (unsigned int)args->GetInt(5),
                 pool = (unsigned int)args->GetInt(6),
                 sharedHandleIndex =
                     (unsigned __int64)args->GetInt(7) |
                     ((unsigned __int64)args->GetInt(8) << 32)] {
                  if (!m_InFlow)
                    return;

                  if (!m_PipeServer->WriteUInt32(
                          (UINT32)DrawingReply::D3d9CreateVertexBuffer))
                    goto error;
                  if (!m_PipeServer->WriteUInt64((UINT64)index))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)length))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)usage))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)fvf))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)pool))
                    goto error;
                  if (0 == sharedHandleIndex) {
                    if (!m_PipeServer->WriteBoolean(false))
                      goto error;
                  } else {
                    if (!m_PipeServer->WriteBoolean(true))
                      goto error;
                    if (!m_PipeServer->WriteHandle(m_ShareHandle))
                      goto error;
                  }

                  return;

                error:
                  Close();
                });
            return true;
          case CefDrawingInteropMessage::D3d9CreateTexture:
            m_DrawingThreadedQueue.Queue(
                [this,
                 index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 width = (unsigned int)args->GetInt(3),
                 height = (unsigned int)args->GetInt(4),
                 levels = (unsigned int)args->GetInt(5),
                 usage = (unsigned int)args->GetInt(6),
                 format = (unsigned int)args->GetInt(7),
                 pool = (unsigned int)args->GetInt(8),
                 sharedHandleIndex =
                     (unsigned __int64)args->GetInt(9) |
                     ((unsigned __int64)args->GetInt(10) << 32)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreateTexture))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)index))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)width))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)height))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)levels))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)usage))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)format))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)pool))
                goto error;
              if (0 == sharedHandleIndex) {
                if (!m_PipeServer->WriteBoolean(false))
                  goto error;
              } else {
                if (!m_PipeServer->WriteBoolean(true))
                  goto error;
                if (!m_PipeServer->WriteHandle(m_ShareHandle))
                  goto error;
              }

              return;

            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9CreateVertexShader:
            m_DrawingThreadedQueue.Queue(
                [this,
                 index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 drawingDataIndex = (unsigned __int64)args->GetInt(3) |
                                    ((unsigned __int64)args->GetInt(4) << 32)] {
                  if (!m_InFlow)
                    return;

                  auto it = m_DrawingData.find(drawingDataIndex);

                  if (m_DrawingData.end() == it)
                    return;

                  if (!m_PipeServer->WriteUInt32(
                          (UINT32)DrawingReply::D3d9CreateVertexShader))
                    goto error;
                  if (!m_PipeServer->WriteUInt64((UINT64)index))
                    goto error;
                  if (!m_PipeServer->WriteUInt32((UINT32)it->second.GetSize()))
                    goto error;
                  if (!m_PipeServer->WriteBytes(it->second.GetData(), 0,
                                                it->second.GetSize()))
                    goto error;

                  return;
                error:
                  Close();
                });
            return true;
          case CefDrawingInteropMessage::D3d9CreatePixelShader:
            m_DrawingThreadedQueue.Queue(
                [this,
                 index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 drawingDataIndex = (unsigned __int64)args->GetInt(3) |
                                    ((unsigned __int64)args->GetInt(4) << 32)] {
              if (!m_InFlow)
                return;

              auto it = m_DrawingData.find(drawingDataIndex);

              if (m_DrawingData.end() == it)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreatePixelShader))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)index))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)it->second.GetSize()))
                goto error;
              if (!m_PipeServer->WriteBytes(it->second.GetData(), 0,
                                            it->second.GetSize()))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9UpdateTexture:
            m_DrawingThreadedQueue.Queue(
                [this,
                 sourceTextureIndex = (unsigned __int64)args->GetInt(1) |
                                      ((unsigned __int64)args->GetInt(2) << 32),
                 destinationTextureIndex =
                     (unsigned __int64)args->GetInt(3) |
                     ((unsigned __int64)args->GetInt(4) << 32)] {
                  if (!m_InFlow)
                    return;

                  if (!m_PipeServer->WriteUInt32(
                          (UINT32)DrawingReply::D3d9UpdateTexture))
                    goto error;
                  if (!m_PipeServer->WriteUInt64((UINT64)sourceTextureIndex))
                    goto error;
                  if (!m_PipeServer->WriteUInt64(
                          (UINT64)destinationTextureIndex))
                    goto error;

                  return;
                error:
                  Close();
                });
            return true;
          case CefDrawingInteropMessage::D3d9SetViewport:
            if (1 == argC) {
              m_DrawingThreadedQueue.Queue([this] {
                if (!m_InFlow)
                  return;

                if (!m_PipeServer->WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetViewport))
                  goto error;
                if (!m_PipeServer->WriteBoolean(false))
                  goto error;

                return;
              error:
                Close();
              });
            } else {
              m_DrawingThreadedQueue.Queue(
                  [this, x = (unsigned int)args->GetInt(1),
                   y = (unsigned int)args->GetInt(2),
                   width = (unsigned int)args->GetInt(3),
                   height = (unsigned int)args->GetInt(4),
                   minZ = (float)args->GetDouble(5),
                   maxZ = (float)args->GetDouble(6)] {
                    if (!m_InFlow)
                      return;

                    if (!m_PipeServer->WriteUInt32(
                            (UINT32)DrawingReply::D3d9SetViewport))
                      goto error;
                    if (!m_PipeServer->WriteBoolean(true))
                      goto error;
                    if (!m_PipeServer->WriteUInt32(x))
                      goto error;
                    if (!m_PipeServer->WriteUInt32(y))
                      goto error;
                    if (!m_PipeServer->WriteUInt32(width))
                      goto error;
                    if (!m_PipeServer->WriteUInt32(height))
                      goto error;
                    if (!m_PipeServer->WriteSingle(minZ))
                      goto error;
                    if (!m_PipeServer->WriteSingle(maxZ))
                      goto error;

                    return;
                  error:
                    Close();
                  });
            }
            return true;
          case CefDrawingInteropMessage::D3d9SetRenderState:
            m_DrawingThreadedQueue.Queue([this,
                                          state = (unsigned int)args->GetInt(1),
                                          value =
                                              (unsigned int)args->GetInt(2)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetRenderState))
                goto error;
              if (!m_PipeServer->WriteUInt32(state))
                goto error;
              if (!m_PipeServer->WriteUInt32(value))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetSamplerState:
            m_DrawingThreadedQueue.Queue(
                [this, sampler = (unsigned int)args->GetInt(1),
                 type = (unsigned int)args->GetInt(2),
                 value = (unsigned int)args->GetInt(3)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetSamplerState))
                goto error;
              if (!m_PipeServer->WriteUInt32(sampler))
                goto error;
              if (!m_PipeServer->WriteUInt32(type))
                goto error;
              if (!m_PipeServer->WriteUInt32(value))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetTexture:
            m_DrawingThreadedQueue.Queue(
                [this, sampler = (unsigned int)args->GetInt(1),
                 textureIndex = (unsigned __int64)args->GetInt(2) |
                                ((unsigned __int64)args->GetInt(3) << 32)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetTexture))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)sampler))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)textureIndex))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetTextureStageState:
            m_DrawingThreadedQueue.Queue([this,
                                          stage = (unsigned int)args->GetInt(1),
                                          type = (unsigned int)args->GetInt(2),
                                          value =
                                              (unsigned int)args->GetInt(3)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetTextureStageState))
                goto error;
              if (!m_PipeServer->WriteUInt32(stage))
                goto error;
              if (!m_PipeServer->WriteUInt32(type))
                goto error;
              if (!m_PipeServer->WriteUInt32(value))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetTransform:
            if (2 == argC) {
              m_DrawingThreadedQueue.Queue([this,
                                            state =
                                                (unsigned int)args->GetInt(1)] {
                if (!m_InFlow)
                  return;

                if (!m_PipeServer->WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetTransform))
                  goto error;
                if (!m_PipeServer->WriteUInt32((UINT32)state))
                  goto error;
                if (!m_PipeServer->WriteBoolean(false))
                  goto error;

                return;
              error:
                Close();
              });
            } else {
              m_DrawingThreadedQueue.Queue([this,
                                            state =
                                                (unsigned int)args->GetInt(1),
                   matrix =std::move(FloatVecFromCefArgs(args,2,16))] {
                if (!m_InFlow)
                  return;

                if (!m_PipeServer->WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetTransform))
                  goto error;
                if (!m_PipeServer->WriteUInt32((UINT32)state))
                  goto error;
                if (!m_PipeServer->WriteBoolean(true))
                  goto error;

                for (size_t i = 0; i < matrix.size(); ++i) {
                  if (!m_PipeServer->WriteSingle(matrix[i]))
                    goto error;
                }

                return;
              error:
                Close();
              });
            }
            return true;
          case CefDrawingInteropMessage::D3d9SetIndices:
            m_DrawingThreadedQueue.Queue([this, indicesIndex =
                (unsigned __int64)args->GetInt(1) |
                ((unsigned __int64)args->GetInt(2) << 32)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetIndices))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)indicesIndex))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetStreamSource:
            m_DrawingThreadedQueue.Queue(
                [this, streamNumber = (unsigned int)args->GetInt(1),
                 streamDataIndex = (unsigned __int64)args->GetInt(2) |
                                   ((unsigned __int64)args->GetInt(3) << 32),
                 offsetInBytes = (unsigned int)args->GetInt(4),
                 stride = (unsigned int)args->GetInt(5)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetStreamSource))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT64)streamNumber))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)streamDataIndex))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT64)offsetInBytes))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT64)stride))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetStreamSourceFreq:
            m_DrawingThreadedQueue.Queue(
                [this, streamNumber = (unsigned int)args->GetInt(1),
                 setting = (unsigned int)args->GetInt(2)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetStreamSourceFreq))
                goto error;
              if (!m_PipeServer->WriteUInt32(streamNumber))
                goto error;
              if (!m_PipeServer->WriteUInt32(setting))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetVertexDeclaration:
            m_DrawingThreadedQueue.Queue(
                [this, declIndex = (unsigned __int64)args->GetInt(1) |
                                   ((unsigned __int64)args->GetInt(2) << 32)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexDeclaration))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)declIndex))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetVertexShader:
            m_DrawingThreadedQueue.Queue(
                [this,
                 shaderIndex = (unsigned __int64)args->GetInt(1) |
                               ((unsigned __int64)args->GetInt(2) << 32)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShader))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)shaderIndex))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetVertexShaderConstantB:
            m_DrawingThreadedQueue.Queue([this,
                                          startRegister =
                                              (unsigned int)args->GetInt(1),
                                          constantData = std::move(BoolVecFromCefArgs(args, 2, args->GetSize() - 2))] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShaderConstantB))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)constantData.size()))
                goto error;
              for (size_t i = 0; i < constantData.size(); ++i) {
                if (!m_PipeServer->WriteBoolean(constantData[i]))
                  goto error;
              }

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetVertexShaderConstantF:
            m_DrawingThreadedQueue.Queue(
                [this, startRegister = (unsigned int)args->GetInt(1),
                                          constantData = std::move(FloatVecFromCefArgs(
                                              args, 2, args->GetSize() - 2))] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShaderConstantF))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)constantData.size()))
                goto error;
              for (size_t i = 0; i < constantData.size(); ++i) {
                if (!m_PipeServer->WriteSingle((FLOAT)constantData[i]))
                  goto error;
              }

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetVertexShaderConstantI:
            m_DrawingThreadedQueue.Queue(
                [this, startRegister = (unsigned int)args->GetInt(1),
                 constantData = std::move(IntVecFromCefArgs(
                                              args, 2, args->GetSize() - 2))] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShaderConstantF))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)constantData.size()))
                goto error;
              for (size_t i = 0; i < constantData.size(); ++i) {
                if (!m_PipeServer->WriteInt32((INT32)constantData[i]))
                  goto error;
              }

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetPixelShader: {
            unsigned __int64 shaderIndex =
                (unsigned __int64)args->GetInt(1) |
                ((unsigned __int64)args->GetInt(2) << 32);
            m_DrawingThreadedQueue.Queue([this, shaderIndex] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShader))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)shaderIndex))
                goto error;

              return;
            error:
              Close();
            });
          }
            return true;
          case CefDrawingInteropMessage::D3d9SetPixelShaderConstantB:
            m_DrawingThreadedQueue.Queue([this,startRegister =
                                              (unsigned int)args->GetInt(1),
                                          constantData = std::move(BoolVecFromCefArgs(
                                              args, 2, args->GetSize() - 2))] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShaderConstantB))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)constantData.size()))
                goto error;
              for (size_t i = 0; i < constantData.size(); ++i) {
                if (!m_PipeServer->WriteBoolean(constantData[i]))
                  goto error;
              }

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetPixelShaderConstantF:
            m_DrawingThreadedQueue.Queue([this, startRegister = (unsigned int)args->GetInt(1),
                 constantData = std::move(
                     FloatVecFromCefArgs(args, 2, args->GetSize() - 2))] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShaderConstantF))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)constantData.size()))
                goto error;
              for (size_t i = 0; i < constantData.size(); ++i) {
                if (!m_PipeServer->WriteSingle((FLOAT)constantData[i]))
                  goto error;
              }

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9SetPixelShaderConstantI:
            m_DrawingThreadedQueue.Queue([this,startRegister = (unsigned int)args->GetInt(1),
                 constantData = std::move(
                     IntVecFromCefArgs(args, 2, args->GetSize() - 2))] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShaderConstantF))
                goto error;
              if (!m_PipeServer->WriteUInt32((UINT32)constantData.size()))
                goto error;
              for (size_t i = 0; i < constantData.size(); ++i) {
                if (!m_PipeServer->WriteInt32((INT32)constantData[i]))
                  goto error;
              }

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9DrawPrimitive:
            m_DrawingThreadedQueue.Queue(
            [this, primitiveType = (unsigned int)args->GetInt(1), startVertex = (unsigned int)args->GetInt(2),  primitiveCount = (unsigned int)args->GetInt(3)] {
                if (!m_InFlow)
                return;

                if (!m_PipeServer->WriteUInt32(
                        (UINT32)DrawingReply::D3d9DrawPrimitive))
                goto error;
                if (!m_PipeServer->WriteUInt32(primitiveType))
                goto error;
                if (!m_PipeServer->WriteUInt32(startVertex))
                goto error;
                if (!m_PipeServer->WriteUInt32(primitiveCount))
                goto error;

                return;
            error:
                Close();
            });
            return true;
          case CefDrawingInteropMessage::D3d9DrawIndexedPrimitive:
            m_DrawingThreadedQueue.Queue([this, primitiveType = (unsigned int)args->GetInt(1), baseVertexIndex = (unsigned int)args->GetInt(2),
                                          minVertexIndex = (unsigned int)args->GetInt(3), numVertices = (unsigned int)args->GetInt(4),
                                           startIndex = (unsigned int)args->GetInt(5), primCount = (unsigned int)args->GetInt(6)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::D3d9DrawIndexedPrimitive))
                goto error;
              if (!m_PipeServer->WriteUInt32(primitiveType))
                goto error;
              if (!m_PipeServer->WriteUInt32(baseVertexIndex))
                goto error;
              if (!m_PipeServer->WriteUInt32(minVertexIndex))
                goto error;
              if (!m_PipeServer->WriteUInt32(numVertices))
                goto error;
              if (!m_PipeServer->WriteUInt32(startIndex))
                goto error;
              if (!m_PipeServer->WriteUInt32(primCount))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::WaitForGpu:
            m_DrawingThreadedQueue.Queue([this] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32((UINT32)DrawingReply::WaitForGpu))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::BeginCleanState:
            m_DrawingThreadedQueue.Queue([this] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::BeginCleanState))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::EndCleanState:
            m_DrawingThreadedQueue.Queue([this] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::EndCleanState))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::CreateCefWindowTextureSharedHandle:
            // We only have one type of shared handle supported at the moment (this one), so no need to do anything here.
            return true;
          case CefDrawingInteropMessage::CreateDrawingData:
            m_DrawingThreadedQueue.Queue([this,size = (unsigned int)args->GetInt(1),dataIndex =
                (unsigned __int64)args->GetInt(2) |
                ((unsigned __int64)args->GetInt(3) << 32)] {
              m_DrawingData.emplace(
                  std::piecewise_construct, std::forward_as_tuple(dataIndex),
                  std::forward_as_tuple(
                      size, malloc(size)));  // malloc might fail, but I am not
                                             // handling that for now, there's
                                             // not much the user could do atm.
            });
            return true;
          case CefDrawingInteropMessage::AfxDrawingHandle_Release:
            // We only have one type of shared handle supported at the moment
            // (this one), so no need to do anything here.
            return true;
          case CefDrawingInteropMessage::AfxDrawingData_Release:
            m_DrawingThreadedQueue.Queue([this, dataIndex =
                (unsigned __int64)args->GetInt(1) |
                ((unsigned __int64)args->GetInt(2) << 32)] { m_DrawingData.erase(dataIndex);
            });
            return true;
          case CefDrawingInteropMessage::AfxDrawingData_Update: {
            m_DrawingThreadedQueue.Queue(
                [this, dataIndex = (unsigned __int64)args->GetInt(1) |
                             ((unsigned __int64)args->GetInt(2) << 32),
                 offset = (unsigned int)args->GetInt(3), data = std::move(CBinaryData(args->GetBinary(4)))] {
                  if (data.Data) {
                    auto it = m_DrawingData.find(dataIndex);
                    if (it != m_DrawingData.end())
                      it->second.Update(offset, data.Size, data.Data);
                  }
            });
          }
            return true;
          case CefDrawingInteropMessage::AfxD3d9VertexDeclaration_Release:
            m_DrawingThreadedQueue.Queue(
                [this, index = (unsigned __int64)args->GetInt(1) |
                               ((unsigned __int64)args->GetInt(2) << 32)] {
                  if (!m_InFlow)
                    return;

                  if (!m_PipeServer->WriteUInt32(
                          (UINT32)DrawingReply::ReleaseD3d9VertexDeclaration))
                    goto error;
                  if (!m_PipeServer->WriteUInt64((UINT64)index))
                    goto error;

                  return;
                error:
                  Close();
            });
            return true;
          case CefDrawingInteropMessage::AfxD3d9IndexBuffer_Release:
            m_DrawingThreadedQueue.Queue(
                [this, index = (unsigned __int64)args->GetInt(1) |
                               ((unsigned __int64)args->GetInt(2) << 32)] {
                  if (!m_InFlow)
                    return;

                  if (!m_PipeServer->WriteUInt32(
                          (UINT32)DrawingReply::ReleaseD3d9IndexBuffer))
                    goto error;
                  if (!m_PipeServer->WriteUInt64((UINT64)index))
                    goto error;

                  return;
                error:
                  Close();
            });
            return true;
          case CefDrawingInteropMessage::AfxD3d9IndexBuffer_Update:
            m_DrawingThreadedQueue.Queue(
                [this, index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 dataIndex = (unsigned __int64)args->GetInt(3) |
                             ((unsigned __int64)args->GetInt(4) << 32),
                 offsetToLock = (unsigned int)args->GetInt(5),
                 sizeToLock = (unsigned int)args->GetInt(6)] {
                  if (!m_InFlow)
                    return;

                  auto it = m_DrawingData.find(dataIndex);
                  if (it != m_DrawingData.end()) {
                    CDrawingData& data = it->second;

                    if (!m_PipeServer->WriteUInt32(
                            (UINT32)DrawingReply::UpdateD3d9IndexBuffer))
                      goto error;
                    if (!m_PipeServer->WriteUInt64((UINT64)index))
                      goto error;
                    if (!m_PipeServer->WriteUInt32((UINT32)offsetToLock))
                      goto error;
                    if (!m_PipeServer->WriteUInt32((UINT32)sizeToLock))
                      goto error;
                    if (!m_PipeServer->WriteBytes(
                            (unsigned char*)data.GetData() + offsetToLock,
                            offsetToLock, sizeToLock))
                      goto error;
                  }

                  return;
                error:
                  Close();
            });
            return true;
          case CefDrawingInteropMessage::AfxD3d9VertexBuffer_Release:
            m_DrawingThreadedQueue.Queue(
                [this, index = (unsigned __int64)args->GetInt(1) |
                               ((unsigned __int64)args->GetInt(2) << 32)] {
                  if (!m_InFlow)
                    return;

                  if (!m_PipeServer->WriteUInt32(
                          (UINT32)DrawingReply::ReleaseD3d9VertexBuffer))
                    goto error;
                  if (!m_PipeServer->WriteUInt64((UINT64)index))
                    goto error;

                  return;
                error:
                  Close();
            });
            return true;
          case CefDrawingInteropMessage::AfxD3d9VertexBuffer_Update:
            m_DrawingThreadedQueue.Queue(
                [this, index = (unsigned __int64)args->GetInt(1) |
                         ((unsigned __int64)args->GetInt(2) << 32),
                 dataIndex = (unsigned __int64)args->GetInt(3) |
                             ((unsigned __int64)args->GetInt(4) << 32),
                 offsetToLock = (unsigned int)args->GetInt(5),
                 sizeToLock = (unsigned int)args->GetInt(6)] {
              if (!m_InFlow)
                return;

              auto it = m_DrawingData.find(dataIndex);
              if (it != m_DrawingData.end()) {
                CDrawingData& data = it->second;

                if (!m_PipeServer->WriteUInt32(
                        (UINT32)DrawingReply::UpdateD3d9VertexBuffer))
                  goto error;
                if (!m_PipeServer->WriteUInt64((UINT64)index))
                  goto error;
                if (!m_PipeServer->WriteUInt32((UINT32)offsetToLock))
                  goto error;
                if (!m_PipeServer->WriteUInt32((UINT32)sizeToLock))
                  goto error;
                if (!m_PipeServer->WriteBytes(
                        (unsigned char*)data.GetData() + offsetToLock,
                        offsetToLock, sizeToLock))
                  goto error;
              }

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::AfxD3d9Texture_Release:
            m_DrawingThreadedQueue.Queue([this, index = (unsigned __int64)args->GetInt(1) |
                               ((unsigned __int64)args->GetInt(2) << 32)] {
              if (!m_InFlow)
                return;

              if (!m_PipeServer->WriteUInt32(
                      (UINT32)DrawingReply::ReleaseD3d9Texture))
                goto error;
              if (!m_PipeServer->WriteUInt64((UINT64)index))
                goto error;

              return;
            error:
              Close();
            });
            return true;
          case CefDrawingInteropMessage::AfxD3d9Texture_Update:
            if (args->GetType(4) == VTYPE_NULL) {
              m_DrawingThreadedQueue.Queue(
                  [this,
                   index = (unsigned __int64)args->GetInt(1) |
                           ((unsigned __int64)args->GetInt(2) << 32),
                   level = (unsigned int)args->GetInt(3),
                   dataIndex = (unsigned __int64)args->GetInt(5) |
                               ((unsigned __int64)args->GetInt(6) << 32),
                   rowOffsetBytes = (unsigned int)args->GetInt(7),
                   columnOffsetBytes = (unsigned int)args->GetInt(8),
                   dataBytesPerRow = (unsigned int)args->GetInt(9),
                   totalBytesPerRow = (unsigned int)args->GetInt(10),
                   numRows = (unsigned int)args->GetInt(11)] {
                    if (!m_InFlow)
                      return;

                    auto it = m_DrawingData.find(dataIndex);
                    if (it != m_DrawingData.end()) {
                      CDrawingData& data = it->second;

                      if (!m_PipeServer->WriteUInt32(
                              (UINT32)DrawingReply::UpdateD3d9Texture))
                        goto error;
                      if (!m_PipeServer->WriteUInt64((UINT64)index))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)level))
                        goto error;
                      if (!m_PipeServer->WriteBoolean(false))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)numRows))
                        goto error;
                      if (!m_PipeServer->WriteUInt32(
                              (UINT32)(dataBytesPerRow - columnOffsetBytes)))
                        goto error;

                      void* pData =
                          (unsigned char*)data.GetData() + rowOffsetBytes;

                      for (unsigned int i = 0; i < numRows; ++i) {
                        if (!m_PipeServer->WriteBytes(
                                (unsigned char*)pData + columnOffsetBytes, 0,
                                dataBytesPerRow - columnOffsetBytes))
                          goto error;

                        pData = (unsigned char*)pData + totalBytesPerRow;
                      }
                    }
                    return;
                  error:
                    Close();
                  });
            }
            else {
              auto dict = args->GetDictionary(4);
              m_DrawingThreadedQueue.Queue(
                  [this,
                   index = (unsigned __int64) args->GetInt(1) |
                       ((unsigned __int64)args->GetInt(2) << 32),
                   level = (unsigned int) args->GetInt(3),
                   rectLeft = (unsigned int) dict->GetInt("left"),
                   rectTop = (unsigned int) dict->GetInt("top"),
                   rectRight = (unsigned int) dict->GetInt("right"),
                   rectBottom = (unsigned int) dict->GetInt("bottom"),
                   dataIndex = (unsigned __int64) args->GetInt(5) |
                       ((unsigned __int64)args->GetInt(6) << 32),
                   rowOffsetBytes = (unsigned int) args->GetInt(7),
                   columnOffsetBytes = (unsigned int) args->GetInt(8),
                   dataBytesPerRow = (unsigned int) args->GetInt(9),
                   totalBytesPerRow = (unsigned int) args->GetInt(10),
                   numRows = (unsigned int) args->GetInt(11)] {
                    if (!m_InFlow)
                      return;

                    auto it = m_DrawingData.find(dataIndex);
                    if (it != m_DrawingData.end()) {
                      CDrawingData& data = it->second;

                      if (!m_PipeServer->WriteUInt32(
                              (UINT32)DrawingReply::UpdateD3d9Texture))
                        goto error;
                      if (!m_PipeServer->WriteUInt64((UINT64)index))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)level))
                        goto error;
                      if (!m_PipeServer->WriteBoolean(true))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)rectLeft))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)rectTop))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)rectRight))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)rectBottom))
                        goto error;
                      if (!m_PipeServer->WriteUInt32((UINT32)numRows))
                        goto error;
                      if (!m_PipeServer->WriteUInt32(
                              (UINT32)(dataBytesPerRow - columnOffsetBytes)))
                        goto error;

                      void* pData =
                          (unsigned char*)data.GetData() + rowOffsetBytes;

                      for (unsigned int i = 0; i < numRows; ++i) {
                        if (!m_PipeServer->WriteBytes(
                                (unsigned char*)pData + columnOffsetBytes, 0,
                                dataBytesPerRow - columnOffsetBytes))
                          goto error;

                        pData = (unsigned char*)pData + totalBytesPerRow;
                      }
                    }
                    return;
                  error:
                    Close();
              });
            }
            return true;
          case CefDrawingInteropMessage::AfxD3d9PixelShader_Release:
              m_DrawingThreadedQueue.Queue(
                  [this, index = (unsigned __int64)args->GetInt(1) |
                       ((unsigned __int64)args->GetInt(2) << 32)] {
                    if (!m_InFlow)
                      return;

                    if (!m_PipeServer->WriteUInt32(
                            (UINT32)DrawingReply::ReleaseD3d9PixelShader))
                      goto error;
                    if (!m_PipeServer->WriteUInt64((UINT64)index))
                      goto error;

                    return;
                  error:
                    Close();
            });
            return true;
          case CefDrawingInteropMessage::AfxD3d9VertexShader_Release:
              m_DrawingThreadedQueue.Queue(
                  [this, index = (unsigned __int64)args->GetInt(1) |
                                 ((unsigned __int64)args->GetInt(2) << 32)] {
                    if (!m_InFlow)
                      return;

                    if (!m_PipeServer->WriteUInt32(
                            (UINT32)DrawingReply::ReleaseD3d9VertexShader))
                      goto error;
                    if (!m_PipeServer->WriteUInt64((UINT64)index))
                      goto error;

                    return;
                  error:
                    Close();
            });
            return true;
        }
      }
    }

    return false;
  }

  virtual void GetViewRect(CefRefPtr<CefBrowser> browser,
      CefRect& rect) override {
    rect.Set(0, 0, m_Width, m_Height);
 }

  virtual void SetSharedHandle(void* handle) override { m_ShareHandle = handle; }

protected:
  virtual ~CDrawingInteropImpl() {
    CloseInterop();
  }

 private:
  class CDrawingData {
   public:
    CDrawingData(unsigned int size, void* data) : m_Size(size), m_Data(data){}

    void Update(unsigned int offset, size_t size, void * data) {
      memcpy((unsigned char*)m_Data + offset, data, size);
    }

    void* GetData() { return m_Data;
    }

    unsigned int GetSize() { return m_Size;
    }

    private:
        unsigned int m_Size;
     void* m_Data;
  };

  CThreadedQueue m_DrawingThreadedQueue;
  std::map<unsigned __int64, CDrawingData> m_DrawingData;
  CefRefPtr<CefBrowser> m_Browser;

  bool m_InFlow = false;

  HANDLE m_ShareHandle = INVALID_HANDLE_VALUE;
  int m_Width = 640;
  int m_Height = 480;

  bool DoThePumping(int frameCount, unsigned int pass) {

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
        case DrawingMessage::DeviceLost:
          OutputDebugStringA("DeviceLost\n");
          bContinue = true;
          break;
        case DrawingMessage::DeviceRestored: {
          OutputDebugStringA("DeviceRestored\n");
          auto browserMessage = CefProcessMessage::Create("afx-interop");
          auto args = browserMessage->GetArgumentList();
          args->SetSize(1);
          args->SetInt(0, (int)CefEngineInteropMessage::DeviceReset);
          m_Browser->GetMainFrame()->SendProcessMessage(PID_RENDERER,
                                                        browserMessage);
        }
          bContinue = true;
          break;

        default:
          return false;  // Unsupported event
      }

      if (bContinue) continue;

      INT32 clientFrameCount;
      if (!m_PipeServer->ReadInt32(clientFrameCount))
        return false;

      UINT32 clientPass;
      if (!m_PipeServer->ReadUInt32(clientPass))
        return false;

      INT32 frameDiff = frameCount - clientFrameCount;

      if (frameDiff < 0 || frameDiff == 0 && pass < clientPass) {
        // Error: client is ahead, otherwise we would have correct data
        // by now.

        if (!m_PipeServer->WriteInt32((INT32)DrawingReply::Retry))
          return false;
        if (!m_PipeServer->Flush())
          return false;

        OutputDebugStringA("DrawingReply::Retry\n");

        return true;

      } else if (frameDiff > 0 || frameDiff == 0 && pass > clientPass) {
        // client is behind.

        if (!m_PipeServer->WriteInt32((INT32)DrawingReply::Skip))
          return false;
        if (!m_PipeServer->Flush())
          return false;

        OutputDebugStringA("DrawingReply::Skip\n");
      } else {
        // we are right on.

        m_InFlow = true;
        return true;
      }
    }
  }

  virtual void OnClosing() override {
    m_InFlow = false;
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

class CDrawingInterop* CreateDrawingInterop(
    CefRefPtr<CefBrowser> const& browser) {
  return new CDrawingInteropImpl(browser);
}

class CEngineInteropImpl : public CEngineInterop, public CInterop {
  IMPLEMENT_REFCOUNTING(CEngineInteropImpl);

 public:

  virtual void CloseInterop() override { CInterop::Close();
  }

  CEngineInteropImpl(CefRefPtr<CefBrowser> const& browser,
                     CefRefPtr<CefFrame> const& frame,
                     CefRefPtr<CefV8Context> const& context)
      : CInterop("advancedfxInterop"),
        m_Browser(browser),
        m_Frame(frame),
        m_Context(context) {

    auto const object = CefV8Value::CreateObject(this, nullptr);

    auto window = context->GetGlobal();
    window->SetValue("afxInterop", object, V8_PROPERTY_ATTRIBUTE_NONE);

    object->SetValue("pipeName", V8_ACCESS_CONTROL_DEFAULT,
                     V8_PROPERTY_ATTRIBUTE_NONE);
    m_GetMap.emplace(
        "pipeName",
        [this](const CefString& name, const CefRefPtr<CefV8Value> object,
               CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateString(CInterop::GetPipeName());

          return true;
        });
    m_SetMap.emplace("pipeName",
        [this](const CefString& name, const CefRefPtr<CefV8Value> object,
               const CefRefPtr<CefV8Value> value, CefString& exception) {
          if (value && value->IsString()) {
            std::string pipeName(value->GetStringValue().ToString());

            CInterop::SetPipeName(pipeName.c_str());

            pipeName.append("_drawing");

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(2);
            args->SetInt(0, (int)CefDrawingInteropMessage::SetPipeName);
            args->SetString(1, pipeName);
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });

    m_Connect.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "connect",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          retval = CefV8Value::CreateBool(CInterop::Connection());
          return true;
        });

    m_GetConnected.InitFunc(object, this, m_ExecuteMap, m_GetMap, "getConnected",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          retval = CefV8Value::CreateBool(CInterop::Connected());
          return true;
        });

    m_Close.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "close",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          CInterop::Close();

          auto message = CefProcessMessage::Create("afx-interop");
          auto args = message->GetArgumentList();
          args->SetSize(1);
          args->SetInt(0, (int)CefDrawingInteropMessage::Close);
          SendProcessMessage(PID_BROWSER, message);
          return true;
        });
                
    m_NewConnectionCallback.InitCallback(context, object, this, m_ExecuteMap,
                                         m_GetMap, "onNewConnection");

    m_CommandsCallback.InitCallback(context, object, this, m_ExecuteMap,
                                    m_GetMap, "onCommands");

    m_ScheduleCommand.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "scheduleCommand",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 <= arguments.size()) {
            const CefRefPtr<CefV8Value>& arg0 = arguments[0];
            if (arg0->IsString()) {
              m_Commands.emplace(arg0->GetStringValue().ToString().c_str());
              return true;
            }
          }

          return false;
        });

    m_RenderViewBeginCallback.InitCallback(context, object, this, m_ExecuteMap,
                                           m_GetMap, "onRenderViewBegin");

    m_OnViewOverrideCallback.InitCallback(context, object, this, m_ExecuteMap,
                                          m_GetMap, "onViewOverride");


    m_RenderPassCallback.InitCallback(context, object, this, m_ExecuteMap,
                                      m_GetMap, "onRenderPass");

    m_HudBeginCallback.InitCallback(context, object, this, m_ExecuteMap,
                                    m_GetMap, "onHudBegin");

    m_HudEndCallback.InitCallback(context, object, this, m_ExecuteMap,
                                    m_GetMap, "onHudEnd");

    m_RenderViewEndCallback.InitCallback(context, object, this, m_ExecuteMap,
                                    m_GetMap, "onRenderViewEnd");

    m_AddCalcHandle.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "addCalcHandle",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsString() && arguments[1]->IsFunction()) {
            retval = (new CCalcResult(
                          &m_HandleCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CCalcCallback(arguments[1], m_Context)))
                         ->GetObj();

            return true;
          }

          return false;
        });
    m_AddCalcVecAng.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "addCalcVecAng",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsString() && arguments[1]->IsFunction()) {
            retval = (new CCalcResult(
                          &m_VecAngCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CCalcCallback(arguments[1], m_Context)))
                         ->GetObj();

            return true;
          }

          return false;
        });
    m_AddCalcCam.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "addCalcCam",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsString() && arguments[1]->IsFunction()) {
            retval = (new CCalcResult(
                          &m_CamCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CCalcCallback(arguments[1], m_Context)))
                         ->GetObj();

            return true;
          }

          return false;
        });
    m_AddCalcFov.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "addCalcFov",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsString() && arguments[1]->IsFunction()) {
            retval = (new CCalcResult(
                          &m_FovCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CCalcCallback(arguments[1], m_Context)))
                         ->GetObj();

            return true;
          }

          return false;
        });
    m_AddCalcBool.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "addCalcBool",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsString() && arguments[1]->IsFunction()) {
            std::string valName = arguments[0]->GetStringValue().ToString();

            retval = (new CCalcResult(
                          &m_BoolCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CCalcCallback(arguments[1], m_Context)))
                         ->GetObj();

            return true;
          }

          return false;
        });
    m_AddCalcInt.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "addCalcInt",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsString() && arguments[1]->IsFunction()) {
            std::string valName = arguments[0]->GetStringValue().ToString();

            retval = (new CCalcResult(
                          &m_IntCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                          new CCalcCallback(arguments[1], m_Context)))
                         ->GetObj();

            return true;
          }

          return false;
        });

    m_GameEventAllowAdd.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventAllowAdd",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] &&
              arguments[0]->IsString()) {
            std::string val = arguments[0]->GetStringValue().ToString();

            m_GameEventsAllowDeletions.erase(val);
            m_GameEventsAllowAdditions.insert(val);

            return true;
          }

          return false;
        });
    m_GameEventAllowRemove.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventAllowRemove",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] &&
              arguments[0]->IsString()) {
            std::string val = arguments[0]->GetStringValue().ToString();

            m_GameEventsAllowAdditions.erase(val);
            m_GameEventsAllowDeletions.insert(val);

            return true;
          }

          return false;
        });
    m_GameEventDenyAdd.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventDenyAdd",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] &&
              arguments[0]->IsString()) {
            std::string val = arguments[0]->GetStringValue().ToString();

            m_GameEventsDenyDeletions.erase(val);
            m_GameEventsDenyAdditions.insert(val);

            return true;
          }

          return false;
        });
    m_GameEventDenyRemove.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventDenyRemove",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] &&
              arguments[0]->IsString()) {
            std::string val = arguments[0]->GetStringValue().ToString();

            m_GameEventsDenyAdditions.erase(val);
            m_GameEventsDenyDeletions.insert(val);

            return true;
          }

          return false;
        });
    m_GameEventSetEnrichment.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventSetEnrichment",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (3 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsString() &&
              arguments[1]->IsString() && arguments[2]->IsUInt()) {
            std::string strEvent = arguments[0]->GetStringValue().ToString();
            std::string strProperty = arguments[1]->GetStringValue().ToString();
            unsigned int uiEnrichments = arguments[2]->GetUIntValue();

            m_GameEventsEnrichmentsChanges[GameEventEnrichmentKey_s(
                strEvent.c_str(), strProperty.c_str())] = uiEnrichments;

            return true;
          }

          return false;
        });

    m_GameEventCallback.InitCallback(context, object, this, m_ExecuteMap,
                                         m_GetMap, "onGameEvent");

    m_GameEventSetTransmitClientTime.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventSetTransmitClientTime",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] && arguments[0]->IsBool()) {
            bool value = arguments[0]->GetBoolValue();

            m_GameEventsTransmitChanged =
                m_GameEventsTransmitChanged ||
                value != m_GameEventsTransmitClientTime;
            m_GameEventsTransmitClientTime = value;

            return true;
          }

          return false;
        });

    m_GameEventSetTransmitTick.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventSetTransmitTick",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] && arguments[0]->IsBool()) {
            bool value = arguments[0]->GetBoolValue();

            m_GameEventsTransmitChanged = m_GameEventsTransmitChanged ||
                                          value != m_GameEventsTransmitTick;
            m_GameEventsTransmitTick = value;

            return true;
          }

          return false;
        });

    m_GameEventSetTransmitSystemTime.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "gameEventSetTransmitSystemTime",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] && arguments[0]->IsBool()) {
            bool value = arguments[0]->GetBoolValue();

            m_GameEventsTransmitChanged =
                m_GameEventsTransmitChanged ||
                value != m_GameEventsTransmitSystemTime;
            m_GameEventsTransmitSystemTime = value;

            return true;
          }

          return false;
        });


    m_OnDeviceReset.InitCallback(context, object, this, m_ExecuteMap,
                                     m_GetMap, "onDeviceReset");
    m_DrawingConnect.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "drawingConnect",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(1);
            args->SetInt(0, (int)CefDrawingInteropMessage::Connect);

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_DrawingPumpBegin.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "drawingPumpBegin",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {

          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsInt() && arguments[1]->IsUInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(3);
            args->SetInt(0, (int)CefDrawingInteropMessage::PumpBegin);
            args->SetInt(1, arguments[0]->GetIntValue());
            args->SetInt(2, (unsigned int)arguments[1]->GetUIntValue());

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_DrawingPumpEnd.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "drawingPumpEnd",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(1);
            args->SetInt(0, (int)CefDrawingInteropMessage::PumpEnd);

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_DrawingBeginFrame.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "drawingBeginFrame",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(1);
            args->SetInt(0, (int)CefDrawingInteropMessage::BeginFrame);

            SendProcessMessage(PID_BROWSER, message);
            return true;
          } else if (2 == arguments.size() && arguments[0] && arguments[1] &&
                     arguments[0]->IsInt() && arguments[1]->IsInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(3);
            args->SetInt(0, (int)CefDrawingInteropMessage::BeginFrame);
            args->SetInt(1, arguments[0]->GetIntValue());
            args->SetInt(2, arguments[1]->GetIntValue());

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9CreateVertexDecalaration.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9CreateVertexDeclaration",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[0]->GetUserData() &&
              static_cast<CAfxInteropUserData*>(
                  arguments[0]->GetUserData().get())
                      ->GetUserDataType() == AfxUserDataType::AfxDrawingData) {
            auto drawingData =
                static_cast<CAfxDrawingData*>(static_cast<CAfxInteropUserData*>(
                    arguments[0]->GetUserData().get()));

            if (nullptr != drawingData) {
              CAfxD3d9VertexDeclaration* val =
                  new CAfxD3d9VertexDeclaration(this);
              retval = val->GetObj();

              auto message = CefProcessMessage::Create("afx-interop");
              auto args = message->GetArgumentList();
              args->SetSize(5);
              args->SetInt(
                  0,
                  (int)CefDrawingInteropMessage::D3d9CreateVertexDecalaration);
              val->InsertIdAsTwoInts(args, 1, 2);
              drawingData->InsertIdAsTwoInts(args, 3, 4);

              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9CreateIndexBuffer.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9CreateIndexBuffer",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
          if (5 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[3] && arguments[4] &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt()) {
            CAfxDrawingHandle* drawingHandle = nullptr;

            if (arguments[4]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[4]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxDrawingHandle)
              drawingHandle = static_cast<CAfxDrawingHandle*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[4]->GetUserData().get()));

            CAfxD3d9IndexBuffer* val = new CAfxD3d9IndexBuffer(this);
            retval = val->GetObj();

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(9);
            args->SetInt(0,
                         (int)CefDrawingInteropMessage::D3d9CreateIndexBuffer);
            val->InsertIdAsTwoInts(args, 1, 2);
            args->SetInt(3, (int)arguments[0]->GetUIntValue());
            args->SetInt(4, (int)arguments[1]->GetUIntValue());
            args->SetInt(5, (int)arguments[2]->GetUIntValue());
            args->SetInt(6, (int)arguments[3]->GetUIntValue());
            if (nullptr != drawingHandle)
              drawingHandle->InsertIdAsTwoInts(args, 7, 8);
            else {
              args->SetInt(7, 0);
              args->SetInt(8, 0);
            }

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9CreateVertexBuffer.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9CreateVertexBuffer",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (5 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[3] && arguments[4] &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt()) {
            CAfxDrawingHandle* drawingHandle = nullptr;

            if (arguments[4]->GetUserData() && arguments[4]
                    ->GetUserData() && static_cast<CAfxInteropUserData*>(
                    arguments[4]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxDrawingHandle)
              drawingHandle = static_cast<CAfxDrawingHandle*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[4]->GetUserData().get()));

            CAfxD3d9VertexBuffer* val = new CAfxD3d9VertexBuffer(this);
            retval = val->GetObj();

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(9);
            args->SetInt(0,
                         (int)CefDrawingInteropMessage::D3d9CreateVertexBuffer);
            val->InsertIdAsTwoInts(args, 1, 2);
            args->SetInt(3, (int)arguments[0]->GetUIntValue());
            args->SetInt(4, (int)arguments[1]->GetUIntValue());
            args->SetInt(5, (int)arguments[2]->GetUIntValue());
            args->SetInt(6, (int)arguments[3]->GetUIntValue());
            if (nullptr != drawingHandle)
              drawingHandle->InsertIdAsTwoInts(args, 7, 8);
            else {
              args->SetInt(7, 0);
              args->SetInt(8, 0);
            }

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9CreateTexture.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9CreateTexture",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (7 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[3] && arguments[4] && arguments[5] &&
              arguments[0]->IsUInt() &&
              arguments[1]->IsUInt() && arguments[2]->IsUInt() &&
              arguments[3]->IsUInt() && arguments[5]->IsUInt() &&
              arguments[4]->IsUInt()) {
            CAfxDrawingHandle* drawingHandle = nullptr;

            if (arguments[6]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[6]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxDrawingHandle)
              drawingHandle = static_cast<CAfxDrawingHandle*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[6]->GetUserData().get()));

            CAfxD3d9Texture* val = new CAfxD3d9Texture(this);
            retval = val->GetObj();

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(11);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9CreateTexture);
            val->InsertIdAsTwoInts(args, 1, 2);
            args->SetInt(3, (int)arguments[0]->GetUIntValue());
            args->SetInt(4, (int)arguments[1]->GetUIntValue());
            args->SetInt(5, (int)arguments[2]->GetUIntValue());
            args->SetInt(6, (int)arguments[3]->GetUIntValue());
            args->SetInt(7, (int)arguments[4]->GetUIntValue());
            args->SetInt(8, (int)arguments[5]->GetUIntValue());
            if (nullptr != drawingHandle)
              drawingHandle->InsertIdAsTwoInts(args, 9, 10);
            else {
              args->SetInt(9, 0);
              args->SetInt(10, 0);
            }

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9CreateVertexShader.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9CreateVertexShader",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
                const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] &&
              arguments[0]->GetUserData() &&
              static_cast<CAfxInteropUserData*>(
                  arguments[0]->GetUserData().get())
                      ->GetUserDataType() == AfxUserDataType::AfxDrawingData) {
            auto drawingData =
                static_cast<CAfxDrawingData*>(static_cast<CAfxInteropUserData*>(
                    arguments[0]->GetUserData().get()));

            if (drawingData) {
              CAfxD3d9VertexShader* val = new CAfxD3d9VertexShader(this);
              retval = val->GetObj();

              auto message = CefProcessMessage::Create("afx-interop");
              auto args = message->GetArgumentList();
              args->SetSize(5);
              args->SetInt(
                  0,
                  (int)CefDrawingInteropMessage::D3d9CreateVertexShader);
              val->InsertIdAsTwoInts(args, 1, 2);
              drawingData->InsertIdAsTwoInts(args, 3, 4);

              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9CreatePixelShader.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9CreatePixelShader",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] &&
              arguments[0]->GetUserData() &&
              static_cast<CAfxInteropUserData*>(
                  arguments[0]->GetUserData().get())
                      ->GetUserDataType() == AfxUserDataType::AfxDrawingData) {
            auto drawingData =
                static_cast<CAfxDrawingData*>(static_cast<CAfxInteropUserData*>(
                    arguments[0]->GetUserData().get()));

            if (drawingData) {
              CAfxD3d9PixelShader* val = new CAfxD3d9PixelShader(this);
              retval = val->GetObj();

              auto message = CefProcessMessage::Create("afx-interop");
              auto args = message->GetArgumentList();
              args->SetSize(5);
              args->SetInt(
                  0,
                  (int)CefDrawingInteropMessage::D3d9CreatePixelShader);
              val->InsertIdAsTwoInts(args, 1, 2);
              drawingData->InsertIdAsTwoInts(args, 3, 4);

              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9UpdateTexture.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9UpdateTexture",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (3 == arguments.size() && arguments[0] && arguments[1]) {
            CAfxD3d9Texture* val = nullptr;
            if (arguments[0]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[0]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxD3d9Texture)
              val = static_cast<CAfxD3d9Texture*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[0]->GetUserData().get()));

            CAfxD3d9Texture* val2 = nullptr;
            if (arguments[1]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[1]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxD3d9Texture)
              val2 = static_cast<CAfxD3d9Texture*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[1]->GetUserData().get()));

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(5);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9UpdateTexture);
            if (val)
              val->InsertIdAsTwoInts(args, 1, 2);
            else {
              args->SetInt(1, 0);
              args->SetInt(2, 0);
            }
            if (val2)
              val2->InsertIdAsTwoInts(args, 3, 4);
            else {
              args->SetInt(3, 0);
              args->SetInt(4, 0);
            }

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetViewport.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetViewport",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(1);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetViewport);
            SendProcessMessage(PID_BROWSER, message);
            return true;
          } else if (1 == arguments.size() && arguments[0] &&
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
              auto message = CefProcessMessage::Create("afx-interop");
              auto args = message->GetArgumentList();
              args->SetSize(7);
              args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetViewport);
              args->SetInt(1, (int)x->GetUIntValue());
              args->SetInt(2, (int)y->GetUIntValue());
              args->SetInt(3, (int)width->GetUIntValue());
              args->SetInt(4, (int)height->GetUIntValue());
              args->SetDouble(5, (int)minZ->GetUIntValue());
              args->SetDouble(6, (int)maxZ->GetUIntValue());
              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9SetRenderState.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetRenderState",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(3);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetRenderState);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            args->SetInt(2, (int)arguments[1]->GetUIntValue());
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetSamplerState.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetSamplerState",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (3 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetSamplerState);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            args->SetInt(2, (int)arguments[1]->GetUIntValue());
            args->SetInt(3, (int)arguments[2]->GetUIntValue());
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetTexture.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetTexture",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt()) {
            CAfxD3d9Texture* val = nullptr;
            if (arguments[1]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[1]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxD3d9Texture)
              val = static_cast<CAfxD3d9Texture*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[1]->GetUserData().get()));

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetTexture);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            if (val)
              val->InsertIdAsTwoInts(args, 2, 3);
            else {
              args->SetInt(2, 0);
              args->SetInt(3, 0);
            }
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetTextureStageState.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetTextureStageState",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (3 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetTextureStageState);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            args->SetInt(2, (int)arguments[1]->GetUIntValue());
            args->SetInt(3, (int)arguments[2]->GetUIntValue());
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetTransform.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetTransform",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt()) {
            if (arguments[1]->IsArray() &&
                arguments[1]->GetArrayLength() == 16) {
              auto message = CefProcessMessage::Create("afx-interop");
              auto args = message->GetArgumentList();
              args->SetSize(18);
              args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetTransform);
              args->SetInt(1, (int)arguments[0]->GetUIntValue());
              bool bOk = true;
              for (int i = 0; i < 16; ++i) {
                auto arrVal = arguments[1]->GetValue(i);

                if (arrVal && arrVal->IsDouble()) {
                  args->SetDouble(i + 2, arrVal->GetDoubleValue());
                } else {
                  bOk = false;
                  break;
                }
              }
              if (bOk) {
                SendProcessMessage(PID_BROWSER, message);
                return true;
              }
            } else {
              auto message = CefProcessMessage::Create("afx-interop");
              auto args = message->GetArgumentList();
              args->SetSize(2);
              args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetTransform);
              args->SetInt(1, (int)arguments[0]->GetUIntValue());

              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9SetIndices.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetIndices",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt()) {
            CAfxD3d9IndexBuffer* val = nullptr;
            if (arguments[1]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[1]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxD3d9IndexBuffer)
              val = static_cast<CAfxD3d9IndexBuffer*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[1]->GetUserData().get()));

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetIndices);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            if (val)
              val->InsertIdAsTwoInts(args, 2, 3);
            else {
              args->SetInt(2, 0);
              args->SetInt(3, 0);
            }
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetStreamSource.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetStreamSource",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[3] && arguments[0]->IsUInt() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt()) {
            CAfxD3d9IndexBuffer* val = nullptr;
            if (arguments[1]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[1]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxD3d9IndexBuffer)
              val = static_cast<CAfxD3d9IndexBuffer*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[1]->GetUserData().get()));

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(6);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetStreamSource);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            if (val)
              val->InsertIdAsTwoInts(args, 2, 3);
            else {
              args->SetInt(2, 0);
              args->SetInt(3, 0);
            }
            args->SetInt(4, (int)arguments[2]->GetUIntValue());
            args->SetInt(5, (int)arguments[3]->GetUIntValue());
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetStreamSourceFreq.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetStreamSourceFreq",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt() && arguments[1]->IsUInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(3);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetStreamSourceFreq);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            args->SetInt(2, (int)arguments[1]->GetUIntValue());
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetVertexDeclaration.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetVertexDeclaration",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt()) {
            CAfxD3d9VertexDeclaration* val = nullptr;
            if (arguments[1]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[1]->GetUserData().get())
                    ->GetUserDataType() ==
                AfxUserDataType::AfxD3d9VertexDeclaration)
              val = static_cast<CAfxD3d9VertexDeclaration*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[1]->GetUserData().get()));

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetVertexDeclaration);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            if (val)
              val->InsertIdAsTwoInts(args, 2, 3);
            else {
              args->SetInt(2, 0);
              args->SetInt(3, 0);
            }
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetVertexShader.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetVertexShader",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt()) {
            CAfxD3d9VertexShader* val = nullptr;
            if (arguments[1]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[1]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxD3d9VertexShader)
              val = static_cast<CAfxD3d9VertexShader*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[1]->GetUserData().get()));

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetVertexShader);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            if (val)
              val->InsertIdAsTwoInts(args, 2, 3);
            else {
              args->SetInt(2, 0);
              args->SetInt(3, 0);
            }
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetVertexShaderConstantB.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetVertexShaderConstantB",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[2]->IsArray()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            size_t arrLen = arguments[2]->GetArrayLength();

            args->SetSize(2 + arrLen);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetVertexShaderConstantB);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[2]->GetValue(i);

              if (arrVal && arrVal->IsBool()) {
                args->SetBool(i + 2, arrVal->GetBoolValue());
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9SetVertexShaderConstantF.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetVertexShaderConstantF",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[2]->IsArray()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            size_t arrLen = arguments[2]->GetArrayLength();

            args->SetSize(2 + arrLen);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetVertexShaderConstantF);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[2]->GetValue(i);

              if (arrVal && arrVal->IsDouble()) {
                args->SetDouble(i + 2, arrVal->GetDoubleValue());
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9SetVertexShaderConstantI.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetVertexShaderConstantI",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[2]->IsArray()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            size_t arrLen = arguments[2]->GetArrayLength();

            args->SetSize(2 + arrLen);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetVertexShaderConstantI);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[2]->GetValue(i);

              if (arrVal && arrVal->IsInt()) {
                args->SetInt(i + 2, arrVal->GetIntValue());
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9SetPixelShader.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetPixelShader",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[0]->IsUInt()) {
            CAfxD3d9PixelShader* val = nullptr;
            if (arguments[1]->GetUserData() &&
                static_cast<CAfxInteropUserData*>(
                    arguments[1]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxD3d9PixelShader)
              val = static_cast<CAfxD3d9PixelShader*>(
                  static_cast<CAfxInteropUserData*>(
                      arguments[1]->GetUserData().get()));

            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9SetPixelShader);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            if (val)
              val->InsertIdAsTwoInts(args, 2, 3);
            else {
              args->SetInt(2, 0);
              args->SetInt(3, 0);
            }
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9SetPixelShaderConstantB.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetPixelShaderConstantB",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[2]->IsArray()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            size_t arrLen = arguments[2]->GetArrayLength();

            args->SetSize(2 + arrLen);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetPixelShaderConstantB);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[2]->GetValue(i);

              if (arrVal && arrVal->IsBool()) {
                args->SetBool(i + 2, arrVal->GetBoolValue());
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9SetPixelShaderConstantF.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetPixelShaderConstantF",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[2]->IsArray()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            size_t arrLen = arguments[2]->GetArrayLength();

            args->SetSize(2 + arrLen);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetPixelShaderConstantF);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[2]->GetValue(i);

              if (arrVal && arrVal->IsDouble()) {
                args->SetDouble(i + 2, arrVal->GetDoubleValue());
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9SetPixelShaderConstantI.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9SetPixelShaderConstantI",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (2 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[2]->IsArray()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            size_t arrLen = arguments[2]->GetArrayLength();

            args->SetSize(2 + arrLen);
            args->SetInt(
                0, (int)CefDrawingInteropMessage::D3d9SetPixelShaderConstantI);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[2]->GetValue(i);

              if (arrVal && arrVal->IsInt()) {
                args->SetInt(i + 2, arrVal->GetIntValue());
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
    m_D3d9DrawPrimitive.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9DrawPrimitive",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (3 == arguments.size() && arguments[0] && arguments[1] &&
              arguments[2] && arguments[0]->IsUInt() &&
              arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(4);
            args->SetInt(0, (int)CefDrawingInteropMessage::D3d9DrawPrimitive);
            args->SetInt(1, (int)arguments[0]->GetUIntValue());
            args->SetInt(2, (int)arguments[1]->GetUIntValue());
            args->SetInt(3, (int)arguments[2]->GetUIntValue());
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_D3d9DrawIndexedPrimitive.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "d3d9DrawIndexedPrimitive",
        [this](const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exceptionoverride) {
      if (6 == arguments.size() && arguments[0] && arguments[1] &&
          arguments[2] && arguments[3] && arguments[4] && arguments[5] &&
          arguments[0]->IsUInt() && arguments[1]->IsUInt() &&
          arguments[2]->IsUInt() && arguments[3]->IsUInt() &&
          arguments[4]->IsUInt() && arguments[5]->IsUInt()) {
        auto message = CefProcessMessage::Create("afx-interop");
        auto args = message->GetArgumentList();
        args->SetSize(7);
        args->SetInt(0,
                     (int)CefDrawingInteropMessage::D3d9DrawIndexedPrimitive);
        args->SetInt(1, (int)arguments[0]->GetUIntValue());
        args->SetInt(2, (int)arguments[1]->GetUIntValue());
        args->SetInt(3, (int)arguments[2]->GetUIntValue());
        args->SetInt(4, (int)arguments[0]->GetUIntValue());
        args->SetInt(5, (int)arguments[1]->GetUIntValue());
        args->SetInt(6, (int)arguments[2]->GetUIntValue());
        SendProcessMessage(PID_BROWSER, message);
        return true;
      }

      return false;
  });
    m_WaitForGpu.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "waitForGpu",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(1);
            args->SetInt(0, (int)CefDrawingInteropMessage::WaitForGpu);
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_BeginCleanState.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "beginCleanState",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(1);
            args->SetInt(0, (int)CefDrawingInteropMessage::BeginCleanState);
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_EndCleanState.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "endCleanState",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();
            args->SetSize(1);
            args->SetInt(0, (int)CefDrawingInteropMessage::EndCleanState);
            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_CreateCefWindowTextureSharedHandle.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "createCefWindowTextureSharedHandle",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (0 == arguments.size()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            CAfxDrawingHandle* val = new CAfxDrawingHandle(this);
            retval = val->GetObj();

            args->SetSize(3);
            args->SetInt(0, (int)CefDrawingInteropMessage::
                                CreateCefWindowTextureSharedHandle);
            val->InsertIdAsTwoInts(args, 1, 2);

            SendProcessMessage(PID_BROWSER, message);
            return true;
          }

          return false;
        });
    m_CreateDrawingData.InitFunc(
        object, this, m_ExecuteMap, m_GetMap, "createDrawingData",
        [this](const CefString& name, CefRefPtr<CefV8Value> object,
           const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          if (1 == arguments.size() && arguments[0] && arguments[0]->IsUInt()) {
            auto message = CefProcessMessage::Create("afx-interop");
            auto args = message->GetArgumentList();

            unsigned int size = arguments[0]->GetUIntValue();
            if (void* pData = malloc(size)) {
              CAfxDrawingData* val = new CAfxDrawingData(this, size, pData);
              retval = val->GetObj();

              args->SetSize(4);
              args->SetInt(0, (int)CefDrawingInteropMessage::CreateDrawingData);
              args->SetInt(1, (int)arguments[0]->GetUIntValue());
              val->InsertIdAsTwoInts(args, 2, 3);

              SendProcessMessage(PID_BROWSER, message);
              return true;
            }
          }

          return false;
        });
  }

  virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message) {
    auto const name = message->GetName().ToString();

    if (name == "afx-interop") {
      auto const args = message->GetArgumentList();
      auto const argC = args->GetSize();

      if (1 <= argC) {
        int msg_id = args->GetInt(0);

        switch ((CefEngineInteropMessage)msg_id) {
            case CefEngineInteropMessage::DeviceReset:
            m_OnDeviceReset.ExecuteCallback(CefV8ValueList());
                return true;
        }

      }
    }

    return false;
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) override {
  
    auto it = m_ExecuteMap.find(name.ToString());

    if (it != m_ExecuteMap.end()) {
      it->second(name, object, arguments, retval, exception);
      return true;
    }

    return false;
  }

  bool Get(const CefString& name,
      const CefRefPtr<CefV8Value> object,
      CefRefPtr<CefV8Value>& retval,
      CefString& exception) override {

      auto it = m_GetMap.find(name.ToString());

      if (it != m_GetMap.end())
        return it->second(name, object, retval, exception);

      return false;
  }

  bool Set(const CefString& name,
           const CefRefPtr<CefV8Value> object,
           const CefRefPtr<CefV8Value> value,
           CefString& exception) override {
    auto it = m_SetMap.find(name.ToString());

    if (it != m_SetMap.end())
      return it->second(name, object, value, exception);

    return false;
  }

 protected:

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

            if (m_CommandsCallback.IsValid()) m_CommandsCallback.ExecuteCallback(args);
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

          if (m_RenderViewBeginCallback.IsValid()) {
            CefV8ValueList args;
            args.push_back(CreateAfxRenderInfo(renderInfo));

            CefRefPtr<CefV8Value> retval =
                m_RenderViewBeginCallback.ExecuteCallback(args);

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
          if (m_RenderViewEndCallback.IsValid())
            m_RenderViewEndCallback.ExecuteCallback(CefV8ValueList());
          done = true;
          break;

        case EngineMessage::BeforeHud:
          if (m_HudBeginCallback.IsValid())
            m_HudBeginCallback.ExecuteCallback(CefV8ValueList());
          break;

        case EngineMessage::AfterHud:
          if (m_HudEndCallback.IsValid())
            m_HudEndCallback.ExecuteCallback(CefV8ValueList());
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

          if (m_OnViewOverrideCallback.IsValid()) {

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
                m_OnViewOverrideCallback.ExecuteCallback(args);

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

    if(m_NewConnectionCallback.IsValid()) m_NewConnectionCallback.ExecuteCallback(CefV8ValueList());
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

  CefRefPtr<CefBrowser> const m_Browser;
  CefRefPtr<CefFrame> const m_Frame;
  CefRefPtr<CefV8Context> const m_Context;

  typedef std::function<bool(const CefString& name,
                             CefRefPtr<CefV8Value> object,
                             const CefV8ValueList& arguments,
                             CefRefPtr<CefV8Value>& retval,
                             CefString& exception)>
      Execute_t;

  typedef std::function<bool(const CefString& name,
                             const CefRefPtr<CefV8Value> object,
                             CefRefPtr<CefV8Value>& retval,
                             CefString& exception)>
      Get_t;

  typedef std::function<bool(const CefString& name,
                             const CefRefPtr<CefV8Value> object,
                             const CefRefPtr<CefV8Value> value,
                             CefString& exception)>
      Set_t;

  typedef std::unordered_map<std::string, Execute_t> ExecuteMap_t;
  typedef std::unordered_map<std::string, Get_t> GetMap_t;
  typedef std::unordered_map<std::string, Set_t> SetMap_t;

  ExecuteMap_t m_ExecuteMap;
  GetMap_t m_GetMap;
  SetMap_t m_SetMap;


  class CFunc{
   public:
    void InitFunc(CefRefPtr<CefV8Value> object,
                  CefRefPtr<CefV8Handler> const& handler,
                  ExecuteMap_t& executeMap,
                  GetMap_t& getMap,
                  const char* name,
                  Execute_t execute) {
      m_Fn = CefV8Value::CreateFunction(name, handler);

      executeMap.emplace(name, execute);

      getMap.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                     std::forward_as_tuple(std::bind<bool>(
                         &CFunc::Get, this, std::placeholders::_1,
                         std::placeholders::_2, std::placeholders::_3,
                         std::placeholders::_4)));

      object->SetValue(name, V8_ACCESS_CONTROL_DEFAULT,
                       V8_PROPERTY_ATTRIBUTE_NONE);
    }

    ~CFunc() { m_Fn = nullptr; }

    bool Get(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             CefRefPtr<CefV8Value>& retval,
             CefString& exception) {

      retval = m_Fn;
      return true;;
    }

   protected:
    CefRefPtr<CefV8Value> m_Fn;
  };

  class CCallback : public CFunc {
   public:
    void InitCallback(const CefRefPtr<CefV8Context>& context,
                      const CefRefPtr<CefV8Value>& object,
              CefRefPtr<CefV8Handler> const& handler,
              ExecuteMap_t& executeMap,
              GetMap_t& getMap,
                      const char* name){

      InitFunc(object, handler, executeMap, getMap, name,
               [this](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval, CefString& exception) {
                 if (1 <= arguments.size()) {
                   const CefRefPtr<CefV8Value>& arg0 = arguments[0];
                   if (arg0->IsFunction()) {
                     m_CallbackFunc = arguments[0];
                     return true;
                   } else if (arg0->IsNull()) {
                     m_CallbackFunc = nullptr;
                     return true;
                   }
                 }

                 return false;
               });

      m_CallbackContext = context;
    }

    ~CCallback() {
      m_CallbackFunc = nullptr;
      m_CallbackContext = nullptr;
    }

    bool IsValid() {
        return m_CallbackFunc != nullptr;
    }

    CefRefPtr<CefV8Value> ExecuteCallback(const CefV8ValueList& arguments) {

      return m_CallbackFunc->ExecuteFunctionWithContext(m_CallbackContext, NULL,
                                                        arguments);
    }

   private:
    CefRefPtr<CefV8Value> m_CallbackFunc;
    CefRefPtr<CefV8Context> m_CallbackContext;
  };

  class CAfxInteropUserData : public CefBaseRefCounted {
   public:
    CAfxInteropUserData(AfxUserDataType userDataType)
        : m_UserDataType(userDataType) {}

    AfxUserDataType GetUserDataType() const { return m_UserDataType; }

   protected:
       ~CAfxInteropUserData() {

    }

   private:
    AfxUserDataType m_UserDataType;
  };

  void SendProcessMessage(CefProcessId target_process,
                          CefRefPtr<CefProcessMessage> &message) {

      m_Frame->SendProcessMessage(target_process, message);
  }

  class CAfxInteropImpl : public CAfxInteropUserData {
   public:
    CAfxInteropImpl(AfxUserDataType userDataType,
                    CEngineInteropImpl * engineInteropImpl)
        : CAfxInteropUserData(userDataType),
          m_EngineInteropImpl(engineInteropImpl) {}

    void InsertIdAsTwoInts(CefRefPtr<CefListValue>& list, int lo, int hi) {
      list->SetInt(lo, ((int64)this) & 0xffffffff);
      list->SetInt(hi, ((int64)this >> 32) & 0xffffffff);
    }

   protected:

    void SendDrawingMessage(CefRefPtr<CefProcessMessage> &message) {
      m_EngineInteropImpl->SendProcessMessage(PID_BROWSER, message);
    }

   private:
    CEngineInteropImpl* m_EngineInteropImpl;
  };

  class CAfxDrawingHandle : public CAfxInteropImpl,
                            public CefV8Accessor,
                            public CefV8Handler {

    IMPLEMENT_REFCOUNTING(CAfxDrawingHandle);

   public:
    CAfxDrawingHandle(CEngineInteropImpl* engineInteropImpl)
        : CAfxInteropImpl(AfxUserDataType::AfxDrawingHandle,
                          engineInteropImpl) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {

      if (m_DoReleased)
        return false;

      if (name == "release") {
        DoRelease();
        return true;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {

      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;
      return false;
    }

   private:
    virtual ~CAfxDrawingHandle() { DoRelease(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Obj;
    bool m_DoReleased = false;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(0, (int)CefDrawingInteropMessage::AfxDrawingHandle_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

  class CAfxDrawingData : public CAfxInteropImpl,
                            public CefV8Accessor,
                            public CefV8Handler, public CefV8ArrayBufferReleaseCallback {
 
     IMPLEMENT_REFCOUNTING(CAfxDrawingData);

   public:
    CAfxDrawingData(CEngineInteropImpl* engineInteropImpl,
                    unsigned int size,
                    void* data)
        : CAfxInteropImpl(AfxUserDataType::AfxDrawingData,
                          engineInteropImpl) , m_Size(size), m_Data(data) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);
      m_Fn_Update = CefV8Value::CreateFunction("update", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
      m_Obj->SetValue("buffer", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
      m_Obj->SetValue("update", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);

      m_Buffer = CefV8Value::CreateArrayBuffer(data, size, this);
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        DoRelease();
        return true;
      }

      if (name == "update") {
        if (2 == arguments.size() && arguments[0] && arguments[1] &&
            arguments[0]->IsUInt() && arguments[1]->IsUInt()) {

            //TODO: Make this more efficient maybe.

            size_t offset = arguments[0]->GetUIntValue();
            size_t size = arguments[0]->GetUIntValue();

            if (offset + size > m_Size)
              return false;

            while(0 < size) {
              size_t curSize = 1024;
              if (size < curSize)
                curSize = size;

              auto message = CefProcessMessage::Create("afx-interop");
              auto args = message->GetArgumentList();
              args->SetSize(5);
              args->SetInt(
                  0, (int)CefDrawingInteropMessage::AfxDrawingData_Update);
              InsertIdAsTwoInts(args, 1, 2);
              args->SetInt(3, (int)offset);
              args->SetBinary(4, CefBinaryValue::Create(
                                     (unsigned char*)m_Data + offset, curSize));

              offset += curSize;
              size -= curSize;

              SendDrawingMessage(message);
            }

            return true;
        }
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }
      if (name == "buffer") {
        retval = m_Buffer;
        return true;
      }
      if (name == "update") {
        retval = m_Fn_Update;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      return false;
    }

    virtual void ReleaseBuffer(void* buffer) override {
      free(buffer);
    }

   private:
    virtual ~CAfxDrawingData() {
      DoRelease();
      m_Buffer = nullptr;
    }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Fn_Update;
    CefRefPtr<CefV8Value> m_Obj;
    CefRefPtr<CefV8Value> m_Buffer;
    bool m_DoReleased = false;
    size_t m_Size;
    void * m_Data;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(0, (int)CefDrawingInteropMessage::AfxDrawingData_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

 class CAfxD3d9VertexDeclaration : public CAfxInteropImpl,
                            public CefV8Accessor,
                            public CefV8Handler {

     IMPLEMENT_REFCOUNTING(CAfxD3d9VertexDeclaration);

   public:
   CAfxD3d9VertexDeclaration(CEngineInteropImpl* engineInteropImpl)
        : CAfxInteropImpl(AfxUserDataType::AfxD3d9VertexDeclaration,
                          engineInteropImpl) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        DoRelease();
        return true;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;
      return false;
    }

   private:
    virtual ~CAfxD3d9VertexDeclaration() { DoRelease(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Obj;
    bool m_DoReleased = false;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(0, (int)CefDrawingInteropMessage::AfxD3d9VertexDeclaration_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

 class CAfxD3d9IndexBuffer : public CAfxInteropImpl,
                            public CefV8Accessor,
                            public CefV8Handler {

     IMPLEMENT_REFCOUNTING(CAfxD3d9IndexBuffer);

   public:
   CAfxD3d9IndexBuffer(CEngineInteropImpl* engineInteropImpl)
        : CAfxInteropImpl(AfxUserDataType::AfxD3d9IndexBuffer,
                          engineInteropImpl) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);
      m_Fn_Update = CefV8Value::CreateFunction("update", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
      m_Obj->SetValue("update", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
   }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
     if (m_DoReleased)
        return false;

      if (name == "release") {
       DoRelease();
        return true;
      }

      if (name == "update") {
        if (3 == arguments.size() && arguments[0] && arguments[1] &&
            arguments[2] && arguments[0]->GetUserData() &&
            static_cast<CAfxInteropUserData*>(arguments[0]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxDrawingData &&
            arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
          auto drawingData =
              static_cast<CAfxDrawingData*>(static_cast<CAfxInteropUserData*>(
                  arguments[0]->GetUserData().get()));

          auto message = CefProcessMessage::Create("afx-interop");
          auto args = message->GetArgumentList();
          args->SetSize(7);
          args->SetInt(
              0, (int)CefDrawingInteropMessage::AfxD3d9IndexBuffer_Update);
          InsertIdAsTwoInts(args, 1, 2);
          drawingData->InsertIdAsTwoInts(args, 3, 4);
          args->SetInt(5, (int)arguments[0]->GetUIntValue());
          args->SetInt(6, (int)arguments[1]->GetUIntValue());

          SendDrawingMessage(message);

          return true;
        }

        return false;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }
      if (name == "update") {
        retval = m_Fn_Update;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;
      return false;
    }

   private:
    virtual ~CAfxD3d9IndexBuffer() { DoRelease(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Fn_Update;
    CefRefPtr<CefV8Value> m_Obj;
    bool m_DoReleased = false;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(0, (int)CefDrawingInteropMessage::AfxD3d9IndexBuffer_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

class CAfxD3d9VertexBuffer : public CAfxInteropImpl,
                              public CefV8Accessor,
                              public CefV8Handler {

     IMPLEMENT_REFCOUNTING(CAfxD3d9VertexBuffer);

   public:
  CAfxD3d9VertexBuffer(CEngineInteropImpl* engineInteropImpl)
        : CAfxInteropImpl(AfxUserDataType::AfxD3d9VertexBuffer,
                          engineInteropImpl) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);
      m_Fn_Update = CefV8Value::CreateFunction("update", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
      m_Obj->SetValue("update", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        DoRelease();
        return true;
      }

      if (name == "update") {
        if (3 == arguments.size() && arguments[0] && arguments[1] &&
            arguments[2] && arguments[0]->GetUserData() &&
            static_cast<CAfxInteropUserData*>(arguments[0]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxDrawingData &&
            arguments[1]->IsUInt() && arguments[2]->IsUInt()) {
          auto drawingData =
              static_cast<CAfxDrawingData*>(static_cast<CAfxInteropUserData*>(
                  arguments[0]->GetUserData().get()));

          auto message = CefProcessMessage::Create("afx-interop");
          auto args = message->GetArgumentList();
          args->SetSize(7);
          args->SetInt(
              0, (int)CefDrawingInteropMessage::AfxD3d9VertexBuffer_Update);
          InsertIdAsTwoInts(args, 1, 2);
          drawingData->InsertIdAsTwoInts(args, 3, 4);
          args->SetInt(5, (int)arguments[0]->GetUIntValue());
          args->SetInt(6, (int)arguments[1]->GetUIntValue());

          SendDrawingMessage(message);

          return true;
        }

        return false;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }
      if (name == "update") {
        retval = m_Fn_Update;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;
      return false;
    }

   private:
    virtual ~CAfxD3d9VertexBuffer() { DoRelease(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Fn_Update;
    CefRefPtr<CefV8Value> m_Obj;
    bool m_DoReleased = false;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(0,
                   (int)CefDrawingInteropMessage::AfxD3d9VertexBuffer_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

class CAfxD3d9Texture : public CAfxInteropImpl,
                               public CefV8Accessor,
                               public CefV8Handler {
    IMPLEMENT_REFCOUNTING(CAfxD3d9Texture);

   public:
  CAfxD3d9Texture(CEngineInteropImpl* engineInteropImpl)
        : CAfxInteropImpl(AfxUserDataType::AfxD3d9Texture,
                          engineInteropImpl) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);
      m_Fn_Update = CefV8Value::CreateFunction("update", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
      m_Obj->SetValue("update", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        DoRelease();
        return true;
      }

      if (name == "update") {
        if (8 == arguments.size() && arguments[0] && arguments[1] &&
            arguments[2] && arguments[3] && arguments[4] && arguments[5] &&
            arguments[6] && arguments[7] &&
            arguments[0]->IsUInt() &&
            arguments[1]->IsObject() &&
            arguments[2]->GetUserData() &&
            static_cast<CAfxInteropUserData*>(arguments[2]->GetUserData().get())
                    ->GetUserDataType() == AfxUserDataType::AfxDrawingData &&
            arguments[3]->IsUInt() && arguments[4]->IsUInt() &&
            arguments[5]->IsUInt() && arguments[6]->IsUInt() && arguments[7]->IsUInt()) {
          auto drawingData =
              static_cast<CAfxDrawingData*>(static_cast<CAfxInteropUserData*>(
                  arguments[2]->GetUserData().get()));

          auto message = CefProcessMessage::Create("afx-interop");
          auto args = message->GetArgumentList();
          args->SetSize(12);
          args->SetInt(
              0, (int)CefDrawingInteropMessage::AfxD3d9Texture_Update);
          InsertIdAsTwoInts(args, 1, 2);
          args->SetInt(3, (int)arguments[0]->GetUIntValue());

          auto rectLeft = arguments[1]->GetValue("left");
          auto rectTop = arguments[1]->GetValue("top");
          auto rectRight = arguments[1]->GetValue("right");
          auto rectBottom = arguments[1]->GetValue("bottom");

          if (nullptr != rectLeft && nullptr != rectTop && nullptr != rectRight && nullptr != rectBottom && rectLeft->IsInt() && rectTop->IsInt() && rectRight->IsInt() && rectBottom->IsInt())
          {
            auto dict = CefDictionaryValue::Create();
            dict->SetInt("left", rectLeft->GetIntValue());
            dict->SetInt("top", rectTop->GetIntValue());
            dict->SetInt("right", rectRight->GetIntValue());
            dict->SetInt("bottom", rectBottom->GetIntValue());

            args->SetDictionary(4, dict);
          } else {
            args->SetNull(4);
          }

          drawingData->InsertIdAsTwoInts(args, 5, 6);
          args->SetInt(7, (int)arguments[3]->GetUIntValue());
          args->SetInt(8, (int)arguments[4]->GetUIntValue());
          args->SetInt(9, (int)arguments[5]->GetUIntValue());
          args->SetInt(10, (int)arguments[6]->GetUIntValue());
          args->SetInt(11, (int)arguments[7]->GetUIntValue());

          SendDrawingMessage(message);

          return true;
        }

        return false;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }
      if (name == "update") {
        retval = m_Fn_Update;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;
      return false;
    }

   private:
    virtual ~CAfxD3d9Texture() { DoRelease(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Fn_Update;
    CefRefPtr<CefV8Value> m_Obj;
    bool m_DoReleased = false;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(0, (int)CefDrawingInteropMessage::AfxD3d9Texture_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

  class CAfxD3d9PixelShader : public CAfxInteropImpl,
                                    public CefV8Accessor,
                                    public CefV8Handler {
    IMPLEMENT_REFCOUNTING(CAfxD3d9PixelShader);

   public:
    CAfxD3d9PixelShader(CEngineInteropImpl* engineInteropImpl)
        : CAfxInteropImpl(AfxUserDataType::AfxD3d9PixelShader,
                          engineInteropImpl) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        DoRelease();
        return true;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;
      return false;
    }

   private:
    virtual ~CAfxD3d9PixelShader() { DoRelease(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Obj;
    bool m_DoReleased = false;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(
          0, (int)CefDrawingInteropMessage::AfxD3d9PixelShader_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

  class CAfxD3d9VertexShader : public CAfxInteropImpl,
                              public CefV8Accessor,
                              public CefV8Handler {

    IMPLEMENT_REFCOUNTING(CAfxD3d9VertexShader);

   public:
    CAfxD3d9VertexShader(CEngineInteropImpl* engineInteropImpl)
        : CAfxInteropImpl(AfxUserDataType::AfxD3d9VertexShader,
                          engineInteropImpl) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetUserData(static_cast<CAfxInteropUserData*>(this));
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);
    }

    virtual bool Execute(const CefString& name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments,
                         CefRefPtr<CefV8Value>& retval,
                         CefString& exception) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        DoRelease();
        return true;
      }

      return false;
    }

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;

      if (name == "release") {
        retval = m_Fn_Release;
        return true;
      }

      return false;
    }

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      if (m_DoReleased)
        return false;
      return false;
    }

   private:
    virtual ~CAfxD3d9VertexShader() { DoRelease(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Obj;
    bool m_DoReleased = false;

    void DoRelease() {
      if (m_DoReleased)
        return;

      m_DoReleased = true;

      auto message = CefProcessMessage::Create("afx-interop");
      auto args = message->GetArgumentList();
      args->SetSize(3);
      args->SetInt(0,
                   (int)CefDrawingInteropMessage::AfxD3d9VertexShader_Release);
      InsertIdAsTwoInts(args, 1, 2);

      SendDrawingMessage(message);
    }
  };

  CFunc m_Connect;
  CFunc m_GetConnected;
  CFunc m_Close;
  CCallback m_NewConnectionCallback;
  CCallback m_CommandsCallback;
  CCallback m_ScheduleCommand;

  CCallback m_RenderViewBeginCallback;
  CCallback m_RenderViewEndCallback;

  std::queue<std::string> m_Commands;

  CCallback m_OnViewOverrideCallback;
  CCallback m_RenderPassCallback;
  CCallback m_HudBeginCallback;
  CCallback m_HudEndCallback;

  CHandleCalcCallbacks m_HandleCalcCallbacks;
  CVecAngCalcCallbacks m_VecAngCalcCallbacks;
  CCamCalcCallbacks m_CamCalcCallbacks;
  CFovCalcCallbacks m_FovCalcCallbacks;
  CBoolCalcCallbacks m_BoolCalcCallbacks;
  CIntCalcCallbacks m_IntCalcCallbacks;

  CFunc m_AddCalcHandle;
  CFunc m_AddCalcVecAng;
  CFunc m_AddCalcCam;
  CFunc m_AddCalcFov;
  CFunc m_AddCalcBool;
  CFunc m_AddCalcInt;

  CFunc m_GameEventAllowAdd;
  CFunc m_GameEventAllowRemove;
  CFunc m_GameEventDenyAdd;
  CFunc m_GameEventDenyRemove;
  CFunc m_GameEventSetEnrichment;
  CCallback m_GameEventCallback;
  CFunc m_GameEventSetTransmitClientTime;
  CFunc m_GameEventSetTransmitTick;
  CFunc m_GameEventSetTransmitSystemTime;

  CCallback m_OnDeviceReset;

  CFunc m_DrawingConnect;
  CFunc m_DrawingPumpBegin;
  CFunc m_DrawingPumpEnd;
  CFunc m_DrawingBeginFrame;
  CFunc m_D3d9CreateVertexDecalaration;
  CFunc m_D3d9CreateIndexBuffer;
  CFunc m_D3d9CreateVertexBuffer;
  CFunc m_D3d9CreateTexture;
  CFunc m_D3d9CreateVertexShader;
  CFunc m_D3d9CreatePixelShader;
  CFunc m_D3d9UpdateTexture;
  CFunc m_D3d9SetViewport;
  CFunc m_D3d9SetRenderState;
  CFunc m_D3d9SetSamplerState;
  CFunc m_D3d9SetTexture;
  CFunc m_D3d9SetTextureStageState;
  CFunc m_D3d9SetTransform;
  CFunc m_D3d9SetIndices;
  CFunc m_D3d9SetStreamSource;
  CFunc m_D3d9SetStreamSourceFreq;
  CFunc m_D3d9SetVertexDeclaration;
  CFunc m_D3d9SetVertexShader;
  CFunc m_D3d9SetVertexShaderConstantB;
  CFunc m_D3d9SetVertexShaderConstantF;
  CFunc m_D3d9SetVertexShaderConstantI;
  CFunc m_D3d9SetPixelShader;
  CFunc m_D3d9SetPixelShaderConstantB;
  CFunc m_D3d9SetPixelShaderConstantF;
  CFunc m_D3d9SetPixelShaderConstantI;
  CFunc m_D3d9DrawPrimitive;
  CFunc m_D3d9DrawIndexedPrimitive;
  CFunc m_WaitForGpu;
  CFunc m_BeginCleanState;
  CFunc m_EndCleanState;
  CFunc m_CreateCefWindowTextureSharedHandle;
  CFunc m_CreateDrawingData;

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

    if (m_RenderPassCallback.IsValid()) {

      CefV8ValueList args;
      args.push_back(CefV8Value::CreateInt((int)pass));
      args.push_back(CreateAfxView(view));

      m_RenderPassCallback.ExecuteCallback(args);
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

    if (m_GameEventCallback.IsValid()) {
      CefV8ValueList args;
      args.push_back(objEvent);
      m_GameEventCallback.ExecuteCallback(args);
    }

    return true;
  }

  bool WriteGameEventSettings(bool delta) {
    if (!m_PipeServer->WriteBoolean(m_GameEventCallback.IsValid() ? true : false))
      return false;

    if (!m_GameEventCallback.IsValid())
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
    CCalcResult(CCalcCallbacksGuts* guts,
                const char* name,
                CCalcCallback* callback)
        : m_Guts(guts), m_Name(name), m_Callback(callback) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);

      m_Obj = CefV8Value::CreateObject(this, nullptr);
      m_Obj->SetValue("release", V8_ACCESS_CONTROL_DEFAULT,
                      V8_PROPERTY_ATTRIBUTE_NONE);

      m_Guts->Add(name, callback);
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

    CefRefPtr<CefV8Value> GetObj() { return m_Obj; }

    bool Set(const CefString& name,
             const CefRefPtr<CefV8Value> object,
             const CefRefPtr<CefV8Value> value,
             CefString& /*exception*/) override {
      return false;
    }

   private:
    CCalcCallbacksGuts* m_Guts;
    std::string m_Name;
    CefRefPtr<CCalcCallback> m_Callback;

    virtual ~CCalcResult() { ReleaseCalc(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    CefRefPtr<CefV8Value> m_Obj;
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

class CEngineInterop* CreateEngineInterop(
    CefRefPtr<CefBrowser> const& browser,
    CefRefPtr<CefFrame> const& frame,
    CefRefPtr<CefV8Context> const& context) {
  return new CEngineInteropImpl(browser, frame, context);
}

}  // namespace interop
}  // namespace advancedfx
