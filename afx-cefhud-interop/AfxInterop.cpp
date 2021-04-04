#include "AfxInterop.h"

#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>

#include <d3d11.h>
#include <d3dcompiler.h>
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


#define AFX_GOTO_ERROR    \
  {                       \
    errorLine = __LINE__; \
    goto error;           \
  }

#ifdef _DEBUG
#define AFX_PRINT_ERROR(name,errorLine)                               \
  TCHAR szError[2000];                                                \
  _sntprintf(szError, 2000, _T("ERROR in %s in line %i\n"), _T(name), \
             errorLine);                                              \
  MessageBox(NULL, szError, _T("AFX ERROR"), MB_OK|MB_ICONERROR);
#else
#define AFX_PRINT_ERROR(name,errorLine)
#endif

const char * g_szAlreadyReleased = "Already released.";
const char* g_szOutOfFlow = "Out of flow.";
const char* g_szConnectionError = "Connection error.";
const char* g_szInvalidArguments = "Invalid arguments.";
const char* g_szMemoryAllocationFailed = "Memory allocation failed.";

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
  //AfxPromise
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



void CPipeReader::ReadBytes(LPVOID bytes, DWORD offset, DWORD length) {
  while (0 < length) {
    DWORD bytesRead = 0;

    if (!(ReadFile(m_Handle, (unsigned char*)bytes + offset, length,
                   &bytesRead, NULL))) {
      DWORD lastError = GetLastError();
      //std::string code = std::to_string(lastError);
      //MessageBoxA(0, code.c_str(), "ReadBytes", MB_OK);
      throw CPipeException(lastError);
    }
    

    offset += bytesRead;
    length -= bytesRead;
  }
}

bool CPipeReader::ReadBoolean() {
  return 0 != ReadByte();
}

BYTE CPipeReader::ReadByte() {
  BYTE tmp;

  ReadBytes(&tmp, 0, (DWORD)sizeof(tmp));

  return tmp;
}

signed char CPipeReader::ReadSByte() {
  signed char tmp;

  ReadBytes(&tmp, 0, (DWORD)sizeof(tmp));

  return tmp;
}

INT16 CPipeReader::ReadInt16() {
  INT16 tmp;
  ReadBytes(&tmp, 0, (DWORD)sizeof(tmp));
  return tmp;
}

UINT32 CPipeReader::ReadUInt32() {
  UINT32 tmp;
  ReadBytes(&tmp, 0, (DWORD)sizeof(tmp));
  return tmp;
}

UINT32 CPipeReader::ReadCompressedUInt32() {
  BYTE tmp = ReadByte();
  if (tmp < 255) {
    return tmp;
  }

  return ReadUInt32();
}

INT32 CPipeReader::ReadInt32() {
  INT32 tmp;
  ReadBytes(&tmp, 0, (DWORD)sizeof(tmp));
  return tmp;
}

INT32 CPipeReader::ReadCompressedInt32() {
  signed char tmp = ReadSByte();
  if (tmp < 127) {
    return tmp;
  }
  return ReadInt32();
}

void CPipeReader::ReadStringUTF8(std::string& outValue) {
  UINT32 length = ReadCompressedUInt32();
  outValue.resize(length);
  ReadBytes(&outValue[0], 0, length);
}

HANDLE CPipeReader::ReadHandle() {
  DWORD value32;
  ReadBytes(&value32, 0, (DWORD)sizeof(value32));
  return ULongToHandle(value32);
}

UINT64 CPipeReader::ReadUInt64() {
  UINT64 tmp;
  ReadBytes(&tmp, 0, (DWORD)sizeof(tmp));
  return tmp;
}

FLOAT CPipeReader::ReadSingle() {
  FLOAT tmp;
  ReadBytes(&tmp, 0, (DWORD)sizeof(tmp));
  return tmp;
}

void CPipeWriter::WriteBytes(const LPVOID bytes, DWORD offset, DWORD length) {
  while (0 < length) {
    DWORD bytesWritten = 0;

    if (!(WriteFile(m_Handle, (unsigned char*)bytes + offset, length,
                    &bytesWritten, NULL))) {
      DWORD lastError = GetLastError();
      //std::string code = std::to_string(lastError);
      //MessageBoxA(0, code.c_str(), "WriteBytes", MB_OK);
      throw CPipeException(lastError);
    }

    offset += bytesWritten;
    length -= bytesWritten;
  }
}

void CPipeWriter::Flush() {
  if (!FlushFileBuffers(m_Handle))
    throw CPipeException(GetLastError());
}

void CPipeWriter::WriteBoolean(bool value) {
  BYTE tmp = value ? 1 : 0;
  WriteBytes(&tmp, 0, sizeof(tmp));
}

void CPipeWriter::WriteByte(BYTE value) {
  WriteBytes(&value, 0, sizeof(value));
}

void CPipeWriter::WriteSByte(signed char value) {
  WriteBytes(&value, 0, sizeof(value));
}

void CPipeWriter::WriteUInt32(UINT32 value) {
  WriteBytes(&value, 0, sizeof(value));
}

void CPipeWriter::WriteCompressedUInt32(UINT32 value) {
  if (0 <= value && value <= 255 - 1) {
    WriteByte((BYTE)value);
  } else {
    WriteByte(255);
    WriteUInt32(value);
  }
}

void CPipeWriter::WriteInt32(INT32 value) {
  WriteBytes(&value, 0, sizeof(value));
}

void CPipeWriter::WriteCompressedInt32(INT32 value) {
  if (-128 <= value && value <= 127 - 1) {
    WriteSByte((signed char)value);
  } else {
    WriteSByte(127);
    WriteInt32(value);
  }
}

void CPipeWriter::WriteUInt64(UINT64 value) {
  WriteBytes(&value, 0, sizeof(value));
}

void CPipeWriter::WriteHandle(HANDLE value) {
  DWORD value32 = HandleToULong(value);
  WriteBytes(&value32, 0, sizeof(value32));
}

void CPipeWriter::WriteSingle(FLOAT value) {
  WriteBytes(&value, 0, sizeof(value));
}

void CPipeWriter::WriteStringUTF8(const std::string& value) {
  UINT32 length = (UINT32)value.length();
  WriteCompressedUInt32(length);
  WriteBytes((LPVOID)value.c_str(), 0, length);
}

// CPipeClient /////////////////////////////////////////////////////////////////

void CPipeClient::OpenPipe(const char* pipeName, int timeOut) {
  ClosePipe();

  for(int i = 0; i < 2; ++i)
  {
    m_Handle = CreateFileA(pipeName, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (m_Handle != INVALID_HANDLE_VALUE)
      return;

    DWORD lastError = GetLastError();

    if (lastError != ERROR_PIPE_BUSY)
    {
      throw CWinApiException("CPipeClient::OpenPipe: CreateFileA failed.", lastError);
    }
    else if (FAILED(WaitNamedPipeA(pipeName, timeOut)))
    {
      throw CWinApiException("CPipeClient::OpenPipe: WaitNamedPipe failed", GetLastError());
    }
  }

  throw "CPipeClient::OpenPipe failed.";
}

void CPipeClient::ClosePipe() {
  if(INVALID_HANDLE_VALUE != m_Handle) {
    if(!CloseHandle(m_Handle)) {
      throw CWinApiException("CPipeClient::ClosePipe: CloseHandle failed.", GetLastError());
    }
    m_Handle = INVALID_HANDLE_VALUE;
  }
}


// CPipeServer /////////////////////////////////////////////////////////////////

void CPipeServer::WaitForConnection(const char* pipeName,
                        unsigned int outBufSize,
                        unsigned int inBufSize,
                        int timeOut) {

  // https://docs.microsoft.com/en-us/windows/win32/ipc/multithreaded-pipe-server

  BOOL fConnected = FALSE;
  DWORD dwThreadId = 0;
  HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;

 hPipe = CreateNamedPipeA(pipeName, PIPE_ACCESS_DUPLEX,
                           PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |
                               PIPE_REJECT_REMOTE_CLIENTS,
                           PIPE_UNLIMITED_INSTANCES, outBufSize, inBufSize,
                           timeOut, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
      throw CWinApiException("CreateNamedPipe failed", GetLastError());
    }

    fConnected = ConnectNamedPipe(hPipe, NULL)
                     ? TRUE
                     : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (fConnected) {
      if (CPipeServerConnectionThread* connectionThread = OnNewConnection(hPipe)) {
        // Create a thread for this client.
        hThread = CreateThread(NULL,            // no security attribute
                               0,               // default stack size
                               InstanceThread,  // thread proc
                               (LPVOID)connectionThread,  // thread parameter
                               0,                         // not suspended
                               &dwThreadId);              // returns thread ID

        if (hThread == NULL) {
          DWORD lastError = GetLastError();
          CloseHandle(hPipe);
          delete connectionThread;
          throw CWinApiException("CreateThread failed", lastError);
        } else
          CloseHandle(hThread);
      } else {
        CloseHandle(hPipe);
        throw "OnNewConnection NULL";
      }
    } else
      // The client could not connect, so close the pipe.
      CloseHandle(hPipe);
}

 
DWORD WINAPI CPipeServer::InstanceThread(LPVOID lpvParam) {
  try {
    if (lpvParam == NULL) {
      return (DWORD)-1;
    }

    HANDLE hPipe = ((CPipeServerConnectionThread*)lpvParam)->GetHandle();

    ((CPipeServerConnectionThread*)lpvParam)->HandleConnection();

    // Flush the pipe to allow the client to read the pipe's contents
    // before disconnecting. Then disconnect the pipe, and close the
    // handle to this pipe instance. 

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
  }
  catch(...) {
    delete ((CPipeServerConnectionThread*)lpvParam);
    return (DWORD)-2;
  }

  delete((CPipeServerConnectionThread*)lpvParam);
  return 1;
}
 
class CNamedPipeServer {
 public:
  enum State { State_Error, State_Waiting, State_Connected };

 private:
  HANDLE m_PipeHandle;

  OVERLAPPED m_Overlapped = {};
  //OVERLAPPED m_OverlappedWrite = {};

  State m_State = State_Error;

  const DWORD m_ReadBufferSize = 4096;
  const DWORD m_WriteBufferSize = 4096;

  DWORD m_ReadTimeoutMs;
  DWORD m_WriteTimeoutMs;

 public:
  CNamedPipeServer(const char* pipeName,
                   DWORD readTimeOutMs = 5000,
                   DWORD writeTimeOutMs = 5000)
      : m_ReadTimeoutMs(readTimeOutMs), m_WriteTimeoutMs(writeTimeOutMs) {
    m_Overlapped.hEvent = CreateEventA(NULL, true, true, NULL);
    //m_OverlappedWrite.hEvent = CreateEventA(NULL, true, true, NULL);

    std::string strPipeName("\\\\.\\pipe\\");
    strPipeName.append(pipeName);

    m_PipeHandle = CreateNamedPipeA(
        strPipeName.c_str(),
        PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_READMODE_BYTE | PIPE_TYPE_BYTE | PIPE_WAIT |
            PIPE_REJECT_REMOTE_CLIENTS,
        1, m_ReadBufferSize, m_WriteBufferSize, 5000, NULL);

    if (INVALID_HANDLE_VALUE != m_Overlapped.hEvent &&
        //INVALID_HANDLE_VALUE != m_OverlappedWrite.hEvent &&
        INVALID_HANDLE_VALUE != m_PipeHandle &&
        FALSE == ConnectNamedPipe(m_PipeHandle, &m_Overlapped)) {
      switch (GetLastError()) {
        case ERROR_IO_PENDING:
          m_State = State_Waiting;
          break;
        case ERROR_PIPE_CONNECTED:
          m_State = State_Connected;
          SetEvent(m_Overlapped.hEvent);
          break;
      }
    }
  }

  ~CNamedPipeServer() {
    if (INVALID_HANDLE_VALUE != m_PipeHandle)
      CloseHandle(m_PipeHandle);
    //if (INVALID_HANDLE_VALUE != m_OverlappedWrite.hEvent)
    //  CloseHandle(m_OverlappedWrite.hEvent);
    if (INVALID_HANDLE_VALUE != m_Overlapped.hEvent)
      CloseHandle(m_Overlapped.hEvent);
  }

  State Connect() {
    if (State_Waiting == m_State) {
      DWORD waitResult = WaitForSingleObject(m_Overlapped.hEvent, 0);

      if (WAIT_OBJECT_0 == waitResult) {
        DWORD cb;

        if (!GetOverlappedResult(m_PipeHandle, &m_Overlapped, &cb, FALSE))
          m_State = State_Error;
        else
          m_State = State_Connected;
      }
    }

    return m_State;
  }

  bool ReadBytes(LPVOID bytes, DWORD offset, DWORD length) {
    while (true) {
      if (0 == length)
        break;

      DWORD bytesRead = 0;

      if (!(ReadFile(m_PipeHandle, (unsigned char*)bytes + offset, length,
                     &bytesRead, &m_Overlapped) &&
            0 != bytesRead)) {

        DWORD lastError = GetLastError();

        bool bContinue = false;

        switch (lastError) {
          case ERROR_IO_PENDING: {
            bool completed = false;
            while (!completed) {
              DWORD result =
                  WaitForSingleObject(m_Overlapped.hEvent, m_ReadTimeoutMs);
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
          } break;
          case ERROR_INVALID_USER_BUFFER:
          case ERROR_NOT_ENOUGH_MEMORY:
              bContinue = true;
              break;
          default:
            return false;
        }

        if (bContinue) continue;

        if (!GetOverlappedResult(m_PipeHandle, &m_Overlapped, &bytesRead,
                                 FALSE))
          return false;
      }

      offset += bytesRead;
      length -= bytesRead;
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
      if (0 == length)
        break;

      DWORD bytesWritten = 0;

      if (!WriteFile(m_PipeHandle, (unsigned char*)bytes +offset,
                      length, &bytesWritten, &m_Overlapped)) {
        DWORD lastError = GetLastError();

        bool bContinue = false;

        switch (lastError) {
          case ERROR_IO_PENDING: {
            bool completed = false;
            while (!completed) {
              DWORD result =
                  WaitForSingleObject(m_Overlapped.hEvent,
                                                 m_ReadTimeoutMs);
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
          } break;
          case ERROR_INVALID_USER_BUFFER:
          case ERROR_NOT_ENOUGH_MEMORY:
            bContinue = true;
            break;
          default:
            return false;
        }

        if (bContinue)
          continue;

        if (!GetOverlappedResult(m_PipeHandle, &m_Overlapped,
                                 &bytesWritten, FALSE))
          return false;
      }

      offset += bytesWritten;
      length -= bytesWritten;
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


class CAfxTask : public CefTask {
 public:
  typedef std::function<void(void)> fp_t;


  explicit CAfxTask(const fp_t& op) : m_Op(op) {
  
  }

  explicit CAfxTask(const fp_t&& op) : m_Op(std::move(op)) {}

  // CefTask method
  virtual void Execute() OVERRIDE {
    m_Op();
  }

 private:
  fp_t m_Op;

  IMPLEMENT_REFCOUNTING(CAfxTask);
  DISALLOW_COPY_AND_ASSIGN(CAfxTask);
};

template<class...>struct types{using type=types;};
template<class T>struct tag{using type=T;};
template<class Tag>using type_t=typename Tag::type;

class CAfxValue : public CefBaseRefCounted {
 public:
  enum class ValueType { Invalid, Array, Object, Bool, Int, UInt, Double, V8Function, String };

  static CefRefPtr<CAfxValue> FromV8Value(CefRefPtr<CefV8Value> value) {
    return new CAfxValue(value);
  }

  static CefRefPtr<CAfxValue> CreateArray(int size) {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_ValueType = ValueType::Array;
    result->m_Elements.resize(size);
    return result;
  }

  static CefRefPtr<CAfxValue> CreateObject() {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_ValueType = ValueType::Object;
    return result;
  }

  static CefRefPtr<CAfxValue> CreateBool(bool value) {
   CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_Value.Bool = value;
    result->m_ValueType = ValueType::Bool;
    return result;
  }

  static CefRefPtr<CAfxValue> CreateInt(int value) {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_Value.Int = value;
    result->m_ValueType = ValueType::Int;
    return result;
  }

  static CefRefPtr<CAfxValue> CreateUInt(unsigned int value) {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_Value.UInt = value;
    result->m_ValueType = ValueType::UInt;
    return result;
  }

  static CefRefPtr<CAfxValue> CreateDouble(double value) {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_Value.Double = value;
    result->m_ValueType = ValueType::Double;
    return result;
  }

  static CefRefPtr<CAfxValue> CreateV8Function(CefRefPtr<CefV8Value> value) {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_Function = value;
    result->m_ValueType = ValueType::V8Function;
    return result;
  }

  static CefRefPtr<CAfxValue> CreateString(const std::string & value) {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_String = value;
    result->m_ValueType = ValueType::String;
    return result;
  }

  CAfxValue() : m_ValueType(ValueType::Invalid), m_Value({false}) {
  }

  CAfxValue(CefRefPtr<CefV8Value> value)
      : m_ValueType(ValueType::Invalid), m_Value({false}) {
    if (value) {
      if (value->IsBool()) {
        m_ValueType = ValueType::Int;
        m_Value.Double = value->GetBoolValue();
      } else if (value->IsInt()) {
        m_ValueType = ValueType::Int;
        m_Value.Double = value->GetIntValue();
      } else if (value->IsUInt()) {
        m_ValueType = ValueType::UInt;
        m_Value.UInt = value->GetUIntValue();
      } else if (value->IsString()) {
        m_ValueType = ValueType::String;
        m_String = value->GetStringValue().ToString();
      } else if (value->IsDouble()) {
        m_ValueType = ValueType::Double;
        m_Value.Double = value->GetDoubleValue();
      } else if (value->IsFunction()) {
        m_ValueType = ValueType::V8Function;
        m_Function = value;
      } else if (value->IsArray()) {
        m_ValueType = ValueType::Array;
        for (int i = 0; i < value->GetArrayLength(); ++i) {
          m_Elements.emplace_back(FromV8Value(value->GetValue(i)));
        }
      } else if (value->IsObject()) {
        m_ValueType = ValueType::Object;
        std::vector<CefString> keys;
        if (value->GetKeys(keys)) {
          int i = 0;
          for (auto it = keys.begin(); it != keys.end(); ++it) {

            m_Children.emplace(it->ToString(),
                               FromV8Value(value->GetValue(*it)));
            ++i;
          }
        }
      }
    }
  }

  void AddChild(const char* name, CefRefPtr<CAfxValue> value){
      m_Children[name] = value;
  }

  CefRefPtr<CAfxValue> GetChild(const char* name) {
    if (m_ValueType == ValueType::Object) {
      auto it = m_Children.find(name);
      if (it != m_Children.end())
        return it->second;
    }
    return nullptr;
  }

  void SetChild(const char* name, CefRefPtr<CAfxValue> value) {      
      m_Children[name] = value;
  }

  int GetArraySize() {
    if (m_ValueType == ValueType::Array)
      return (int)m_Elements.size();

      return 0;
  }

  CefRefPtr<CAfxValue> GetArrayElement(int index) {

    if (m_ValueType == ValueType::Array && 0 <= index && index < m_Elements.size())
        return m_Elements[index];

    return nullptr;
  }

  void SetArrayElement(int index, CefRefPtr<CAfxValue> value) {m_Elements[index] = value;
  }

  CefRefPtr<CefV8Value> GetFunction() {
      return m_Function;
  }

    bool IsString() { return m_ValueType == ValueType::String; }


  bool IsArray() { return m_ValueType == ValueType::Array; }

  bool IsObject() { return m_ValueType == ValueType::Object; }

  bool IsV8Function() { return m_ValueType == ValueType::V8Function; }

  bool IsBool() {
    return m_ValueType == ValueType::Bool;
  }

  bool IsUInt() {
    return m_ValueType == ValueType::Int || m_ValueType == ValueType::UInt;
  }

  bool IsInt() {
      return m_ValueType == ValueType::Int || m_ValueType == ValueType::UInt;
  }

  bool IsDouble() {
    return m_ValueType == ValueType::Int || m_ValueType == ValueType::UInt || m_ValueType == ValueType::Double;
  }

  CefRefPtr<CefV8Value> GetV8Function() {
    if (m_ValueType == ValueType::V8Function)
      return m_Function;

    return nullptr;
  }

  int GetBool() {
    if (m_ValueType == ValueType::Bool)
      return m_Value.Bool;

    return false;
  }

  unsigned int GetUInt() {
    if (m_ValueType == ValueType::Int)
      return (unsigned int)m_Value.Int;
    if (m_ValueType == ValueType::UInt)
      return m_Value.UInt;
    return 0;
  }

  int GetInt() {
    if (m_ValueType == ValueType::Int)
      return m_Value.Int;
    if (m_ValueType == ValueType::UInt)
      return (int)m_Value.UInt;
    return 0;
  }

  double GetDouble() {
    if (m_ValueType == ValueType::Int)
      return (double)m_Value.Int;
    if (m_ValueType == ValueType::UInt)
      return (double)m_Value.UInt;
    if (m_ValueType == ValueType::Double)
      return m_Value.Double;

    return 0;
  }

  std::string GetString() {
    if (m_ValueType == ValueType::String)
      return m_String;
    return "";
  }

  CefRefPtr<CefV8Value> ToV8Value() {
      switch (m_ValueType) {
        case ValueType::Bool:
          return CefV8Value::CreateBool(m_Value.Bool);
        case ValueType::Int:
          return CefV8Value::CreateInt(m_Value.Int);
        case ValueType::UInt:
          return CefV8Value::CreateUInt(m_Value.UInt);
        case ValueType::Double:
          return CefV8Value::CreateDouble(m_Value.Double);
        case ValueType::V8Function:
          return m_Function;
        case ValueType::Array: {
          CefRefPtr<CefV8Value> arr = CefV8Value::CreateArray((int)m_Elements.size());
          for(int i = 0; i < (int)m_Elements.size(); ++i) {
            arr->SetValue(i, m_Elements[i]->ToV8Value());
          }
          return arr;
        }
        case ValueType::Object: {

          CefRefPtr<CefV8Value> obj =
              CefV8Value::CreateObject(nullptr, nullptr);

          for (auto it = m_Children.begin(); it != m_Children.end(); ++it) {
            obj->SetValue(CefString(it->first), it->second->ToV8Value(),
                          V8_PROPERTY_ATTRIBUTE_NONE);
          }

          return obj;
        }
        case ValueType::String:
          return CefV8Value::CreateString(m_String);
      }

      return nullptr;
  }

      private :
  ValueType m_ValueType;
  std::map<std::string, CefRefPtr<CAfxValue>> m_Children;
  std::vector<CefRefPtr<CAfxValue>> m_Elements;
  union Value_u {
    bool Bool;
    int Int;
    unsigned int UInt;
    double Double;
  } m_Value;
  CefRefPtr<CefV8Value> m_Function;
  std::string m_String;

  IMPLEMENT_REFCOUNTING(CAfxValue);
  DISALLOW_COPY_AND_ASSIGN(CAfxValue);
};

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
  CAfxCallback() {
  }

  CAfxCallback(CefRefPtr<CefV8Value> func)
      : m_CallbackFunc(func) {}


  CefRefPtr<CefV8Value> GetFunc() {
      return m_CallbackFunc;
  }

  void SetFunc(CefRefPtr<CefV8Value> value) {
      m_CallbackFunc = value;
  }

  bool IsValid() { return m_CallbackFunc != nullptr; }

  CefRefPtr<CefV8Value> ExecuteCallback(const CefV8ValueList& arguments) {
    return m_CallbackFunc->ExecuteFunctionWithContext(CefV8Context::GetCurrentContext(), NULL,
                                                      arguments);
  }

 private:
  CefRefPtr<CefV8Value> m_CallbackFunc;

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

    CefRefPtr<CAfxCallback> AddCallback(const char* name) {
    {
      CefRefPtr<CAfxCallback> callback = new CAfxCallback();

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


class CAfxUserData : public CefBaseRefCounted {
 public:
  CAfxUserData(AfxUserDataType userDataType, void* userData)
      : m_UserDataType(userDataType), m_UserData(userData) {}

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
  if (nullptr == value || !value->IsObject())
    return nullptr;
  CefRefPtr<CefBaseRefCounted> user_data = value->GetUserData();
  if (nullptr == user_data)
    return nullptr;
  CAfxUserData* afxUserData = static_cast<CAfxUserData*>(user_data.get());
  if (type != afxUserData->GetUserDataType())
    return nullptr;
  return (T*)afxUserData->GetUserData();
}

class CAfxInteropImpl : public CefBaseRefCounted {
  public:
  UINT64 GetIndex() const { return (UINT64)this; }
};

class CAfxHandle : public CAfxInteropImpl {
  IMPLEMENT_REFCOUNTING(CAfxHandle);

  public:
      static HANDLE ToHandle(unsigned int lo, unsigned int hi) {
    return (void*)((unsigned __int64)lo | ((unsigned __int64)hi << 32));
  }

  static CefRefPtr<CefV8Value> Create(HANDLE handle, CefRefPtr<CAfxHandle>* out = nullptr) {
    CefRefPtr<CAfxHandle> val = new CAfxHandle(handle);

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
    afxObject->AddGetter(
        "invalid", [val](const CefString& name, const CefRefPtr<CefV8Value> object,
                    CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateBool(val->GetHandle() ==
                                          INVALID_HANDLE_VALUE);
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

  CAfxHandle(unsigned int lo, unsigned int hi) : CAfxInteropImpl(), m_Handle(ToHandle(lo,hi)) {}

  HANDLE GetHandle() { return m_Handle;
  }

  void SetHandle(HANDLE value) {
      m_Handle = value;
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

  virtual bool Connection(DWORD readTimeOutMs = 15000, DWORD writeTimeOutMs = 15000) {
    if (nullptr == m_PipeServer) {
      std::string pipeName(m_PipeName);

      m_PipeServer = new CNamedPipeServer(pipeName.c_str(), readTimeOutMs, writeTimeOutMs);
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
  CThreadedQueue m_PipeQueue;

  virtual ~CAfxInterop() { Close(); }

  virtual bool OnNewConnection() { return true; }

  virtual bool OnConnection() { return true; }

  virtual void OnClosing() {}

 private:
  bool m_Connecting = false;
};

/*
 class CAfxPromise : public CefBaseRefCounted {
  IMPLEMENT_REFCOUNTING(CAfxPromise);
 public:
  CAfxPromise(CefRefPtr<CefV8Value> fnResolve,
              CefRefPtr<CefV8Value> fnReject)
      : m_FnResolve(fnResolve), m_FnReject(fnReject) {}

  unsigned int GetLo() {
    return (unsigned int)((unsigned __int64)this & 0xffffffff);
  }

  unsigned int GetHi() {
    return (unsigned int)(((unsigned __int64)this >> 32) & 0xffffffff);
  }

  CefRefPtr<CefV8Value> m_FnResolve;
  CefRefPtr<CefV8Value> m_FnReject;

  void DoResolve(const CefV8ValueList& arguments) {
    // TODO: Handle exceptions and promises being returned.
    if (nullptr != m_FnResolve && m_FnResolve->IsFunction())
      m_FnResolve->ExecuteFunctionWithContext(CefV8Context::GetCurrentContext(), NULL, arguments);
  }

  void DoReject(const CefV8ValueList& arguments) {
    // TODO: Handle exceptions and promises being returned.
    if (nullptr != m_FnReject && m_FnReject->IsFunction())
      m_FnReject->ExecuteFunctionWithContext(CefV8Context::GetCurrentContext(), NULL, arguments);
  }
};

class CAfxPromises : public CefBaseRefCounted {
  IMPLEMENT_REFCOUNTING(CAfxPromises);
 public:
  void FreeAll() {
    while (!m_Promises.empty()) {
      auto it = m_Promises.begin();
      (*it)->Release();
      m_Promises.erase(it);
    }
  }

  CefRefPtr<CefV8Value> Create(CefRefPtr<CefV8Value> fnResolve,
              CefRefPtr<CefV8Value> fnReject, CefRefPtr<CefFrame> frame, CefRefPtr<CAfxPromise> * out = nullptr) {
    CefRefPtr<CAfxPromise> val =
        new CAfxPromise(fnResolve, fnReject);
    val->AddRef();
    m_Promises.insert(val);

    CefRefPtr<CAfxObject> afxObject = new CAfxObject();
    afxObject->AddFunction(
        "cancel", [val,frame](
                      const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval, CefString& exception) {


          auto message = CefProcessMessage::Create("afx-cancel");
          auto args = message->GetArgumentList();
          args->SetSize(2);
          args->SetInt(0, (int)val->GetLo());
          args->SetInt(1, (int)val->GetHi());

          frame->SendProcessMessage(PID_BROWSER, message);
          return true;          

          return true;
        });


    if (out)
      *out = val;

    afxObject->SetUserData(
        new CAfxUserData(AfxUserDataType::AfxPromise, val.get()));
    return afxObject->GetObj();
  }

  void Resolve(unsigned int lo, unsigned int hi, const CefV8ValueList & arguments) {
    CAfxPromise* val =
        (CAfxPromise*)((unsigned __int64)lo | ((unsigned __int64)hi << 32));

    auto it = m_Promises.find(val);
    if (it != m_Promises.end()) {
      m_Promises.erase(it);
      val->DoResolve(arguments);
      val->Release();
    }
  }

  void Reject(unsigned int lo,
                     unsigned int hi,
              const CefV8ValueList& arguments) {
    CAfxPromise* val =
        (CAfxPromise*)((unsigned __int64)lo | ((unsigned __int64)hi << 32));

    auto it = m_Promises.find(val);
    if (it != m_Promises.end()) {
      m_Promises.erase(it);
      val->DoReject(arguments);
      val->Release();
    }
  }

  protected:
    ~CAfxPromises() {
        FreeAll();
    }

 private:

  std::set<CAfxPromise*> m_Promises;
};
*/
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

    self->m_Context = context;

    afxObject->AddGetter(
        "id",
        [browser](const CefString& name, const CefRefPtr<CefV8Value> object,
                  CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateInt(browser->GetIdentifier());
          return true;
        });

    afxObject->AddFunction(
        "init", [obj = afxObject->GetObj(), argStr](const CefString& name,
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

              argInitFn->ExecuteFunctionWithContext(CefV8Context::GetCurrentContext(), NULL, args);

              return true;
            }
          }

          return true;
        });

    afxObject->AddFunction(
        "sendMessage",
        [frame](const CefString& name, CefRefPtr<CefV8Value> object,
                const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                CefString& exceptionoverride) {
          if (arguments.size() == 2) {
            auto argId = arguments[0];
            auto argStr = arguments[1];

            if (argId->IsInt() && argStr->IsString()) {
              auto message = CefProcessMessage::Create("afx-message");
              auto args = message->GetArgumentList();
              args->SetInt(0, argId->GetIntValue());
              args->SetString(1, argStr->GetStringValue());

              frame->SendProcessMessage(PID_BROWSER, message);

              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    self->m_OnMessage = afxObject->AddCallback("onMessage");

    self->m_OnError = afxObject->AddCallback("onError");

    self->m_OnCefFrameRendered = afxObject->AddCallback("onCefFrameRendered");
    
    afxObject->AddFunction("setPipeName", [self](const CefString& name,
                                              CefRefPtr<CefV8Value> object,
                                              const CefV8ValueList& arguments,
                                              CefRefPtr<CefV8Value>& retval,
                                              CefString& exceptionoverride) {
      if (1 <= arguments.size() && arguments[0]->IsString()) {
        self->m_PipeQueue.Queue(
            [self, pipeName = arguments[0]->GetStringValue().ToString()]() {
              self->SetPipeName(pipeName.c_str());
            });
        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });

    self->m_OnDeviceLost = afxObject->AddCallback("onDeviceLost");

    self->m_OnDeviceReset = afxObject->AddCallback("onDeviceReset");

    afxObject->AddFunction("connect", [self](const CefString& name,
                                             CefRefPtr<CefV8Value> object,
                                             const CefV8ValueList& arguments,
                                             CefRefPtr<CefV8Value>& retval,
                                             CefString& exceptionoverride) {
      if (2 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction()) {
        self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                 fn_reject = arguments[1]]() {
          if (self->Connection(12000, 12000)) {
            CefPostTask(TID_RENDERER,
                        new CAfxTask([ctx = self->m_Context, fn_resolve, connected = self->Connected()]() {
                          ctx->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateBool(connected));
                          fn_resolve->ExecuteFunction(NULL, args);
                          ctx->Exit();
                        }));
          } else {
            CefPostTask(TID_RENDERER,
                        new CAfxTask([ctx = self->m_Context, fn_reject]() {
                          ctx->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          ctx->Exit();
                        }));
          }
        });
        return true;
      }
      exceptionoverride = g_szInvalidArguments;
      return true;
    });
    afxObject->AddFunction(
        "close",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {

          self->m_PipeQueue.Queue([self]() { self->Close(); });
          return true;
        });
    afxObject->AddFunction("pumpBegin", [self](const CefString& name,
                                               CefRefPtr<CefV8Value> object,
                                               const CefV8ValueList& arguments,
                                               CefRefPtr<CefV8Value>& retval,
                                               CefString& exceptionoverride) {
      if (4 <= arguments.size() && arguments[0]->IsInt() &&
          arguments[1]->IsUInt() && arguments[2]->IsFunction() &&
          arguments[3]->IsFunction()) {
        self->m_PipeQueue.Queue([self, frameCount = arguments[0]->GetIntValue(),
                                 pass = arguments[1]->GetIntValue(),
                                 fn_resolve = arguments[2],
                                 fn_reject = arguments[3]]() {
          if (self->Connected() && self->DoPumpBegin(frameCount, pass)) {
            CefPostTask(TID_RENDERER,
                        new CAfxTask([ctx = self->m_Context, fn_resolve,
                                      result = self->m_InFlow]() {
                          ctx->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateBool(result));
                          fn_resolve->ExecuteFunction(NULL, args);
                          ctx->Exit();
                        }));
          } else {
            CefPostTask(TID_RENDERER,
                        new CAfxTask([ctx = self->m_Context, fn_reject]() {
                          ctx->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          ctx->Exit();
                        }));
          }
        });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });

    afxObject->AddFunction("pumpEnd", [self](const CefString& name,
                                             CefRefPtr<CefV8Value> object,
                                             const CefV8ValueList& arguments,
                                             CefRefPtr<CefV8Value>& retval,
                                             CefString& exceptionoverride) {

     if (2 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction()) {
        self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                 fn_reject = arguments[1]]() {
            if (!self->m_InFlow)
              goto error;

            if (!self->m_PipeServer->WriteUInt32(
                    (UINT32)DrawingReply::Finished))
              goto error;

            if (!self->m_PipeServer->Flush())
              goto error;


            CefPostTask(TID_RENDERER,
                        new CAfxTask([ctx = self->m_Context, fn_resolve]() {
                          ctx->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          ctx->Exit();
                        }));
            return;

          error:
            self->Close();

            CefPostTask(TID_RENDERER,
                     new CAfxTask([ctx = self->m_Context, fn_reject]() {
                       ctx->Enter();
                       fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                       ctx->Exit();
                     }));
        });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
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

              auto message = CefProcessMessage::Create("afx-resize");
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

/*
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
              self->Close();
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9CreateVertexDeclaration", __LINE__)
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
              if (NULL == drawingHandle->GetHandle()) {
                // readback.
                if (!self->m_PipeServer->Flush())
                  goto error;
                HANDLE tmpHandle;
                if (!self->m_PipeServer->ReadHandle(tmpHandle))
                  goto error;
                drawingHandle->SetHandle(tmpHandle);
              }
            }

            return true;

          error:
            self->Close();
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9CreateIndexBuffer",
                            __LINE__)
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
              if (NULL == drawingHandle->GetHandle()) {
                // readback.
                if (!self->m_PipeServer->Flush())
                  goto error;
                HANDLE tmpHandle;
                if (!self->m_PipeServer->ReadHandle(tmpHandle))
                  goto error;
                drawingHandle->SetHandle(tmpHandle);
              }
            }

            return true;

          error:
            self->Close();
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9CreateVertexBuffer", __LINE__)
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
              if (NULL == drawingHandle->GetHandle()) {
                // readback.
                if (!self->m_PipeServer->Flush())
                  goto error;
                HANDLE tmpHandle;
                if (!self->m_PipeServer->ReadHandle(tmpHandle))
                  goto error;
                drawingHandle->SetHandle(tmpHandle);
              }
            }

            return true;

          error:
            self->Close();
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9CreateTexture", __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9CreateVertexShader", __LINE__)

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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9CreatePixelShader", __LINE__)

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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9UpdateTexture", __LINE__)

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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetViewport", __LINE__)

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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetViewport", __LINE__)

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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetRenderState", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetSamplerState", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetTexture", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetTextureStageState", __LINE__)
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
                AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetTransform",
                                __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetTransform", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetIndices", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetStreamSource", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetStreamSourceFreq", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetVertexDeclaration", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetVertexShader", __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetVertexShaderConstantB", __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetVertexShaderConstantF",
                              __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetVertexShaderConstantI",
                              __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetPixelShader",
                            __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetPixelShaderConstantB", __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetPixelShaderConstantF",
                              __LINE__)
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
              AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9SetPixelShaderConstantI",
                              __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9DrawPrimitive",
                            __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.d3d9DrawIndexedPrimitive", __LINE__)
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
            if (!self->m_PipeServer->Flush())
              goto error;

            bool waited;
            if (!self->m_PipeServer->ReadBoolean(waited)
                || !waited // this currently has to be always true.
            )
                goto error;

            return true;
          error:
            self->Close();
            AFX_PRINT_ERROR("CDrawingInteropImpl.waitForClientGpu", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.beginCleanState", __LINE__)
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
            AFX_PRINT_ERROR("CDrawingInteropImpl.endCleanState", __LINE__)
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

            exceptionoverride = g_szMemoryAllocationFailed;
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
*/
    afxObject->AddFunction(
        "renderCefFrame",
        [self, frame](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          auto message = CefProcessMessage::Create("afx-render");

          frame->SendProcessMessage(PID_BROWSER, message);

          return true;
        });
    afxObject->AddFunction(
        "d3dCompile2",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
           const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          if (10 == arguments.size() && (arguments[1]->IsNull() || arguments[1]->IsString()) &&
              arguments[3]->IsNull() && (arguments[4]->IsNull() || arguments[4]->IsString()) &&
              arguments[5]->IsString() && arguments[6]->IsUInt() &&
              arguments[7]->IsUInt() && arguments[8]->IsUInt()) {

            auto srcData = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(arguments[0]);
            auto defines = GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(
                arguments[2]);
            auto target = arguments[5]->GetStringValue();
            auto flags1 = arguments[6]->GetUIntValue();
            auto flags2 = arguments[7]->GetUIntValue();
            auto secondaryDataFlags = arguments[8]->GetUIntValue();
            auto secondaryData =
                GetAfxUserData<AfxUserDataType::AfxData, CAfxData>(
                    arguments[9]);

            ID3DBlob* pCode = nullptr;
            ID3DBlob* pErrorMsgs = nullptr;

            HRESULT result = D3DCompile2(
                srcData ? srcData->GetData() : NULL,
                srcData ? srcData->GetSize() : 0,
                // srcName:
                arguments[1]->IsNull()
                    ? NULL
                    : arguments[1]->GetStringValue().ToString().c_str(),
                defines ? (D3D_SHADER_MACRO*)defines->GetData() : NULL, NULL,
                // entryPoint:
                arguments[4]->IsNull()
                    ? NULL
                    : arguments[4]->GetStringValue().ToString().c_str(),
                target.ToString().c_str(),
                flags1, flags2, secondaryDataFlags,
                secondaryData ? secondaryData->GetData() : NULL,
                secondaryData ? secondaryData->GetSize() : 0, &pCode,
                &pErrorMsgs);

            retval = CefV8Value::CreateObject(nullptr, nullptr);

            retval->SetValue("hResult", CefV8Value::CreateUInt(result),
                              V8_PROPERTY_ATTRIBUTE_NONE);

            if (NULL == pCode)
              retval->SetValue("code", CefV8Value::CreateNull(),
                               V8_PROPERTY_ATTRIBUTE_NONE);
            else {

              size_t size = pCode->GetBufferSize();
              void* pData = malloc(size);
              if (nullptr == pData) {
                exceptionoverride = g_szMemoryAllocationFailed;
                return true;
              }
              memcpy(pData, pCode->GetBufferPointer(), size);

              retval->SetValue("code", CAfxData::Create((int)size,pData),
                               V8_PROPERTY_ATTRIBUTE_NONE);

              pCode->Release();
            }

            if (NULL == pErrorMsgs)
              retval->SetValue("errorMsgs", CefV8Value::CreateNull(),
                               V8_PROPERTY_ATTRIBUTE_NONE);
            else {
              size_t size = pErrorMsgs->GetBufferSize();
              void* pData = malloc(size);
              if (nullptr == pData) {
                exceptionoverride = g_szMemoryAllocationFailed;
                return true;
              }
              memcpy(pData, pErrorMsgs->GetBufferPointer(), size);

              retval->SetValue("errorMsgs", CAfxData::Create((int)size, pData),
                               V8_PROPERTY_ATTRIBUTE_NONE);

              pErrorMsgs->Release();
            }

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "createNullHandle",
        [self, frame](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          retval = CAfxHandle::Create(NULL);
          return true;
        });
    afxObject->AddFunction(
        "createInvalidHandle",
        [self, frame](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          retval = CAfxHandle::Create(INVALID_HANDLE_VALUE);
          return true;
        });
    //

    if (out)
      *out = self;
    
    return afxObject->GetObj();
  }

  virtual void CloseInterop() override {
    Close();

    m_PipeQueue.Abort();

    m_Context = nullptr;
  }

 private:
    CefRefPtr<CefV8Context> m_Context;

 public:


  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override {
    auto name = message->GetName();
    if (name == "afx-message") {
      auto args = message->GetArgumentList();

      auto argSenderId = args->GetInt(0);
      auto argMessage = args->GetString(1);

      CefPostTask(TID_RENDERER,
                  base::Bind(&CDrawingInteropImpl::OnClientMessage_Message,
                             this, argSenderId, argMessage.ToString()));

      return true;
    } else if (name == "afx-render") {
      auto args = message->GetArgumentList();

      HANDLE share_handle = (HANDLE)((unsigned __int64)args->GetInt(0) |
                            ((unsigned __int64)args->GetInt(1) << 32));

      CefPostTask(TID_RENDERER,
          base::Bind(&CDrawingInteropImpl::OnClientMessage_CefFrameRendered,
                             this, share_handle));

      return true;
    }

    return false;
  }

 protected:
  CDrawingInteropImpl() : CAfxInterop("advancedfxInterop_drawing") {
  }

  virtual void OnClosing() override { m_InFlow = false; }

 private:

 void OnClientMessage_Message(int senderId, const std::string & message)
 {
  m_Context->Enter();

  CefV8ValueList execArgs;
  execArgs.push_back(CefV8Value::CreateInt(senderId));
  execArgs.push_back(CefV8Value::CreateString(message));

  if (m_OnMessage->IsValid()) {

    m_OnMessage->ExecuteCallback(execArgs);
  }

  m_Context->Exit();
 }

 void OnClientMessage_CefFrameRendered(HANDLE sharedTextureHandle) {
   m_Context->Enter();

   CefV8ValueList execArgs;
   execArgs.push_back(CAfxHandle::Create(sharedTextureHandle));

   if (m_OnCefFrameRendered->IsValid())
     m_OnCefFrameRendered->ExecuteCallback(execArgs);

   m_Context->Exit();
 }
  private:

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
              AFX_PRINT_ERROR("CAfxD3d9VertexDeclaration.release", __LINE__)
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
        AFX_PRINT_ERROR("CAfxD3d9IndexBuffer.release", __LINE__)
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
            size_t offsetToLock = (int)arguments[1]->GetUIntValue();
            size_t sizeToLock = (int)arguments[2]->GetUIntValue();

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

            return true;

        error_2:
            AFX_PRINT_ERROR("CAfxD3d9IndexBuffer.update", __LINE__)
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
            AFX_PRINT_ERROR("CAfxD3d9VertexBuffer.release", __LINE__)
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
                size_t offsetToLock = (int)arguments[1]->GetUIntValue();
                size_t sizeToLock = (int)arguments[2]->GetUIntValue();

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

                return true;

              error_2:
                AFX_PRINT_ERROR("CAfxD3d9VertexBuffer.update", __LINE__)
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
        AFX_PRINT_ERROR("CAfxD3d9Texture.release", __LINE__)

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
              AFX_PRINT_ERROR("CAfxD3d9Texture.update", __LINE__)

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
        AFX_PRINT_ERROR("CAfxD3d9PixelShader.release", __LINE__)
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
        AFX_PRINT_ERROR("CAfxD3d9VertexShader.release", __LINE__)
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

  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;
  CefRefPtr<CAfxCallback> m_OnDeviceLost;
  CefRefPtr<CAfxCallback> m_OnDeviceReset;
  CefRefPtr<CAfxCallback> m_OnCefFrameRendered;

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
          break;
        case DrawingMessage::DeviceLost: {
          CefPostTask(TID_RENDERER, new CAfxTask([this]() {
                        if (m_OnDeviceLost->IsValid())
                          m_OnDeviceLost->ExecuteCallback(CefV8ValueList());
                      }));

        }
          bContinue = true;
          break;
        case DrawingMessage::DeviceRestored: {
          CefPostTask(TID_RENDERER, new CAfxTask([this]() {
                        if (m_OnDeviceReset->IsValid())
                          m_OnDeviceReset->ExecuteCallback(CefV8ValueList());
                      }));        
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
        /*std::string code = std::to_string(clientFrameCount) + " < " +
                           std::to_string(frameCount) + " || " +
                           std::to_string(clientFrameCount) +
                           " == " + std::to_string(frameCount) + " && " +
                           std::to_string(pass) + " < " +
                           std::to_string(clientPass);
        MessageBoxA(0, code.c_str(), "DrawingReply::Retry", MB_OK);*/

        return true;

      } else if (frameDiff > 0 || frameDiff == 0 && pass > clientPass) {
        // client is behind.

        if (!m_PipeServer->WriteInt32((INT32)DrawingReply::Skip))
          return false;

        if(!m_PipeServer->Flush())
            return false;

        // OutputDebugStringA("DrawingReply::Skip\n");
        /*std::string code = std::to_string(clientFrameCount) + " > " +
                           std::to_string(frameCount) + " || " +
                           std::to_string(clientFrameCount) +
                           " == " + std::to_string(frameCount) + " && " +
                           std::to_string(pass) + "> " +
                           std::to_string(clientPass);
        MessageBoxA(0, code.c_str(), "DrawingReply::Retry", MB_OK);*/
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
   self->m_Context = context;

   afxObject->AddGetter("id",
       [browser](const CefString& name, const CefRefPtr<CefV8Value> object,
                 CefRefPtr<CefV8Value>& retval, CefString& exception) {
         retval = CefV8Value::CreateInt(browser->GetIdentifier());

         return true;
       });

   afxObject->AddFunction("init",
       [obj = afxObject->GetObj(), argStr](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (1 == arguments.size()) {
                 auto argInitFn = arguments[0];

                 if (argInitFn->IsFunction()) {
                   CefV8ValueList args;
                   args.push_back(obj);
                   args.push_back(CefV8Value::CreateString(argStr));

                   argInitFn->ExecuteFunctionWithContext(CefV8Context::GetCurrentContext(), NULL, args);

                   return true;
                 }
               }

               return false;
             });

    afxObject->AddFunction(
       "sendMessage",
       [frame](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
         if (arguments.size() == 2) {
           auto argId = arguments[0];
           auto argStr = arguments[1];

           if (argId->IsInt() && argStr->IsString()) {
             auto message = CefProcessMessage::Create("afx-message");
             auto args = message->GetArgumentList();
             args->SetInt(0, argId->GetIntValue());
             args->SetString(1, argStr->GetStringValue());

             frame->SendProcessMessage(PID_BROWSER, message);

             return true;
           }
         }

         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   self->m_OnMessage = afxObject->AddCallback("onMessage");

   self->m_OnError = afxObject->AddCallback("onError");

  self->m_OnCefFrameRendered = afxObject->AddCallback("onCefFrameRendered");

    afxObject->AddFunction(
      "setPipeName",
      [self](const CefString& name, CefRefPtr<CefV8Value> object,
             const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
             CefString& exceptionoverride) {
        if (1 <= arguments.size() && arguments[0]->IsString()) {
          self->m_PipeQueue.Queue(
              [self, pipeName = arguments[0]->GetStringValue().ToString()]() {
                self->SetPipeName(pipeName.c_str());
              });
          return true;
        }

        exceptionoverride = g_szInvalidArguments;
        return true;
      });

  afxObject->AddFunction("connect", [self](const CefString& name,
                                             CefRefPtr<CefV8Value> object,
                                             const CefV8ValueList& arguments,
                                             CefRefPtr<CefV8Value>& retval,
                                             CefString& exceptionoverride) {
      if (2 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction()) {
        self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                 fn_reject = arguments[1]]() {
          if (self->Connection(12000, 12000)) {
            CefPostTask(TID_RENDERER,
                        new CAfxTask([ctx = self->m_Context, fn_resolve,
                                      connected = self->Connected()]() {
                          ctx->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateBool(connected));
                          fn_resolve->ExecuteFunction(NULL, args);
                          ctx->Exit();
                        }));
          } else {
            CefPostTask(TID_RENDERER,
                        new CAfxTask([ctx = self->m_Context, fn_reject]() {
                          ctx->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          ctx->Exit();
                        }));
          }
        });
        return true;
      }
      exceptionoverride = g_szInvalidArguments;
      return true;
    });

    afxObject->AddFunction("pump", [self](const CefString& name,
                                          CefRefPtr<CefV8Value> object,
                                          const CefV8ValueList& arguments,
                                          CefRefPtr<CefV8Value>& retval,
                                          CefString& exceptionoverride) {
      if (2 <= arguments.size() && arguments[0]->IsObject() &&
          arguments[1]->IsObject()) {

        self->m_PipeQueue.Queue(
            [self, filter = new CAfxValue(arguments[0]),
                                 obj = new CAfxValue(arguments[1])]() {
          self->DoPump(filter, obj);
        });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });

    afxObject->AddFunction(
        "close",
        [self](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          self->m_PipeQueue.Queue([self]() { self->Close(); });
          return true;
        });

   afxObject->AddFunction("scheduleCommand",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
              CefString& exceptionoverride) {
         if (1 <= arguments.size()) {
           const CefRefPtr<CefV8Value>& arg0 = arguments[0];
           if (arg0->IsString()) {
             self->m_PipeQueue.Queue(
                 [self, cmd = arg0->GetStringValue().ToString()]() {
                   self->m_Commands.emplace(cmd.c_str());
                 });
             return true;
           }
         }

         exceptionoverride = g_szInvalidArguments;
         return true;
       });

/*
   afxObject->AddFunction("addCalcHandle",
             [self](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() && arguments[0]->IsString() && arguments[1]->IsFunction()) {
           retval = CCalcResult::Create(
                          &self->m_HandleCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   afxObject->AddFunction("addCalcVecAng",
             [self](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                    CCalcResult::Create(
                          &self->m_VecAngCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   afxObject->AddFunction("addCalcCam",
             [self](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                     CCalcResult::Create(
                          &self->m_CamCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   afxObject->AddFunction("addCalcFov",
             [self](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
               if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                     CCalcResult::Create(
                          &self->m_FovCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction(
       "addCalcBool",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exceptionoverride) {
         if (2 == arguments.size() &&
             arguments[0]->IsString() && arguments[1]->IsFunction()) {
           std::string valName = arguments[0]->GetStringValue().ToString();

           retval = CCalcResult::Create(
                         &self->m_BoolCalcCallbacks,
                         arguments[0]->GetStringValue().ToString().c_str(),
               new CAfxCallback(arguments[1]));

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   afxObject->AddFunction(
       "addCalcInt",
       [self](const CefString& name, CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exceptionoverride) {
         if (2 == arguments.size() &&
             arguments[0]->IsString() && arguments[1]->IsFunction()) {
           std::string valName = arguments[0]->GetStringValue().ToString();

           retval = CCalcResult::Create(
                         &self->m_IntCalcCallbacks,
                         arguments[0]->GetStringValue().ToString().c_str(),
               new CAfxCallback(arguments[1]));

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
       */
   self->m_OnGameEvent = afxObject->AddCallback("onGameEvent");
   /*
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
*/
   afxObject->AddFunction(
       "renderCefFrame",
       [self, frame](const CefString& name, CefRefPtr<CefV8Value> object,
                     const CefV8ValueList& arguments,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& exceptionoverride) {
         auto message = CefProcessMessage::Create("afx-render");

         frame->SendProcessMessage(PID_BROWSER, message);

         return true;
       });
 
   //

   if (out)
     *out = self;

   return afxObject->GetObj();
}

  virtual void CloseInterop() override {
    Close();

    m_Context = nullptr;
  }

  private:
      CefRefPtr<CefV8Context> m_Context;


 public:


virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override {
    auto name = message->GetName();
    if (name == "afx-message") {
      auto args = message->GetArgumentList();

      auto argSenderId = args->GetInt(0);
      auto argMessage = args->GetString(1);

      CefPostTask(TID_RENDERER,
                  base::Bind(&CEngineInteropImpl::OnClientMessage_Message,
                             this, argSenderId, argMessage.ToString()));

      return true;
    } else if (name == "afx-render") {
      auto args = message->GetArgumentList();

      HANDLE share_handle = (HANDLE)((unsigned __int64)args->GetInt(0) |
                                     ((unsigned __int64)args->GetInt(1) << 32));

      CefPostTask(
          TID_RENDERER,
          base::Bind(&CEngineInteropImpl::OnClientMessage_CefFrameRendered,
                     this, share_handle));

      return true;
    }

    return false;
  }

 protected:

  CEngineInteropImpl() : CAfxInterop("advancedfxInterop") {
  }

  CefRefPtr<CefV8Value> GetPumpFilter(CefRefPtr<CAfxValue> filter,
                                      const char* what) {
      if (filter != nullptr && filter->IsObject()) {
      auto value = filter->GetChild(what);
      if (value && value->IsV8Function()) {
        return value->GetFunction();
      }
    }
    return nullptr;
  }

  int m_PumpResumeAt = 0;

  float m_Tx, m_Ty, m_Tz, m_Rx, m_Ry, m_Rz, m_Fov;

  void DoPump(CefRefPtr<CAfxValue> filter, CefRefPtr<CAfxValue> obj) {
    int errorLine = 0;

    if (!Connected())
      AFX_GOTO_ERROR;

    switch (m_PumpResumeAt) {
      case 0:
        goto __0;
      case 1:
        goto __1;
      case 2:
        goto __2;
      case 3:
        goto __3;
      case 4:
        goto __4;
      case 5:
        goto __5;
      default:
        AFX_GOTO_ERROR;
    }

  __0 : {
    if (m_NewConnection) {
      m_NewConnection = false;

      // Check if our version is supported by client:

      if (!m_PipeServer->WriteInt32(Version))
        AFX_GOTO_ERROR
      
      if (!m_PipeServer->Flush())
        AFX_GOTO_ERROR


      bool versionSupported;
      if (!m_PipeServer->ReadBoolean(versionSupported))
        AFX_GOTO_ERROR

      if (!versionSupported)
        AFX_GOTO_ERROR

      // Supply server info required by client:

#if _WIN64
      if (!m_PipeServer->WriteBoolean(true))
        AFX_GOTO_ERROR
#else
      if (!m_PipeServer->WriteBoolean(false))
        AFX_GOTO_ERROR
#endif

      if (!WriteGameEventSettings(false))
        AFX_GOTO_ERROR

      if (!m_PipeServer->Flush())
        AFX_GOTO_ERROR

      auto onNewConnection = GetPumpFilter(filter, "onNewConnection");
      if (nullptr != onNewConnection) {
        m_PumpResumeAt = 1;
        CefPostTask(TID_RENDERER, new CAfxTask([this, onNewConnection]() {
                      m_Context->Enter();
                      onNewConnection->ExecuteFunction(nullptr, CefV8ValueList());
                      m_Context->Exit();
                    }));
        return;
      }
    }

    goto __1;
  }
 
  __1: {
      UINT32 engineMessage;
      if (!m_PipeServer->ReadUInt32(engineMessage))
        AFX_GOTO_ERROR

      switch ((EngineMessage)engineMessage) {
        case EngineMessage::BeforeFrameStart: {
          // Read incoming commands from client:
          {
            UINT32 commandIndex = 0;
            UINT32 commandCount;
            if (!m_PipeServer->ReadCompressedUInt32(commandCount))
              AFX_GOTO_ERROR

            CefRefPtr<CAfxValue> objCommands = CAfxValue::CreateArray((int)commandCount);

            while (0 < commandCount) {
              UINT32 argIndex = 0;
              UINT32 argCount;
              if (!m_PipeServer->ReadCompressedUInt32(argCount))
                AFX_GOTO_ERROR

              CefRefPtr<CAfxValue> objArgs =
                  CAfxValue::CreateArray((int)argCount);

              objCommands->SetArrayElement((int)commandIndex, objArgs);

              while (0 < argCount) {
                std::string str;

                if (!m_PipeServer->ReadStringUTF8(str))
                  AFX_GOTO_ERROR

                objArgs->SetArrayElement((int)argIndex,
                                         CAfxValue::CreateString(str));

                --argCount;
                ++argIndex;
              }

              --commandCount;
              ++commandIndex;
            }

            auto onCommands = GetPumpFilter(filter, "onCommands");
            if (nullptr != onCommands) {
              m_PumpResumeAt = 2;
              CefPostTask(TID_RENDERER,
                          new CAfxTask([this, onCommands, objCommands]() {
                            m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(objCommands->ToV8Value());
                            onCommands->ExecuteFunction(nullptr, args);
                            m_Context->Exit();
                          }));
              return;
            }
          }          
        } goto __2;

        case EngineMessage::BeforeFrameRenderStart: {
          if (!WriteGameEventSettings(true))
            AFX_GOTO_ERROR
          if (!m_PipeServer->Flush())
            AFX_GOTO_ERROR
        } goto __1;

        case EngineMessage::AfterFrameRenderStart: {
          if (!m_HandleCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_VecAngCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_CamCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_FovCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_BoolCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_IntCalcCallbacks.BatchUpdateRequest(m_PipeServer))
            AFX_GOTO_ERROR

          if (!m_PipeServer->Flush())
            AFX_GOTO_ERROR

          if (!m_HandleCalcCallbacks.BatchUpdateResult(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_VecAngCalcCallbacks.BatchUpdateResult(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_CamCalcCallbacks.BatchUpdateResult(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_FovCalcCallbacks.BatchUpdateResult(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_BoolCalcCallbacks.BatchUpdateResult(m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_IntCalcCallbacks.BatchUpdateResult(m_PipeServer))
            AFX_GOTO_ERROR
        }
          goto __1;

        case EngineMessage::OnRenderView: {
          struct RenderInfo_s renderInfo;

          if (!m_PipeServer->ReadInt32(renderInfo.FrameCount))
            AFX_GOTO_ERROR

          if (!m_PipeServer->ReadSingle(renderInfo.AbsoluteFrameTime))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.CurTime))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.FrameTime))
            AFX_GOTO_ERROR

          if (!m_PipeServer->ReadInt32(renderInfo.View.X))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadInt32(renderInfo.View.Y))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadInt32(renderInfo.View.Width))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadInt32(renderInfo.View.Height))
            AFX_GOTO_ERROR

          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M00))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M01))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M02))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M03))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M10))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M11))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M12))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M13))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M20))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M21))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M22))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M23))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M30))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M31))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M32))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ViewMatrix.M33))
            AFX_GOTO_ERROR

          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M00))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M01))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M02))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M03))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M10))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M11))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M12))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M13))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M20))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M21))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M22))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M23))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M30))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M31))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M32))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(renderInfo.View.ProjectionMatrix.M33))
            AFX_GOTO_ERROR

          auto onRenderViewBegin = GetPumpFilter(filter, "onRenderViewBegin");
          if (nullptr != onRenderViewBegin) {
            m_PumpResumeAt = 3;
            CefPostTask(TID_RENDERER,
                new CAfxTask([this, onRenderViewBegin,
                              renderInfo = CreateAfxRenderInfo(renderInfo)]() {
                          m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(renderInfo->ToV8Value());
                          onRenderViewBegin->ExecuteFunction(nullptr, args);
                          m_Context->Exit();
                        }));
            return;
          }
        } goto __3;

        case EngineMessage::OnRenderViewEnd: {
          auto onRenderViewEnd = GetPumpFilter(filter, "onRenderViewEnd");
          if (nullptr != onRenderViewEnd) {
            CefPostTask(TID_RENDERER, new CAfxTask([this, onRenderViewEnd]() {
                          m_PumpResumeAt = 4;
                          m_Context->Enter();
                          onRenderViewEnd->ExecuteFunction(nullptr,
                                                           CefV8ValueList());
                          m_Context->Exit();
                        }));
            return;
          }
        } goto __4;

        case EngineMessage::BeforeHud: {
          auto onHudBegin = GetPumpFilter(filter, "onRenderViewHudBegin");
          if (nullptr != onHudBegin) {
            CefPostTask(TID_RENDERER, new CAfxTask([this, onHudBegin]() {
                          m_PumpResumeAt = 1;
                          m_Context->Enter();
                          onHudBegin->ExecuteFunction(nullptr, CefV8ValueList());
                          m_Context->Exit();
                        }));
            return;
          }
        } goto __1;

        case EngineMessage::AfterHud: {
          auto onHudEnd = GetPumpFilter(filter, "onRenderViewHudEnd");
          if (nullptr != onHudEnd) {
            CefPostTask(TID_RENDERER, new CAfxTask([this, onHudEnd]() {
                          m_PumpResumeAt = 1;
                          m_Context->Enter();
                          onHudEnd->ExecuteFunction(nullptr, CefV8ValueList());
                          m_Context->Exit();
                        }));
            return;
          }
        } goto __1;

        case EngineMessage::BeforeTranslucentShadow: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewBeforeTranslucentShadow", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return;
        }
          goto __1;
        case EngineMessage::AfterTranslucentShadow: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewAfterTranslucentShadow", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return;
        }
          goto __1;
        case EngineMessage::BeforeTranslucent: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewBeforeTranslucent", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return;
        }
          goto __1;
        case EngineMessage::AfterTranslucent: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewAfterTranslucent", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return;
        }
          goto __1;

        case EngineMessage::OnViewOverride: {
          if (!m_PipeServer->ReadSingle(m_Tx))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(m_Ty))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(m_Tz))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(m_Rx))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(m_Ry))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(m_Rz))
            AFX_GOTO_ERROR
          if (!m_PipeServer->ReadSingle(m_Fov))
            AFX_GOTO_ERROR

          auto onViewOverride = GetPumpFilter(filter, "onViewOverride");
          if (nullptr != onViewOverride) {
            m_PumpResumeAt = 5;

            CefPostTask(TID_RENDERER,
                        new CAfxTask([this, onViewOverride, tx = m_Tx, ty = m_Ty, tz = m_Tz, rx = m_Rx, ry = m_Ry, rz = m_Rz, fov= m_Fov]() {
                          m_Context->Enter();

                        CefV8ValueList args;
                        CefRefPtr<CefV8Value> obj2 =
                              CefV8Value::CreateObject(nullptr, nullptr);

                          obj2->SetValue("tX", CefV8Value::CreateDouble(tx),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                          obj2->SetValue("tY", CefV8Value::CreateDouble(ty),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                          obj2->SetValue("tZ", CefV8Value::CreateDouble(tz),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                          obj2->SetValue("rX", CefV8Value::CreateDouble(rx),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                          obj2->SetValue("rY", CefV8Value::CreateDouble(ry),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                          obj2->SetValue("rZ", CefV8Value::CreateDouble(rz),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                          obj2->SetValue("fov", CefV8Value::CreateDouble(fov),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                          args.push_back(obj2);

                          onViewOverride->ExecuteFunction(nullptr, args);
                          m_Context->Exit();
                        }));
            return;
          }
        } goto __5;
 

        case EngineMessage::GameEvent:
          if (!ReadGameEvent())
            AFX_GOTO_ERROR
          break;
      }
    }

    AFX_GOTO_ERROR

__2: {
    if (!m_PipeServer->WriteCompressedUInt32((UINT32)m_Commands.size()))
      AFX_GOTO_ERROR

    while (!m_Commands.empty()) {
      if (!m_PipeServer->WriteStringUTF8(m_Commands.front().c_str()))
        AFX_GOTO_ERROR
      m_Commands.pop();
    }

    if (!m_PipeServer->Flush())
      AFX_GOTO_ERROR

    goto __1;
}

__3 : {
  bool outBeforeTranslucentShadow = false;
  bool outAfterTranslucentShadow = false;
  bool outBeforeTranslucent = false;
  bool outAfterTranslucent = false;
  bool outBeforeHud = false;
  bool outAfterHud = false;
  bool outAfterRenderView = false;

  if (filter && filter->IsObject()) {
    CefRefPtr<CAfxValue> beforeTranslucentShadow =
        filter->GetChild("onRenderViewBeforeTranslucentShadow");
    if (beforeTranslucentShadow && beforeTranslucentShadow->IsBool())
      outBeforeTranslucentShadow = beforeTranslucentShadow->GetBool();

    CefRefPtr<CAfxValue> afterTranslucentShadow =
        filter->GetChild("onRenderViewAfterTranslucentShadow");
    if (afterTranslucentShadow && afterTranslucentShadow->IsBool())
      outAfterTranslucentShadow = afterTranslucentShadow->GetBool();

    CefRefPtr<CAfxValue> beforeTranslucent =
        filter->GetChild("onRenderViewBeforeTranslucent");
    if (beforeTranslucent && beforeTranslucent->IsBool())
      outBeforeTranslucent = beforeTranslucent->GetBool();

    CefRefPtr<CAfxValue> afterTranslucent =
        filter->GetChild("onRenderViewAfterTranslucent");
    if (afterTranslucent && afterTranslucent->IsBool())
      outAfterTranslucent = afterTranslucent->GetBool();

    CefRefPtr<CAfxValue> beforeHud = filter->GetChild("onRenderViewHudBegin");
    if (beforeHud && beforeHud->IsBool())
      outBeforeHud = beforeHud->GetBool();

    CefRefPtr<CAfxValue> afterHud = filter->GetChild("onRenderViewHudEnd");
    if (afterHud && afterHud->IsBool())
      outAfterHud = afterHud->GetBool();

    CefRefPtr<CAfxValue> afterRenderView = filter->GetChild("onRenderViewEnd");
    if (afterRenderView && afterRenderView->IsBool())
      outAfterRenderView = afterRenderView->GetBool();
  }

  if (!m_PipeServer->WriteBoolean(outBeforeTranslucentShadow))
    AFX_GOTO_ERROR
  if (!m_PipeServer->WriteBoolean(outAfterTranslucentShadow))
    AFX_GOTO_ERROR
  if (!m_PipeServer->WriteBoolean(outBeforeTranslucent))
    AFX_GOTO_ERROR
  if (!m_PipeServer->WriteBoolean(outAfterTranslucent))
    AFX_GOTO_ERROR
  if (!m_PipeServer->WriteBoolean(outBeforeHud))
    AFX_GOTO_ERROR
  if (!m_PipeServer->WriteBoolean(outAfterHud))
    AFX_GOTO_ERROR
  if (!m_PipeServer->WriteBoolean(outAfterRenderView))
    AFX_GOTO_ERROR

  bool done = !(outBeforeTranslucentShadow || outAfterTranslucentShadow ||
                outBeforeTranslucent || outAfterTranslucent || outBeforeHud ||
                outAfterHud || outAfterRenderView || true);

  if (done)
    goto __4;

  goto __1;
}

__4 : {
  auto onDone = GetPumpFilter(filter, "onDone");
  if (nullptr != onDone) {
    m_PumpResumeAt = 1;
    CefPostTask(TID_RENDERER, new CAfxTask([this, onDone]() {
                  m_Context->Enter();
                  onDone->ExecuteFunction(nullptr, CefV8ValueList());
                  m_Context->Exit();
                }));
    return;
  }
}

__5 : {
      bool overriden = false;

      if (obj && obj->IsObject()) {
        CefRefPtr<CAfxValue> v8Tx = obj->GetChild("tX");
        if (v8Tx && v8Tx->IsDouble()) {
          m_Tx = (float)v8Tx->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Ty = obj->GetChild("tY");
        if (v8Ty && v8Ty->IsDouble()) {
          m_Ty = (float)v8Ty->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Tz = obj->GetChild("tZ");
        if (v8Tz && v8Tz->IsDouble()) {
          m_Tz = (float)v8Tz->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Rx = obj->GetChild("rX");
        if (v8Rx && v8Rx->IsDouble()) {
          m_Rx = (float)v8Rx->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Ry = obj->GetChild("rY");
        if (v8Ry && v8Ry->IsDouble()) {
          m_Ry = (float)v8Ry->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Rz = obj->GetChild("rZ");
        if (v8Rz && v8Rz->IsDouble()) {
          m_Rz = (float)v8Rz->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Fov = obj->GetChild("fov");
        if (v8Fov && v8Fov->IsDouble()) {
          m_Fov = (float)v8Fov->GetDouble();
          overriden = true;
        }
      }

      if (overriden) {
        if (!m_PipeServer->WriteBoolean(true))
          AFX_GOTO_ERROR

        if (!m_PipeServer->WriteSingle(m_Tx))
          AFX_GOTO_ERROR
        if (!m_PipeServer->WriteSingle(m_Ty))
          AFX_GOTO_ERROR
        if (!m_PipeServer->WriteSingle(m_Tz))
          AFX_GOTO_ERROR
        if (!m_PipeServer->WriteSingle(m_Rx))
          AFX_GOTO_ERROR
        if (!m_PipeServer->WriteSingle(m_Ry))
          AFX_GOTO_ERROR
        if (!m_PipeServer->WriteSingle(m_Rz))
          AFX_GOTO_ERROR
        if (!m_PipeServer->WriteSingle(m_Fov))
          AFX_GOTO_ERROR
      } else {
        if (!m_PipeServer->WriteBoolean(false))
          AFX_GOTO_ERROR
      }

    if (!m_PipeServer->Flush())
        AFX_GOTO_ERROR  // client is waiting

            goto __1;
  }

  error:
    AFX_PRINT_ERROR("CEngineInteropImpl::DoPump", errorLine)
    m_PumpResumeAt = -1;
    auto onError = GetPumpFilter(filter, "onError");
    if (nullptr != onError) {
      CefPostTask(TID_RENDERER, new CAfxTask([this, onError, errorLine]() {
                    m_Context->Enter();
                    CefV8ValueList args;
                    args.push_back(CefV8Value::CreateInt(errorLine));
                    onError->ExecuteFunction(nullptr, args);
                    m_Context->Exit();
                  }));
    }
    return;
  }


  virtual bool OnNewConnection() override {
    m_PumpResumeAt = 0;
    m_NewConnection = true;
    return true;
  }

 private:

 void OnClientMessage_Message(int senderId, const std::string & message)
 {
  m_Context->Enter();

  CefV8ValueList execArgs;
  execArgs.push_back(CefV8Value::CreateInt(senderId));
  execArgs.push_back(CefV8Value::CreateString(message));

  if (m_OnMessage->IsValid())
    m_OnMessage->ExecuteCallback(execArgs);

  m_Context->Exit();
 }

 void OnClientMessage_CefFrameRendered(HANDLE sharedTextureHandle)
 {
   m_Context->Enter();

  CefV8ValueList execArgs;
  execArgs.push_back(CAfxHandle::Create(sharedTextureHandle));

  if (m_OnCefFrameRendered->IsValid())
    m_OnCefFrameRendered->ExecuteCallback(execArgs);

  m_Context->Exit();
 }

private:
  static CefRefPtr<CAfxValue> CreateAfxMatrix4x4(
      const struct advancedfx::interop::Matrix4x4_s& value) {
   CefRefPtr<CAfxValue> obj = CAfxValue::CreateArray(16);

    obj->SetArrayElement(0, CAfxValue::CreateDouble(value.M00));
   obj->SetArrayElement(
       1, CAfxValue::CreateDouble(value.M01));
    obj->SetArrayElement(
        2, CAfxValue::CreateDouble(value.M02));
   obj->SetArrayElement(
       3, CAfxValue::CreateDouble(value.M03));

    obj->SetArrayElement(
       4, CAfxValue::CreateDouble(value.M10));
   obj->SetArrayElement(
       5, CAfxValue::CreateDouble(value.M11));
    obj->SetArrayElement(
        6, CAfxValue::CreateDouble(value.M12));
   obj->SetArrayElement(
       7, CAfxValue::CreateDouble(value.M13));

    obj->SetArrayElement(
       8, CAfxValue::CreateDouble(value.M20));
   obj->SetArrayElement(
       9, CAfxValue::CreateDouble(value.M21));
    obj->SetArrayElement(
        10, CAfxValue::CreateDouble(value.M22));
   obj->SetArrayElement(
       11, CAfxValue::CreateDouble(value.M23));

    obj->SetArrayElement(
       12, CAfxValue::CreateDouble(value.M30));
   obj->SetArrayElement(
       13, CAfxValue::CreateDouble(value.M31));
    obj->SetArrayElement(
        14, CAfxValue::CreateDouble(value.M32));
   obj->SetArrayElement(
       15, CAfxValue::CreateDouble(value.M33));

    return obj;
  }

  static CefRefPtr<CAfxValue> CreateAfxView(
      const struct advancedfx::interop::View_s& value) {
    CefRefPtr<CAfxValue> obj = CAfxValue::CreateObject();

    obj->SetChild("x", CAfxValue::CreateInt(value.X));
    obj->SetChild("y", CAfxValue::CreateInt(value.Y));
    obj->SetChild("width", CAfxValue::CreateInt(value.Width));
    obj->SetChild("height", CAfxValue::CreateInt(value.Height));
    obj->SetChild("viewMatrix", CreateAfxMatrix4x4(value.ViewMatrix));
    obj->SetChild("projectionMatrix",
                  CreateAfxMatrix4x4(value.ProjectionMatrix));

    return obj;
  }

  static CefRefPtr<CAfxValue> CreateAfxRenderInfo(
      const struct advancedfx::interop::RenderInfo_s& value) {
    CefRefPtr<CAfxValue> obj = CAfxValue::CreateObject();

    obj->SetChild("view", CreateAfxView(value.View));

    obj->SetChild("frameCount", CAfxValue::CreateInt(value.FrameCount));
    obj->SetChild("absoluteFrameTime",
                  CAfxValue::CreateDouble(value.AbsoluteFrameTime));
    obj->SetChild("curTime", CAfxValue::CreateDouble(value.CurTime));
    obj->SetChild("frameTime", CAfxValue::CreateDouble(value.FrameTime));

    return obj;
  }

  bool m_NewConnection = true;

  std::queue<std::string> m_Commands;

  CHandleCalcCallbacks m_HandleCalcCallbacks;
  CVecAngCalcCallbacks m_VecAngCalcCallbacks;
  CCamCalcCallbacks m_CamCalcCallbacks;
  CFovCalcCallbacks m_FovCalcCallbacks;
  CBoolCalcCallbacks m_BoolCalcCallbacks;
  CIntCalcCallbacks m_IntCalcCallbacks;

  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;
  CefRefPtr<CAfxCallback> m_OnGameEvent;

  CefRefPtr<CAfxCallback> m_OnCefFrameRendered;

  bool DoRenderPass(CefRefPtr<CAfxValue> filter, const char* what, bool & bReturn) {

    bReturn = false;

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

    auto onRenderPass = GetPumpFilter(filter, what);
    if (nullptr != onRenderPass) {
      m_PumpResumeAt = 1;

      CefPostTask(TID_RENDERER, new CAfxTask([this, onRenderPass,
                                              argView = CreateAfxView(view)]() {
                    m_Context->Enter();
                    CefV8ValueList args;
                    args.push_back(argView->ToV8Value());
                    onRenderPass->ExecuteFunction(nullptr, args);
                    m_Context->Exit();
                  }));
      bReturn = true;
      return true;
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
  return CEngineInteropImpl::Create(browser,frame,context, argStr, out);
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

    self->m_Context = context;

    afxObject->AddGetter(
        "id",
        [browser](const CefString& name, const CefRefPtr<CefV8Value> object,
                  CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateInt(browser->GetIdentifier());

          return true;
        });

    afxObject->AddFunction("init",
        [obj = afxObject->GetObj(), argStr](
                  const CefString& name, CefRefPtr<CefV8Value> object,
                  const CefV8ValueList& arguments,
                  CefRefPtr<CefV8Value>& retval, CefString& exceptionoverride) {
                if (1 == arguments.size()) {
                  auto argInitFn = arguments[0];

                  if (argInitFn->IsFunction()) {
                    CefV8ValueList args;
                    args.push_back(obj);
                    args.push_back(CefV8Value::CreateString(argStr));

                    argInitFn->ExecuteFunctionWithContext(CefV8Context::GetCurrentContext(), NULL, args);

                    return true;
                  }
                }

               exceptionoverride = g_szInvalidArguments;
          return true;

              });

    afxObject->AddFunction(
      "createDrawingInterop",
      [frame](
          const CefString& name, CefRefPtr<CefV8Value> object,
          const CefV8ValueList& arguments,
          CefRefPtr<CefV8Value>& retval, CefString& exceptionoverride) {
        if (2 == arguments.size()) {
          auto argUrl = arguments[0];
          auto argStr = arguments[1];

          if (argUrl->IsString() && argStr->IsString()) {
            auto message = CefProcessMessage::Create("afx-create-drawing");
            auto args = message->GetArgumentList();
            args->SetString(0, argUrl->GetStringValue());
            args->SetString(1, argStr->GetStringValue());

            frame->SendProcessMessage(PID_BROWSER, message);

            return true;
          }
        }

        exceptionoverride = g_szInvalidArguments;
        return true;
      });

    afxObject->AddFunction(
      "createEngineInterop",
      [frame](
          const CefString& name, CefRefPtr<CefV8Value> object,
          const CefV8ValueList& arguments,
          CefRefPtr<CefV8Value>& retval, CefString& exceptionoverride) {
        if (2 == arguments.size()) {
          auto argUrl = arguments[0];
          auto argStr = arguments[1];

          if (argUrl->IsString() && argStr->IsString()) {

                            auto message = CefProcessMessage::Create("afx-create-engine");
            auto args = message->GetArgumentList();
                            args->SetString(0, argUrl->GetStringValue());
            args->SetString(1, argStr->GetStringValue());

            frame->SendProcessMessage(PID_BROWSER, message);

            return true;
          }
        }

        exceptionoverride = g_szInvalidArguments;
        return true;
      });

    afxObject->AddFunction(
        "sendMessage",
        [frame](const CefString& name, CefRefPtr<CefV8Value> object,
                const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                CefString& exceptionoverride) {
          if (arguments.size() == 2) {
            auto argId = arguments[0];
            auto argStr = arguments[1];

            if (argId->IsInt() && argStr->IsString()) {
              auto message = CefProcessMessage::Create("afx-message");
              auto args = message->GetArgumentList();
              args->SetInt(0, argId->GetIntValue());
              args->SetString(1, argStr->GetStringValue());

              frame->SendProcessMessage(PID_BROWSER, message);

              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    afxObject->AddFunction(
        "renderCefFrame",
        [self, frame](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          auto message = CefProcessMessage::Create("afx-render");

          frame->SendProcessMessage(PID_BROWSER, message);

          return true;
        });
  
    self->m_OnMessage = afxObject->AddCallback("onMessage");

    self->m_OnError = afxObject->AddCallback("onError");

    self->m_OnCefFrameRendered = afxObject->AddCallback("onCefFrameRendered");

    //

    if (out)
      *out = self;

    return afxObject->GetObj();
  }

  virtual void CloseInterop() override {
    
    
    m_Context = nullptr;  
  }

  private:

      CefRefPtr<CefV8Context> m_Context;

virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override {
    auto name = message->GetName();
    if (name == "afx-message") {
      auto args = message->GetArgumentList();

      auto argSenderId = args->GetInt(0);
      auto argMessage = args->GetString(1);

      CefPostTask(TID_RENDERER,
                  base::Bind(&CInteropImpl::OnClientMessage_Message,
                             this, argSenderId, argMessage.ToString()));

      return true;
    } else if (name == "afx-render") {
      auto args = message->GetArgumentList();

      HANDLE share_handle = (HANDLE)((unsigned __int64)args->GetInt(0) |
                                     ((unsigned __int64)args->GetInt(1) << 32));

      CefPostTask(
          TID_RENDERER,
          base::Bind(&CInteropImpl::OnClientMessage_CefFrameRendered,
                     this, share_handle));

      return true;
    }

    return false;
  }

 private:
  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;
 CefRefPtr<CAfxCallback> m_OnCefFrameRendered;

  void OnClientMessage_Message(int senderId, const std::string & message)
 {
  m_Context->Enter();

  CefV8ValueList execArgs;
  execArgs.push_back(CefV8Value::CreateInt(senderId));
  execArgs.push_back(CefV8Value::CreateString(message));

  if (m_OnMessage->IsValid())
    m_OnMessage->ExecuteCallback(execArgs);

  m_Context->Exit();
 }

 void OnClientMessage_CefFrameRendered(HANDLE sharedTextureHandle)
 {
 m_Context->Enter();

  CefV8ValueList execArgs;
  execArgs.push_back(CAfxHandle::Create(sharedTextureHandle));

  if (m_OnCefFrameRendered->IsValid())
    m_OnCefFrameRendered->ExecuteCallback(execArgs);

  m_Context->Exit();
 }
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
