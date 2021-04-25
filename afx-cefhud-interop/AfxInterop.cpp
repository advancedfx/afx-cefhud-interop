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

const char * g_szAlreadyReleased = "Already released.";
const char* g_szOutOfFlow = "Out of flow.";
const char* g_szConnectionError = "Connection error.";
const char* g_szInvalidArguments = "Invalid arguments.";
const char* g_szMemoryAllocationFailed = "Memory allocation failed.";
const char* g_szInvalidThis = "Invalid this object.";

class CVersion {
 public:
  CVersion() : m_Major(0), m_Minor(0), m_Patch(0), m_Build(0) {}

  CVersion(unsigned int major,
           unsigned int minor,
           unsigned int patch,
           unsigned int build)
      : m_Major(major), m_Minor(minor), m_Patch(patch), m_Build(build) {}

  CVersion(const CVersion& other)
      : m_Major(other.m_Major),
        m_Minor(other.m_Minor),
        m_Patch(other.m_Patch),
        m_Build(other.m_Build) {}

  void operator=(const CVersion& other) {
    m_Major = other.m_Major;
    m_Minor = other.m_Minor;
    m_Patch = other.m_Patch;
    m_Build = other.m_Build;
  }

  bool operator<(const CVersion& other) const { return Compare(other) < 0; }

  bool operator==(const CVersion& other) const { return Compare(other) == 0; }

  bool operator>(const CVersion& other) const { return Compare(other) > 0; }

  int Compare(const CVersion& other) const {
    if (m_Major < other.m_Major)
      return -1;
    else if (m_Major > other.m_Major)
      return 1;
    if (m_Minor < other.m_Minor)
      return -1;
    else if (m_Minor > other.m_Minor)
      return 1;
    if (m_Patch < other.m_Patch)
      return -1;
    else if (m_Patch > other.m_Patch)
      return 1;
    if (m_Build < other.m_Build)
      return -1;
    else if (m_Build > other.m_Build)
      return 1;
    return 0;
  }

  void Set(unsigned int major,
           unsigned int minor,
           unsigned int patch,
           unsigned int build) {
    m_Major = major;
    m_Minor = minor;
    m_Patch = patch;
    m_Build = build;
  }

  unsigned int GetMajor() const { return m_Major; }
  unsigned int GetMinor() const { return m_Minor; }
  unsigned int GetPatch() const { return m_Patch; }
  unsigned int GetBuild() const { return m_Build; }

 private:
  unsigned int m_Major;
  unsigned int m_Minor;
  unsigned int m_Patch;
  unsigned int m_Build;
};


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

  D3d9UpdateTexture = 43,
  DrawPrimitiveUP = 44
};

enum class AfxObjectType : int {
  AfxObject,
  AfxHandle,
  AfxData,
  AfxD3d9VertexDeclaration,
  AfxD3d9IndexBuffer,
  AfxD3d9VertexBuffer,
  AfxD3d9Texture,
  AfxD3d9PixelShader,
  AfxD3d9VertexShader,
  AfxD3d9Viewport,
  DrawingInteropImpl,
  EngineInteropImpl,
  InteropImpl
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

    float operator[](size_t index) const {
    switch (index) {
      case 0:
        return M00;
      case 1:
        return M01;
      case 2:
        return M02;
      case 3:
        return M03;
      case 4:
        return M10;
      case 5:
        return M11;
      case 6:
        return M12;
      case 7:
        return M13;
      case 8:
        return M20;
      case 9:
        return M21;
      case 10:
        return M22;
      case 11:
        return M23;
      case 12:
        return M30;
      case 13:
        return M31;
      case 14:
        return M32;
      case 15:
        return M33;
      default:
        return M00;

    }
  }

  float& operator[](size_t index){
      switch (index) {
        case 0:
            return M00;
        case 1:
          return M01;
        case 2:
          return M02;
        case 3:
          return M03;
        case 4:
          return M10;
        case 5:
          return M11;
        case 6:
          return M12;
        case 7:
          return M13;
        case 8:
          return M20;
        case 9:
          return M21;
        case 10:
          return M22;
        case 11:
          return M23;
        case 12:
          return M30;
        case 13:
          return M31;
        case 14:
          return M32;
        case 15:
          return M33;
        default:
          return M00;
    }
  }
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

struct HandleCalcResult_s : public CefBaseRefCounted {
  int IntHandle = -1;

  IMPLEMENT_REFCOUNTING(HandleCalcResult_s);
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

struct VecAngCalcResult_s : public CefBaseRefCounted {
  struct Vector_s Vector;
  struct QAngle_s QAngle;

  IMPLEMENT_REFCOUNTING(VecAngCalcResult_s);
};

struct CamCalcResult_s : public CefBaseRefCounted  {
  struct Vector_s Vector;
  struct QAngle_s QAngle;
  float Fov = 90.0f;

  IMPLEMENT_REFCOUNTING(CamCalcResult_s);
};

struct FovCalcResult_s : public CefBaseRefCounted  {
  float Fov = 90.0f;

  IMPLEMENT_REFCOUNTING(FovCalcResult_s);
};

struct BoolCalcResult_s : public CefBaseRefCounted  {
  bool Result = false;

  IMPLEMENT_REFCOUNTING(BoolCalcResult_s);
};

struct IntCalcResult_s : public CefBaseRefCounted  {
  int Result = 0;

  IMPLEMENT_REFCOUNTING(IntCalcResult_s);
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
    m_Cv.notify_all();
    lock.unlock();

    if (m_Thread.joinable()) {
      m_Thread.join();
    }  
  }
}


void CThreadedQueue::Queue(const fp_t& op) {

  std::unique_lock<std::mutex> lock(m_Lock);
  m_Queue.push(op);

  m_Cv.notify_one();
}

void CThreadedQueue::Queue(fp_t&& op) {

  std::unique_lock<std::mutex> lock(m_Lock);
  m_Queue.push(std::move(op));

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
      switch (lastError) {
        case ERROR_MORE_DATA:
          break;
        case ERROR_INVALID_HANDLE:
          m_Handle = INVALID_HANDLE_VALUE;
        default:
          throw CPipeException(lastError);
      }
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
  HANDLE value;
  ReadBytes(&value, 0, (DWORD)sizeof(value));
  return value;
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
      switch (lastError) {
        case ERROR_MORE_DATA:
          break;
        case ERROR_INVALID_HANDLE:
          m_Handle = INVALID_HANDLE_VALUE;
        default:
          throw CPipeException(lastError);
      }
    }

    offset += bytesWritten;
    length -= bytesWritten;
  }
}

void CPipeWriter::Flush() {

  if (!FlushFileBuffers(m_Handle)) {
    DWORD lastError = GetLastError();
    switch (lastError) {
      case ERROR_INVALID_HANDLE:
        m_Handle = INVALID_HANDLE_VALUE;
      default:
        throw CPipeException(lastError);
    }
  }
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
  WriteBytes(&value, 0, sizeof(value));
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
    else if (FALSE == WaitNamedPipeA(pipeName, timeOut))
    {
      throw CWinApiException("CPipeClient::OpenPipe: WaitNamedPipe failed", GetLastError());
    }
  }

  throw "CPipeClient::OpenPipe failed.";
}

void CPipeClient::ClosePipe() {
  if(INVALID_HANDLE_VALUE != m_Handle) {
    if(!CloseHandle(m_Handle)) {
      m_Handle = INVALID_HANDLE_VALUE;
      throw CWinApiException("CPipeClient::ClosePipe: CloseHandle failed.",
                             GetLastError());
    }
    m_Handle = INVALID_HANDLE_VALUE;
  }
}


// CPipeServer /////////////////////////////////////////////////////////////////

CPipeServerConnectionThread* CPipeServer::WaitForConnection(
    const char* pipeName,
    DWORD pipeTimeOut) {


  CPipeServerConnectionThread* result = nullptr;

  // https://docs.microsoft.com/en-us/windows/win32/ipc/multithreaded-pipe-server

  BOOL fConnected = FALSE;
  HANDLE hPipe = INVALID_HANDLE_VALUE;

 hPipe = CreateNamedPipeA(pipeName, PIPE_ACCESS_DUPLEX,
                           PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |
                               PIPE_REJECT_REMOTE_CLIENTS,
                           PIPE_UNLIMITED_INSTANCES, 0, 0, pipeTimeOut, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
      throw CWinApiException("CreateNamedPipeA failed", GetLastError());
    }

    fConnected = ConnectNamedPipe(hPipe, NULL)
                     ? TRUE
                     : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (fConnected) {
      result = OnNewConnection(hPipe);
      if (nullptr == result) {
        CloseHandle(hPipe);
        throw "CPipeServer::WaitForConnection: OnNewConnection returned no thread";
      }
    } else {
      // The client could not connect, so close the pipe.
      CloseHandle(hPipe);
    }

    return result;
}
 
class CNamedPipeServer {
 private:
  HANDLE m_PipeHandle = INVALID_HANDLE_VALUE;

  DWORD m_ReadTimeoutMs;
  DWORD m_WriteTimeoutMs;

  std::mutex m_PipeMutex;

 public:
  CNamedPipeServer(DWORD readTimeOutMs = TEN_MINUTES_IN_MILLISECONDS,
                   DWORD writeTimeOutMs = TEN_MINUTES_IN_MILLISECONDS)
      : m_ReadTimeoutMs(readTimeOutMs), m_WriteTimeoutMs(writeTimeOutMs) {


  }

  ~CNamedPipeServer() {
    Close();
  }

  bool OpenPipe(const char* pipeName) {

    std::unique_lock<std::mutex> lock(m_PipeMutex);

    if (m_PipeHandle != INVALID_HANDLE_VALUE)
        return false;

    std::string strPipeName("\\\\.\\pipe\\");
    strPipeName.append(pipeName);

    m_PipeHandle = CreateNamedPipeA(strPipeName.c_str(), PIPE_ACCESS_DUPLEX,
                                    PIPE_READMODE_BYTE | PIPE_TYPE_BYTE |
                                        PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                                    1, 0, 0, TEN_MINUTES_IN_MILLISECONDS, NULL);

    return m_PipeHandle != INVALID_HANDLE_VALUE;
  }

  bool Connect() {
    std::unique_lock<std::mutex> lock(m_PipeMutex);

    if (m_PipeHandle != INVALID_HANDLE_VALUE) {
      BOOL fConnected = ConnectNamedPipe(m_PipeHandle, NULL)
                            ? TRUE
                            : (GetLastError() == ERROR_PIPE_CONNECTED);

      if (fConnected)
        return true;

    }

    return false;
  }

  bool ReadBytes(LPVOID bytes, DWORD offset, DWORD length) {
    while (0 < length) {
      DWORD bytesRead = 0;

      if (!(ReadFile(m_PipeHandle, (unsigned char*)bytes + offset, length,
                     &bytesRead, NULL))) {
        DWORD lastError = GetLastError();
        switch (lastError) {
          case ERROR_MORE_DATA:
            break;
          case ERROR_INVALID_HANDLE:
            m_PipeHandle = INVALID_HANDLE_VALUE;
          default:
            return false;
        }
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
    while (0 < length) {
      DWORD bytesWritten = 0;

      if (!(WriteFile(m_PipeHandle, (unsigned char*)bytes + offset, length,
                      &bytesWritten, NULL))) {
        DWORD lastError = GetLastError();
        switch (lastError) {
          case ERROR_MORE_DATA:
            break;
          case ERROR_INVALID_HANDLE:
            m_PipeHandle = INVALID_HANDLE_VALUE;
          default:
            return false;
        }
      }

      offset += bytesWritten;
      length -= bytesWritten;
    }
    return true;
  }

  bool Flush() {

  if (!FlushFileBuffers(m_PipeHandle)) {
      DWORD lastError = GetLastError();
      switch (lastError) {
        case ERROR_INVALID_HANDLE:
          m_PipeHandle = INVALID_HANDLE_VALUE;
        default:
          return false;
      }
    }

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

  bool WriteSingle(FLOAT value) {
      return WriteBytes(&value, 0, sizeof(value));
  }

  bool WriteStringUTF8(const std::string &value) {
    UINT32 length = (UINT32)value.length();

    return WriteCompressedUInt32(length) &&
           WriteBytes((LPVOID)value.c_str(), 0, length);
  }

  void Close()
  {
    std::unique_lock<std::mutex> lock(m_PipeMutex);

    if (INVALID_HANDLE_VALUE != m_PipeHandle)
    {
      CloseHandle(m_PipeHandle);
      m_PipeHandle = INVALID_HANDLE_VALUE;
    }
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

  explicit CAfxTask(fp_t&& op) : m_Op(std::move(op)) {}

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
  enum class ValueType { Invalid, Array, Object, Bool, Int, UInt, Double, V8Function, String, Time };

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

  
  static CefRefPtr<CAfxValue> CreateTime(time_t value) {
    CefRefPtr<CAfxValue> result = new CAfxValue();
    result->m_Value.Time = value;
    result->m_ValueType = ValueType::Time;
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

  bool IsTime() { return m_ValueType == ValueType::Time; }

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

    time_t GetTime() {
    if (m_ValueType == ValueType::Time)
      return m_Value.Time;

    return false;
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

  const std::string & GetString() {
    if (m_ValueType == ValueType::String)
      return m_String;
    return m_String;
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
        case ValueType::Time:
          return CefV8Value::CreateDate(CefTime(m_Value.Time));
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
    time_t Time;
  } m_Value;
  CefRefPtr<CefV8Value> m_Function;
  std::string m_String;

  IMPLEMENT_REFCOUNTING(CAfxValue);
  DISALLOW_COPY_AND_ASSIGN(CAfxValue);
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


class CAfxObject;

class CAfxObjectBase : public CefBaseRefCounted {
public:
  virtual CAfxObject * AsRef() { return nullptr; }
};

class CAfxObject : public CAfxObjectBase, public CefV8Handler {
 public:
  static CefRefPtr<CefV8Value> Create(CefRefPtr<CAfxObject> self) {
    auto obj = CefV8Value::CreateObject(new CAfxObjectHandler(), nullptr);
    obj->SetUserData(static_cast<CAfxObjectBase*>(self.get()));

    return obj;
  }

    static void AddFunction(CefRefPtr<CefV8Value> object,
                          const char* name,
                          AfxExecute_t execute) {
    auto self = As(object);
    if (self == nullptr)
      return;

    self->m_ExecuteMap.emplace(name, execute);
    CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(name, self);
    object->SetValue(name, func, V8_PROPERTY_ATTRIBUTE_NONE);
    self->m_GetMap.emplace(
        name, [func](const CefString& name, const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& exception) { return func; });
  }

  static CefRefPtr<CAfxObject> As(CefRefPtr<CefV8Value> value) {
    if (nullptr == value || !value->IsObject())
      return nullptr;
    CefRefPtr<CefBaseRefCounted> user_data = value->GetUserData();
    if (nullptr == user_data)
      return nullptr;
    return static_cast<CAfxObjectBase*>(user_data.get())->AsRef();
  }

  template<AfxObjectType type, class T>
  static CefRefPtr<T> As(CefRefPtr<CefV8Value> value) {
    if (nullptr == value || !value->IsObject())
      return nullptr;
    CefRefPtr<CefBaseRefCounted> user_data = value->GetUserData();
    if (nullptr == user_data)
      return nullptr;

    auto obj = static_cast<CAfxObjectBase*>(user_data.get())->AsRef();

    if (type != obj->GetObjectType())
      return nullptr;

    return static_cast<T*>(obj);
  }

  static void AddGetter(CefRefPtr<CefV8Value> object,
                        const char* name,
                        AfxGet_t get) {
    auto self = As(object);
    if (self == nullptr)
      return;

    self->m_GetMap.emplace(name, get);
    object->SetValue(name, V8_ACCESS_CONTROL_DEFAULT,
                     V8_PROPERTY_ATTRIBUTE_NONE);
  }

  static void AddSetter(CefRefPtr<CefV8Value> object,
                        const char* name,
                        AfxSet_t set) {
    auto self = As(object);
    if (self == nullptr)
      return;

    self->m_SetMap.emplace(name, set);
    object->SetValue(name, V8_ACCESS_CONTROL_DEFAULT,
                     V8_PROPERTY_ATTRIBUTE_NONE);
  }

  static CefRefPtr<CAfxCallback> AddCallback(CefRefPtr<CefV8Value> object,
                                             const char* name) {
    {
      CefRefPtr<CAfxCallback> callback = new CAfxCallback();

      AddSetter(
          object, name,
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

      AddGetter(
          object, name,
          [callback](const CefString& name, const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval, CefString& exception) {
            CefRefPtr<CefV8Value> func = callback->GetFunc();
            if (nullptr == func)
              retval = CefV8Value::CreateNull();
            else
              retval = func;

            return true;
          });

      object->SetValue(name, V8_ACCESS_CONTROL_DEFAULT,
                       V8_PROPERTY_ATTRIBUTE_NONE);

      return callback;
    }
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) OVERRIDE {
    auto self = As(object);
    if (self == nullptr) {
      exception = g_szInvalidThis;
      return true;
    }

    auto it = self->m_ExecuteMap.find(name);

    if (it != self->m_ExecuteMap.end())
      return it->second(name, object, arguments, retval, exception);

    return false;
  }

  CAfxObject(AfxObjectType obectType) : m_ObjectType(obectType) {}

  AfxObjectType GetObjectType() const { return m_ObjectType; }

  UINT64 GetIndex() const { return (UINT64)this; }

  virtual CAfxObject * AsRef() OVERRIDE { return this; }

 private:
  class CAfxObjectHandler : public CefV8Accessor {
   public:

    virtual bool Get(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& exception) OVERRIDE {
      auto self = As(object);
      if (self == nullptr) {
        exception = g_szInvalidThis;
        return true;
      }

      auto it = self->m_GetMap.find(name);

      if (it != self->m_GetMap.end())
        return it->second(name, object, retval, exception);

      return false;
    }

    virtual bool Set(const CefString& name,
                     const CefRefPtr<CefV8Value> object,
                     const CefRefPtr<CefV8Value> value,
                     CefString& exception) OVERRIDE {
      auto self = As(object);
      if (self == nullptr) {
        exception = g_szInvalidThis;
        return true;
      }

      auto it = self->m_SetMap.find(name);

      if (it != self->m_SetMap.end())
        return it->second(name, object, value, exception);

      return false;
    }

   private:
    IMPLEMENT_REFCOUNTING(CAfxObjectHandler);
  };

  AfxObjectType m_ObjectType;

  AfxExecuteMap_t m_ExecuteMap;
  AfxSetMap_t m_SetMap;
  AfxGetMap_t m_GetMap;

  IMPLEMENT_REFCOUNTING(CAfxObject);
};

class CAfxHandle : public CAfxObject {
  public:
      static HANDLE ToHandle(unsigned int lo, unsigned int hi) {
    return (void*)((unsigned __int64)lo | ((unsigned __int64)hi << 32));
  }

  static CefRefPtr<CefV8Value> Create(HANDLE handle,
                                      CefRefPtr<CAfxHandle>* out = nullptr) {
    auto obj = CAfxObject::Create(new CAfxHandle(handle));
    CAfxObject::AddGetter(
        obj, "lo",
        [](const CefString& name, const CefRefPtr<CefV8Value> object,
           CefRefPtr<CefV8Value>& retval, CefString& exception) {
          auto self =
              CAfxObject::As<AfxObjectType::AfxHandle, CAfxHandle>(
                  object);
          if (self == nullptr) {
            exception = g_szInvalidThis;
            return true;
          }

          retval = CefV8Value::CreateUInt(self->GetLo());
          return true;
        });
    CAfxObject::AddGetter(
        obj, "hi",
        [](const CefString& name, const CefRefPtr<CefV8Value> object,
               CefRefPtr<CefV8Value>& retval, CefString& exception) {
          auto self =
              CAfxObject::As<AfxObjectType::AfxHandle, CAfxHandle>(object);
          if (self == nullptr) {
            exception = g_szInvalidThis;
            return true;
          }

          retval = CefV8Value::CreateUInt(self->GetHi());
          return true;
        });
    CAfxObject::AddGetter(
        obj, "invalid",
        [](const CefString& name, const CefRefPtr<CefV8Value> object,
               CefRefPtr<CefV8Value>& retval, CefString& exception) {
          auto self =
              CAfxObject::As<AfxObjectType::AfxHandle, CAfxHandle>(object);
          if (self == nullptr) {
            exception = g_szInvalidThis;
            return true;
          }

          retval =
              CefV8Value::CreateBool(self->GetHandle() == INVALID_HANDLE_VALUE);
          return true;
        });

    if (out)
      *out = CAfxObject::As<AfxObjectType::AfxHandle,  CAfxHandle>(obj);

    return obj;
  }

  CAfxHandle()
      : CAfxObject(AfxObjectType::AfxHandle),
        m_Handle(INVALID_HANDLE_VALUE) {     
  }

  CAfxHandle(HANDLE value)
      : CAfxObject(AfxObjectType::AfxHandle), m_Handle(value) {}

  HANDLE GetHandle() { return m_Handle;
  }

  void SetHandle(HANDLE value) {
      m_Handle = value;
  }

  unsigned int GetLo() {        
      return (unsigned int)(((unsigned __int64)m_Handle) & 0xffffffff);
  }

  unsigned int GetHi() {
    return (unsigned int)((((unsigned __int64)m_Handle) >> 32) & 0xffffffff);
  }

  private:
      HANDLE m_Handle;

  IMPLEMENT_REFCOUNTING(CAfxHandle);
};

 class CAfxData : public CAfxObject,
                  public CefV8ArrayBufferReleaseCallback {
  public:
  static CefRefPtr<CefV8Value> Create(unsigned int size,
                                      void* data,
                                      CefRefPtr<CAfxData> *out = nullptr) {
    CefRefPtr<CAfxData> dataObj = new CAfxData(size, data);

    auto buffer = CefV8Value::CreateArrayBuffer(
        data, size, dataObj);

    if (out)
      *out = dataObj;

    return buffer;
  }

  size_t GetSize() { return m_Size; }

  void* GetData() { return m_Data; }


  private:
   CAfxData(unsigned int size, void* data)
       : CAfxObject(AfxObjectType::AfxData),
        m_Size(size),
        m_Data(data) {
  }

  size_t m_Size;
  void* m_Data;

  virtual void ReleaseBuffer(void* buffer) override { free(buffer); }

  IMPLEMENT_REFCOUNTING(CAfxData);
};

class CCalcCallbacksGuts {
public:
  ~CCalcCallbacksGuts() {
    while (!m_Map.empty()) {
      while (!m_Map.begin()->second.empty()) {
        CefRefPtr<CAfxCallback> callback = (*m_Map.begin()->second.begin());
        callback->Release();
        m_Map.begin()->second.erase(m_Map.begin()->second.begin());
      }
      m_Map.erase(m_Map.begin());
    }
  }

  void Add(const char* name, CefRefPtr<CAfxCallback> callback) {
    callback->AddRef();
    m_Map[name].emplace(callback);
  }

  void Remove(const char* name, CefRefPtr<CAfxCallback> callback) {
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
  typename typedef std::set<CefRefPtr<CAfxCallback>> Callbacks_t;
  typedef std::map<std::string, Callbacks_t> NameToCallbacks_t;
  NameToCallbacks_t m_Map;
};

template <typename R>
class CCalcCallbacks abstract : public CCalcCallbacksGuts {
 public:
  bool BatchUpdateRequest(CNamedPipeServer & pipeServer) {
    if (!pipeServer.WriteCompressedUInt32((UINT32)m_Map.size()))
      return false;
    for (typename std::map<std::string, std::set<CefRefPtr<CAfxCallback>>>::iterator it =
             m_Map.begin();
         it != m_Map.end(); ++it) {
      if (!pipeServer.WriteStringUTF8(it->first))
        return false;
    }

    return true;
  }

  bool BatchUpdateResult(CefRefPtr<CInterop> interop, CNamedPipeServer & pipeServer) {
    CefRefPtr<R> result;

    for (typename std::map<std::string, std::set<CefRefPtr<CAfxCallback>>>::iterator it =
             m_Map.begin();
         it != m_Map.end(); ++it) {
      bool hasResult;

      if (!pipeServer.ReadBoolean(hasResult))
        return false;

      if (hasResult) {
        result = new R();
        if (!ReadResult(pipeServer, result))
          return false;
      }

      for (typename std::set<CefRefPtr<CAfxCallback>>::iterator setIt = (*it).second.begin(); setIt != (*it).second.end(); ++setIt) {
        CefPostTask(TID_RENDERER,
                    new CAfxTask([this, interop, callback = *setIt, result]() {
                      if (nullptr == interop->m_Context)
                        return;

          interop->m_Context->Enter();
          CallResult(callback, result);
            interop->m_Context->Exit();
        }));
      }
    }

    return true;
  }

 protected:
  virtual bool ReadResult(CNamedPipeServer & pipeServer, CefRefPtr<R> outResult) = 0;
  virtual void CallResult(CefRefPtr<CAfxCallback> callback, CefRefPtr<R> result) = 0;
};

class CHandleCalcCallbacks
    : public CCalcCallbacks<struct HandleCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer & pipeServer,
                          CefRefPtr<struct HandleCalcResult_s> outResult) override {
    if (!pipeServer.ReadInt32(outResult->IntHandle))
      return false;

    return true;
  }

  virtual void CallResult(CefRefPtr<CAfxCallback> callback,
                          CefRefPtr<struct HandleCalcResult_s> result) override {

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
  virtual bool ReadResult(CNamedPipeServer & pipeServer,
                         CefRefPtr<struct VecAngCalcResult_s> outResult) override {
    if (!pipeServer.ReadSingle(outResult->Vector.X))
      return false;
    if (!pipeServer.ReadSingle(outResult->Vector.Y))
      return false;
    if (!pipeServer.ReadSingle(outResult->Vector.Z))
      return false;

    if (!pipeServer.ReadSingle(outResult->QAngle.Pitch))
      return false;
    if (!pipeServer.ReadSingle(outResult->QAngle.Yaw))
      return false;
    if (!pipeServer.ReadSingle(outResult->QAngle.Roll))
      return false;

    return true;
  }

  virtual void CallResult(CefRefPtr<CAfxCallback> callback,
                          CefRefPtr<struct VecAngCalcResult_s> result) override {

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
  virtual bool ReadResult(CNamedPipeServer & pipeServer,
                          CefRefPtr<struct CamCalcResult_s> outResult) override {
    if (!pipeServer.ReadSingle(outResult->Vector.X))
      return false;
    if (!pipeServer.ReadSingle(outResult->Vector.Y))
      return false;
    if (!pipeServer.ReadSingle(outResult->Vector.Z))
      return false;

    if (!pipeServer.ReadSingle(outResult->QAngle.Pitch))
      return false;
    if (!pipeServer.ReadSingle(outResult->QAngle.Yaw))
      return false;
    if (!pipeServer.ReadSingle(outResult->QAngle.Roll))
      return false;

    if (!pipeServer.ReadSingle(outResult->Fov))
      return false;

    return true;
  }

  virtual void CallResult(CefRefPtr<CAfxCallback> callback,
                          CefRefPtr<struct CamCalcResult_s> result) override {
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
  virtual bool ReadResult(CNamedPipeServer & pipeServer,
                          CefRefPtr<struct FovCalcResult_s> outResult) override {
    if (!pipeServer.ReadSingle(outResult->Fov))
      return false;

    return true;
  }

  virtual void CallResult(CefRefPtr<CAfxCallback> callback,
                          CefRefPtr<struct FovCalcResult_s> result) override {
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
  virtual bool ReadResult(CNamedPipeServer & pipeServer,
                          CefRefPtr<struct BoolCalcResult_s> outResult) override {
    bool result;
    if (!pipeServer.ReadBoolean(result))
      return false;

    outResult->Result = result;

    return true;
  }

  virtual void CallResult(CefRefPtr<CAfxCallback> callback,
                          CefRefPtr<struct BoolCalcResult_s> result) override {
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
  virtual bool ReadResult(CNamedPipeServer & pipeServer,
                          CefRefPtr<struct IntCalcResult_s> outResult) override {
    INT32 result;
    if (!pipeServer.ReadInt32(result))
      return false;

    outResult->Result = result;

    return true;
  }

  virtual void CallResult(CefRefPtr<CAfxCallback> callback,
                          CefRefPtr<struct IntCalcResult_s> result) override {
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
  CAfxInterop(const char* pipeName)
      : m_PipeName(pipeName),
        m_PipeServer(TEN_MINUTES_IN_MILLISECONDS, TEN_MINUTES_IN_MILLISECONDS) {
  }

  virtual bool Connection(DWORD readTimeOutMs = TEN_MINUTES_IN_MILLISECONDS, DWORD writeTimeOutMs = TEN_MINUTES_IN_MILLISECONDS) {

    Close();

    if (m_PipeServer.OpenPipe(m_PipeName.c_str())) {
      if (m_PipeServer.Connect()) {
        m_Connected = true;
        OnConnect();
        return true;
      }
    }

    return false;
  }

  virtual void Cancel() {
    CancelSynchronousIo(m_PipeQueue.GetNativeThreadHandle());
  }


  virtual void Close() {
    if(m_Connected) {
      OnClose();
      m_Connected = false;   
    }

    m_PipeServer.Close();
  }

  virtual void SetPipeName(const char* value) { m_PipeName = value; }

  bool GetConnected() {
    return m_Connected;
  }

 protected:
  std::string m_PipeName;
  CNamedPipeServer m_PipeServer;
  CThreadedQueue m_PipeQueue;
  bool m_Connected = false;

  virtual ~CAfxInterop() {
    Close();
  }

  virtual void OnConnect() {

  }

  virtual void OnClose() {

  }
};

class CDrawingInteropImpl : public CAfxObject,
                            public CInterop,
                            public CAfxInterop,
                            public CPipeServer,
                            public CPipeClient {
  IMPLEMENT_REFCOUNTING(CDrawingInteropImpl);

 public:
  static CefRefPtr<CefV8Value> Create(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context,
                                      const CefString& argStr,
                                      DWORD handlerId,
                                      CefRefPtr<CInterop>* out = nullptr) {
    CefRefPtr<CDrawingInteropImpl> self =
        new CDrawingInteropImpl(browser->GetIdentifier());

    std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_handler_");
    strPipeName.append(std::to_string(handlerId));

    try {
      while(true)
      {
      bool bError = false;
      try {
        self->OpenPipe(strPipeName.c_str(), INFINITE);
        Sleep(100);
      }
      catch(const std::exception&)
      {
        bError = true;
      }

      if(!bError) break;
      }
      self->WriteUInt32(GetCurrentProcessId());
      self->WriteInt32(browser->GetIdentifier());
      self->Flush();
    } catch (const std::exception& e) {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << e.what();
      if (out)
        *out = nullptr;
      return CefV8Value::CreateNull();
    }

    auto obj = CAfxObject::Create(self);

    //

    self->m_Context = context;

    CAfxObject::AddGetter(
        obj,
        "id",
        [](const CefString& name, const CefRefPtr<CefV8Value> object,
                  CefRefPtr<CefV8Value>& retval, CefString& exception) {

          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                       CDrawingInteropImpl>(
                  object);
          if (self == nullptr) {
            exception = g_szInvalidThis;
            return true;
          }

          retval = CefV8Value::CreateInt(self->m_BrowserId);
          return true;
        });

    CAfxObject::AddGetter(
        obj,
      "args",
      [argStr](const CefString& name, const CefRefPtr<CefV8Value> object,
        CefRefPtr<CefV8Value>& retval, CefString& exception) {
        retval = CefV8Value::CreateString(argStr);
        return true;
    });

CAfxObject::AddFunction(
        obj,
        "sendMessage",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsInt() &&
              arguments[3]->IsString()) {
            self->m_InteropQueue.Queue([self, fn_resolve = arguments[0],
                                      fn_reject = arguments[1],
                                      id = arguments[2]->GetIntValue(),
                                      str = arguments[3]->GetStringValue()]() {
                  try {
                    self->WriteInt32((int)HostMessage::Message);
                    self->WriteInt32(id);
                    self->WriteStringUTF8(str.ToString().c_str());
                    self->Flush();

                    CefPostTask(
                        TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));
                  } catch (const std::exception& e) {
                    CefPostTask(
                        TID_RENDERER,
                        new CAfxTask([self, fn_reject,
                                      error_msg = std::string(e.what())]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateString(error_msg));
                          fn_reject->ExecuteFunction(NULL, args);
                          self->m_Context->Exit();
                        }));

                  }
                });

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    self->m_OnMessage = CAfxObject::AddCallback(obj, "onMessage");

    self->m_OnAcceleratedPaint =
        CAfxObject::AddCallback(obj, "onAcceleratedPaint");
    self->m_OnReleaseShareHandle =
        CAfxObject::AddCallback(obj, "onReleaseShareHandle");

    self->m_OnError = CAfxObject::AddCallback(obj, "onError");
 
    CAfxObject::AddFunction(
        obj, "setPipeName",
        [](const CefString& name,
                                              CefRefPtr<CefV8Value> object,
                                              const CefV8ValueList& arguments,
                                              CefRefPtr<CefV8Value>& retval,
                                              CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

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

    self->m_OnDeviceLost = CAfxObject::AddCallback(obj, "onDeviceLost");

    self->m_OnDeviceReset = CAfxObject::AddCallback(obj, "onDeviceReset");

    CAfxObject::AddFunction(
        obj, "connect",
        [](const CefString& name,
                                             CefRefPtr<CefV8Value> object,
                                             const CefV8ValueList& arguments,
                                             CefRefPtr<CefV8Value>& retval,
                                             CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

      if (2 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction()) {
        self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                 fn_reject = arguments[1]]() {
          if (self->Connection()) {

            CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));
          } else {
            CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));
          }
        });
        return true;
      }
      exceptionoverride = g_szInvalidArguments;
      return true;
    });

  CAfxObject::AddFunction(
        obj, "cancel",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          self->Cancel();

          return true;
        });

    CAfxObject::AddFunction(
        obj, "close",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (2 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0]]() {
              self->Close();
              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }
          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, "pumpBegin",
        [](const CefString& name,
                                               CefRefPtr<CefV8Value> object,
                                               const CefV8ValueList& arguments,
                                               CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

      if (4 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction() && arguments[2]->IsInt() &&
          arguments[3]->IsUInt()) {
            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                 fn_reject = arguments[1], frameCount = arguments[2]->GetIntValue(),
                                 pass = arguments[3]->GetIntValue()]() {

          int errorLine = __LINE__;

          if (self->GetConnected() && 0 == (errorLine = self->DoPumpBegin(frameCount, pass))) {
            bool inFlow = self->m_InFlow;
            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_resolve, result = inFlow]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateBool(result));
                          fn_resolve->ExecuteFunction(NULL, args);
                          self->m_Context->Exit();
                        }));
          } else {
            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_reject, errorLine]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateString("AfxInterop.cpp:"+std::to_string(errorLine)));
                          fn_reject->ExecuteFunction(NULL, args);
                          self->m_Context->Exit();
                        }));
          }
        });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });

    CAfxObject::AddFunction(
        obj, "pumpEnd",
        [](const CefString& name,
                                             CefRefPtr<CefV8Value> object,
                                             const CefV8ValueList& arguments,
                                             CefRefPtr<CefV8Value>& retval,
                                             CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

     if (2 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction()) {
            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                 fn_reject = arguments[1]]() {
         if (!self->m_InFlow)
              goto error;

            if (!self->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::Finished))
              goto error;

            if (!self->m_PipeServer.Flush())
              goto error;


            CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          self->m_Context->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));
            return;

          error:
          self->Close();

            CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();
                       fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                        self->m_Context->Exit();
                     }));
        });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
      });

    CAfxObject::AddFunction(
        obj, "d3d9CreateVertexDeclaration",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && (arguments[2]->IsArrayBuffer() || arguments[2]->IsNull()) && arguments[3]->IsArray() &&
              1 <= arguments[3]->GetArrayLength()) {
            CefRefPtr<CAfxData> data =
                arguments[2]->IsNull() ? nullptr: static_cast<CAfxData*>(
                arguments[2]->GetArrayBufferReleaseCallback().get());

            if (nullptr != data) {
              CefRefPtr<CAfxD3d9VertexDeclaration> val;
              auto retobj = CAfxD3d9VertexDeclaration::Create(self, &val);

              self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1], data, val,
                                       refVertexDeclaration = arguments[3],
                                       retobj]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9CreateVertexDeclaration))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt64((UINT64)val->GetIndex()))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)data->GetSize()))
                  goto __error;
                if (!self->m_PipeServer.WriteBytes(data->GetData(), 0,
                                                    (DWORD)data->GetSize()))
                  goto __error;

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                  CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve,
                                          refVertexDeclaration, retobj, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                                self->m_Context->Enter();
                              refVertexDeclaration->SetValue(0, retobj);

                                CefV8ValueList args;
                                args.push_back(CefV8Value::CreateInt(hr));
                                fn_resolve->ExecuteFunction(
                                    NULL, args);
                                self->m_Context->Exit();
                              }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });

              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9CreateIndexBuffer",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (8 <= arguments.size() && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt() &&
              arguments[4]->IsUInt() && arguments[5]->IsUInt()
              && arguments[6]->IsArray() && 1 <= arguments[6]->GetArrayLength()
              && arguments[7]->IsArray() && 1 <= arguments[7]->GetArrayLength()) {

            CefRefPtr<CAfxD3d9IndexBuffer> val;
            auto retobj = CAfxD3d9IndexBuffer::Create(self, &val);

            auto drawingHandle =
                CAfxObject::As<AfxObjectType::AfxData, CAfxHandle>(
                    arguments[7]);

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       index = val->GetIndex(),
                                       length = arguments[2]->GetUIntValue(),
                                       usage = arguments[3]->GetUIntValue(),
                                       format = arguments[4]->GetUIntValue(),
                                       pool = arguments[5]->GetUIntValue(),
                                       refIndexBuffer = arguments[6],
                                       refHandle = arguments[7]
                                       , drawingHandle, retobj]() {

              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreateIndexBuffer))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32((UINT64)index))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)length))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)usage))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)format))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)pool))
                goto __error;
              if (nullptr == drawingHandle) {
                if (!self->m_PipeServer.WriteBoolean(false))
                  goto __error;
              } else {
                if (!self->m_PipeServer.WriteBoolean(true))
                  goto __error;
                if (!self->m_PipeServer.WriteHandle(drawingHandle->GetHandle()))
                  goto __error;
              }
              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;

              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);

                      self->m_Context->Exit();
                    }));
                return;
              }

              HANDLE tmpHandle;

              if (nullptr != drawingHandle) {
                if (!self->m_PipeServer.ReadHandle(tmpHandle))
                  goto __error;
              }

              CefPostTask(TID_RENDERER,
                          new CAfxTask([self, fn_resolve, hr,
                                        tmpHandle, refIndexBuffer, refHandle, retobj,
                                        drawingHandle]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();

                            refIndexBuffer->SetValue(0, retobj);
                            if (nullptr != drawingHandle) {
                              refHandle->SetValue(
                                  0, CAfxHandle::Create(tmpHandle, nullptr));
                            }

                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });

        return true;
      }
          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9CreateVertexBuffer",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (8 <= arguments.size() && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt() &&
              arguments[4]->IsUInt() && arguments[5]->IsUInt()
              && arguments[6]->IsArray() && 1 <= arguments[6]->GetArrayLength()
              && arguments[7]->IsArray() && 1 <= arguments[7]->GetArrayLength()) {

            CefRefPtr<CAfxD3d9VertexBuffer> val;
            auto retobj = CAfxD3d9VertexBuffer::Create(self, &val);

            auto drawingHandle =
                CAfxObject::As<AfxObjectType::AfxData, CAfxHandle>(
                    arguments[7]);

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       index = val->GetIndex(),
                                       length = arguments[2]->GetUIntValue(),
                                       usage = arguments[3]->GetUIntValue(),
                                       format = arguments[4]->GetUIntValue(),
                                       pool = arguments[5]->GetUIntValue(),
                                       refVertexBuffer = arguments[6],
                                       refHandle = arguments[7]
                                       , drawingHandle, retobj]() {

              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreateVertexBuffer))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32((UINT64)index))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)length))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)usage))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)format))
                goto __error;
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)pool))
                goto __error;
              if (nullptr == drawingHandle) {
                if (!self->m_PipeServer.WriteBoolean(false))
                  goto __error;
              } else {
                if (!self->m_PipeServer.WriteBoolean(true))
                  goto __error;
                if (!self->m_PipeServer.WriteHandle(drawingHandle->GetHandle()))
                  goto __error;
              }
              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;

              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);

                      self->m_Context->Exit();
                    }));
                return;
              }

              HANDLE tmpHandle;

              if (nullptr != drawingHandle) {
                if (!self->m_PipeServer.ReadHandle(tmpHandle))
                  goto __error;
              }

              CefPostTask(TID_RENDERER,
                          new CAfxTask([self, fn_resolve, hr,
                                        tmpHandle, refVertexBuffer, refHandle, retobj,
                                        drawingHandle]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();

                            refVertexBuffer->SetValue(0, retobj);
                            if (nullptr != drawingHandle) {
                              refHandle->SetValue(
                                  0, CAfxHandle::Create(tmpHandle, nullptr));
                            }

                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });

        return true;
      }
          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, 
        "d3d9CreateTexture",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
      if (10 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction() && arguments[2]->IsUInt() &&
          arguments[3]->IsUInt() && arguments[4]->IsUInt() &&
          arguments[5]->IsUInt() && arguments[6]->IsUInt() &&
          arguments[7]->IsUInt() && arguments[8]->IsArray() &&
          1 <= arguments[8]->GetArrayLength() && arguments[9]->IsArray() &&
          1 <= arguments[9]->GetArrayLength()) {
        CefRefPtr<CAfxD3d9Texture> val;
        auto retobj = CAfxD3d9Texture::Create(self, &val);

        CefRefPtr<CAfxHandle> drawingHandle =
            CAfxObject::As<AfxObjectType::AfxHandle, CAfxHandle>(
                arguments[9]->GetValue(0)); 

        self->m_PipeQueue.Queue(
            [self, fn_resolve = arguments[0], fn_reject = arguments[1],
             index = val->GetIndex(), width = arguments[2]->GetUIntValue(),
             height = arguments[3]->GetUIntValue(),
             levels = arguments[4]->GetUIntValue(),
             usage = arguments[5]->GetUIntValue(),
             format = arguments[6]->GetUIntValue(),
             pool = arguments[7]->GetUIntValue(), refTexture = arguments[8],
             refHandle = arguments[9], drawingHandle, retobj]() {

              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9CreateTexture))
                goto __error;
              if (!self->m_PipeServer.WriteUInt64((UINT64)index))
                goto __error;
              // width:
              if (!self->m_PipeServer.WriteUInt32((UINT32)width))
                goto __error;
              // height:
              if (!self->m_PipeServer.WriteUInt32((UINT32)height))
                goto __error;
              // levels:
              if (!self->m_PipeServer.WriteUInt32((UINT32)levels))
                goto __error;
              // usage:
              if (!self->m_PipeServer.WriteUInt32((UINT32)usage))
                goto __error;
              // format:
              if (!self->m_PipeServer.WriteUInt32((UINT32)format))
                goto __error;
              // pool:
              if (!self->m_PipeServer.WriteUInt32((UINT32)pool))
                goto __error;
              if (nullptr == drawingHandle) {
                if (!self->m_PipeServer.WriteBoolean(false))
                  goto __error;
              } else {
                if (!self->m_PipeServer.WriteBoolean(true))
                  goto __error;
                if (!self->m_PipeServer.WriteHandle(
                        drawingHandle->GetHandle()))
                  goto __error;
              }
              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;

              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);

                      self->m_Context->Exit();
                    }));
                return;
              }

              HANDLE tmpHandle;

              if (nullptr != drawingHandle) {
                if (!self->m_PipeServer.ReadHandle(tmpHandle))
                  goto __error;
              }

              CefPostTask(TID_RENDERER,
                          new CAfxTask([self, fn_resolve, hr,
                                        tmpHandle, refTexture, refHandle, retobj,
                                        drawingHandle]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();

                            refTexture->SetValue(0, retobj);
                            if (nullptr != drawingHandle) {
                              refHandle->SetValue(
                                  0, CAfxHandle::Create(tmpHandle, nullptr));
                            }

                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });
    
    CAfxObject::AddFunction(
        obj, 
        "d3d9CreateVertexShader",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && (arguments[2]->IsArrayBuffer() || arguments[2]->IsNull()) && arguments[3]->IsArray() &&
              1 <= arguments[3]->GetArrayLength()) {
            auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                     CDrawingInteropImpl>(object);
            if (self == nullptr) {
              exceptionoverride = g_szInvalidThis;
              return true;
            }
            CefRefPtr<CAfxData> data =
                arguments[2]->IsNull()
                    ? nullptr
                    : static_cast<CAfxData*>(
                arguments[2]->GetArrayBufferReleaseCallback().get());

            if (nullptr != data) {
              CefRefPtr<CAfxD3d9VertexShader> val;
              auto retobj = CAfxD3d9VertexShader::Create(self, &val);

              self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1], data, index = val->GetIndex(),
                                       refVertexShader = arguments[3],
                                       retobj]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9CreateVertexShader))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt64((UINT64)index))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)data->GetSize()))
                  goto __error;
                if (!self->m_PipeServer.WriteBytes(data->GetData(), 0,
                                                    (DWORD)data->GetSize()))
                  goto __error;

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }


                  CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, refVertexShader,
                                          retobj, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();

                              refVertexShader->SetValue(0, retobj);

                              CefV8ValueList args;

                              args.push_back(CefV8Value::CreateInt(hr));

                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });

              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    CAfxObject::AddFunction(
        obj, 
        "d3d9CreatePixelShader",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && (arguments[2]->IsArrayBuffer() || arguments[2]->IsNull()) && arguments[3]->IsArray() &&
              1 <= arguments[3]->GetArrayLength()) {
            CefRefPtr<CAfxData> data =
                arguments[2]->IsNull()
                    ? nullptr
                    : static_cast<CAfxData*>(
                arguments[2]->GetArrayBufferReleaseCallback().get());

            if (nullptr != data) {
              CefRefPtr<CAfxD3d9PixelShader> val;
              auto retobj = CAfxD3d9PixelShader::Create(self, &val);

              self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1], data, val,
                                       refPixelShader = arguments[3],
                                       retobj]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9CreatePixelShader))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt64((UINT64)val->GetIndex()))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)data->GetSize()))
                  goto __error;
                if (!self->m_PipeServer.WriteBytes(data->GetData(), 0,
                                                    (DWORD)data->GetSize()))
                  goto __error;

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }



                  CefPostTask(TID_RENDERER,
                              new CAfxTask([self, fn_resolve, refPixelShader,
                                          retobj, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                                self->m_Context->Enter();
                              refPixelShader->SetValue(0, retobj);

                                CefV8ValueList args;
                                args.push_back(CefV8Value::CreateInt(hr));
                                fn_resolve->ExecuteFunction(
                                    NULL, args);
                                self->m_Context->Exit();
                              }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });

              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9UpdateTexture",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()&& arguments[0]->IsFunction() && arguments[1]->IsFunction()) {
            CefRefPtr<CAfxD3d9Texture> sourceTexture =
                CAfxObject::As<AfxObjectType::AfxD3d9Texture,
                               CAfxD3d9Texture>(arguments[2]);
            CefRefPtr<CAfxD3d9Texture> destinationTexture =
                CAfxObject::As<AfxObjectType::AfxD3d9Texture,
                               CAfxD3d9Texture>(arguments[3]);

              self->m_PipeQueue.Queue([self,
                fn_resolve = arguments[0], fn_reject = arguments[1],
                sourceTextureIndex = sourceTexture == nullptr ? 0 : sourceTexture->GetIndex(),
                destinationTextureIndex = destinationTexture == nullptr ? 0 : destinationTexture->GetIndex()](){

                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9UpdateTexture))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt64(sourceTextureIndex))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt64(destinationTextureIndex))
                  goto __error;                  

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;

                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);

                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();

                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));


              });

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, 
        "d3d9SetVertexDeclaration",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (3 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
            CefRefPtr<CAfxD3d9VertexDeclaration> val =
                CAfxObject::As<AfxObjectType::AfxD3d9VertexDeclaration,
                               CAfxD3d9VertexDeclaration>(arguments[2]);

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1], val]() {
              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexDeclaration))
                goto __error;
              if (!self->m_PipeServer.WriteUInt64(val ? (UINT64)val->GetIndex()
                                                       : 0))
                goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, 
        "d3d9SetViewport",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (2 == arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1]]() {
              if (!self->m_InFlow)
                goto __error;

            if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetViewport))
                goto __error;
              if (!self->m_PipeServer.WriteBoolean(false))
              goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

           if (3 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsObject()) {

            auto x = arguments[2]->GetValue("x");
            auto y = arguments[2]->GetValue("y");
            auto width = arguments[2]->GetValue("width");
            auto height = arguments[2]->GetValue("height");
            auto minZ = arguments[2]->GetValue("minZ");
            auto maxZ = arguments[2]->GetValue("maxZ");

            if (nullptr != x && nullptr != y && nullptr != width &&
                nullptr != height && nullptr != minZ && nullptr != maxZ &&
                x->IsUInt() &&
                y->IsUInt() && width->IsUInt() && height->IsUInt() &&
                minZ->IsDouble() && maxZ->IsDouble()) {
              self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       v_x = x->GetUIntValue(),
                                       v_y = y->GetUIntValue(),
                                       v_width = width->GetUIntValue(),
                                       v_height = height->GetUIntValue(),
                                       v_minz = minZ->GetDoubleValue(),
                                       v_maxz = maxZ->GetDoubleValue()]() {
                if (!self->m_InFlow)
                  goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetViewport))
                  goto __error;
                if (!self->m_PipeServer.WriteBoolean(true))
                goto __error;
                if (!self->m_PipeServer.WriteUInt32(v_x))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(v_y))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(v_width))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(v_height))
                  goto __error;
                if (!self->m_PipeServer.WriteSingle(v_minz))
                  goto __error;
                if (!self->m_PipeServer.WriteSingle(v_maxz))
                  goto __error;

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });


    CAfxObject::AddFunction(
        obj, 
        "d3d9SetRenderState",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsUInt() &&
              arguments[3]->IsUInt()) {

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1], state = arguments[2]->GetUIntValue(), value = arguments[3]->GetUIntValue()]() {
              if (!self->m_InFlow)
                goto __error;

                          if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetRenderState))
                goto __error;
              // state
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)state))
                            goto __error;
              // value
              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)value))
                goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });



    CAfxObject::AddFunction(
        obj, 
        "d3d9SetSamplerState",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (5 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsUInt() &&
              arguments[3]->IsUInt() && arguments[4]->IsUInt()) {

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1],
                                     sampler = arguments[2]->GetUIntValue(),
                                     type = arguments[3]->GetUIntValue(),
                                     value = arguments[4]->GetUIntValue()]() {
              if (!self->m_InFlow)
                goto __error;

            if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetSamplerState))
                goto __error;
              // sampler
              if (!self->m_PipeServer.WriteUInt32((UINT32)sampler))
              goto __error;
              // type
              if (!self->m_PipeServer.WriteUInt32((UINT32)type))
                goto __error;
              // value
              if (!self->m_PipeServer.WriteUInt32((UINT32)value))
                goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });


 CAfxObject::AddFunction(
        obj, 
        "d3d9SetTexture",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsUInt()) {
            CefRefPtr<CAfxD3d9Texture> val =
                CAfxObject::As<AfxObjectType::AfxD3d9Texture,
                               CAfxD3d9Texture>(arguments[3]);

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1], sampler = arguments[2]->GetUIntValue(), val]() {
              if (!self->m_InFlow)
                goto __error;

            if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetTexture))
                goto __error;

              // sampler
              if (!self->m_PipeServer.WriteUInt32((UINT32)sampler))
              goto __error;
              // textureIndex
              if (!self->m_PipeServer.WriteUInt64(val ? (UINT64)val->GetIndex()
                                                       : 0))
                goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
     obj, 
        "d3d9SetTextureStageState",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
        CefString& exceptionoverride) {
       auto self =
           CAfxObject::As<AfxObjectType::DrawingInteropImpl, CDrawingInteropImpl>(
               object);
       if (self == nullptr) {
         exceptionoverride = g_szInvalidThis;
         return true;
       }
          if (5 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsUInt() &&
              arguments[3]->IsUInt() && arguments[4]->IsUInt()) {

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1],
                                     stage = arguments[2]->GetUIntValue(),
                                     type = arguments[3]->GetUIntValue(),
                                     value = arguments[4]->GetUIntValue()]() {
              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetTextureStageState))
                goto __error;
              // stage
              if (!self->m_PipeServer.WriteUInt32(stage))
                goto __error;
              // type
              if (!self->m_PipeServer.WriteUInt32(type))
                goto __error;
              // value
              if (!self->m_PipeServer.WriteUInt32(value))
                goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

CAfxObject::AddFunction(
        obj, 
        "d3d9SetTransform",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
      if (4 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction() && arguments[2]->IsUInt()) {
        if (arguments[3]->IsArray() && arguments[3]->GetArrayLength() == 16) {
          Matrix4x4_s matrix;

          bool bOk = true;
          for (int i = 0; i < 16; ++i) {
            auto arrVal = arguments[3]->GetValue(i);

            if (arrVal && arrVal->IsDouble()) {
              matrix[i] = (float)arrVal->GetDoubleValue();
            } else {
              bOk = false;
              break;
            }
          }
          if (bOk) {
            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1],
                                     state = arguments[2]->GetUIntValue(),
                                     matrix]() {
              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetTransform))
                goto __error;
              // state
              if (!self->m_PipeServer.WriteUInt32((UINT32)state))
                goto __error;
              if (!self->m_PipeServer.WriteBoolean(true))
                goto __error;

              for (int i = 0; i < 16; ++i) {
                if (!self->m_PipeServer.WriteSingle(matrix[i]))
                  goto __error;
              }

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }
        }
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });


    CAfxObject::AddFunction(obj,
        "d3d9SetIndices",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (3 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
            CefRefPtr<CAfxD3d9IndexBuffer> val =
                CAfxObject::As<AfxObjectType::AfxD3d9IndexBuffer,
                               CAfxD3d9IndexBuffer>(arguments[2]);


            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1], val]() {
              if (!self->m_InFlow)
                goto __error;                               

            if (!self->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetIndices))
              goto __error;
            if (!self->m_PipeServer.WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9SetStreamSource",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (6 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsUInt() &&
              arguments[4]->IsUInt() && arguments[5]->IsUInt()) {
            CefRefPtr<CAfxD3d9VertexBuffer> val =
                CAfxObject::As<AfxObjectType::AfxD3d9VertexBuffer,
                               CAfxD3d9VertexBuffer>(arguments[3]);

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1], val,
                                     streamNumber = arguments[2]->GetUIntValue(),
                                     offsetInBytes = arguments[4]->GetUIntValue(),
                                     stride = arguments[5]->GetUIntValue()]() {
              if (!self->m_InFlow)
                goto __error;                

            if (!self->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetStreamSource))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(streamNumber))
              goto __error;
            if (!self->m_PipeServer.WriteUInt64(val ? (UINT64)val->GetIndex()
                                                     : 0))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(offsetInBytes))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(stride))
              goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9SetStreamSourceFreq",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()  && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt()) {

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1],
                                     streamNumber = arguments[2]->GetUIntValue(),
                                     setting = arguments[3]->GetUIntValue()]() {
              if (!self->m_InFlow)
                goto __error;

            if (!self->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::D3d9SetStreamSourceFreq))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(streamNumber))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(setting))
              goto __error;


              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }


          exceptionoverride = g_szInvalidArguments;
          return true;
        });
   
    CAfxObject::AddFunction(
        obj, 
        "d3d9SetVertexShader",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (3 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
            CefRefPtr<CAfxD3d9VertexShader> val =
                CAfxObject::As<AfxObjectType::AfxD3d9VertexShader,
                               CAfxD3d9VertexShader>(arguments[2]);

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1], val]() {
              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetVertexShader))
                goto __error;
              if (!self->m_PipeServer.WriteUInt64(val ? (UINT64)val->GetIndex()
                                                       : 0))
                goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, 
        "d3d9SetPixelShader",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (3 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
            CefRefPtr<CAfxD3d9PixelShader> val =
                CAfxObject::As<AfxObjectType::AfxD3d9PixelShader,
                               CAfxD3d9PixelShader>(arguments[2]);

            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1], val]() {
              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::D3d9SetPixelShader))
                goto __error;
              if (!self->m_PipeServer.WriteUInt64(val ? (UINT64)val->GetIndex()
                                                       : 0))
                goto __error;

              if (!self->m_PipeServer.Flush())
                goto __error;

              int hr;
              if (!self->m_PipeServer.ReadInt32(hr))
                goto __error;

              if (FAILED(hr)) {
                unsigned int lastError;
                if (!self->m_PipeServer.ReadUInt32(lastError))
                  goto __error;
                CefPostTask(
                    TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();

                      CefRefPtr<CefV8Value> result =
                          CefV8Value::CreateObject(nullptr, nullptr);
                      result->SetValue("hr", CefV8Value::CreateInt(hr),
                                       V8_PROPERTY_ATTRIBUTE_NONE);
                      result->SetValue("lastError",
                                       CefV8Value::CreateUInt(lastError),
                                       V8_PROPERTY_ATTRIBUTE_NONE);

                      CefV8ValueList args;
                      args.push_back(result);
                      fn_resolve->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
                return;
              }

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve, hr]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateInt(hr));
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });


    CAfxObject::AddFunction(obj,
        "d3d9SetVertexShaderConstantB",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()
            && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsArray()) {
            size_t arrLen = arguments[3]->GetArrayLength();

            std::vector<bool> arr(arrLen);

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[3]->GetValue(i);

              if (arrVal && arrVal->IsBool()) {
                arr[i] = arrVal->GetBoolValue();
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              self->m_PipeQueue.Queue([self,
              fn_resolve = arguments[0], fn_reject = arguments[1], startRegister = arguments[2]->GetUIntValue(), arr]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetVertexShaderConstantB))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(startRegister))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)arr.size()))
                  goto __error;
                for (int i = 0; i < arr.size(); ++i) {
                  if (!self->m_PipeServer.WriteBoolean(arr[i]))
                    goto __error;
                }

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9SetVertexShaderConstantF",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()
            && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsArray()) {
            size_t arrLen = arguments[3]->GetArrayLength();

            std::vector<float> arr(arrLen);

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[3]->GetValue(i);

              if (arrVal && arrVal->IsDouble()) {
                arr[i] = (float)arrVal->GetDoubleValue();
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              self->m_PipeQueue.Queue([self,
              fn_resolve = arguments[0], fn_reject = arguments[1], startRegister = arguments[2]->GetUIntValue(), arr]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetVertexShaderConstantF))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(startRegister))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)arr.size()))
                  goto __error;
                for (int i = 0; i < arr.size(); ++i) {
                  if (!self->m_PipeServer.WriteSingle(arr[i]))
                    goto __error;
                }

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9SetVertexShaderConstantI",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()
            && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsArray()) {
            size_t arrLen = arguments[3]->GetArrayLength();

            std::vector<int> arr(arrLen);

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[3]->GetValue(i);

              if (arrVal && arrVal->IsInt()) {
                arr[i] = arrVal->GetIntValue();
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              self->m_PipeQueue.Queue([self,
              fn_resolve = arguments[0], fn_reject = arguments[1], startRegister = arguments[2]->GetUIntValue(), arr]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetVertexShaderConstantI))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(startRegister))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)arr.size()))
                  goto __error;
                for (int i = 0; i < arr.size(); ++i) {
                  if (!self->m_PipeServer.WriteInt32(arr[i]))
                    goto __error;
                }

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9SetPixelShaderConstantB",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()
            && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsArray()) {
            size_t arrLen = arguments[3]->GetArrayLength();

            std::vector<bool> arr(arrLen);

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[3]->GetValue(i);

              if (arrVal && arrVal->IsBool()) {
                arr[i] = arrVal->GetBoolValue();
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              self->m_PipeQueue.Queue([self,
              fn_resolve = arguments[0], fn_reject = arguments[1], startRegister = arguments[2]->GetUIntValue(), arr]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetPixelShaderConstantB))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(startRegister))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)arr.size()))
                  goto __error;
                for (int i = 0; i < arr.size(); ++i) {
                  if (!self->m_PipeServer.WriteBoolean(arr[i]))
                    goto __error;
                }

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9SetPixelShaderConstantF",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()
            && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsArray()) {
            size_t arrLen = arguments[3]->GetArrayLength();

            std::vector<float> arr(arrLen);

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[3]->GetValue(i);

              if (arrVal && arrVal->IsDouble()) {
                arr[i] = (float)arrVal->GetDoubleValue();
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              self->m_PipeQueue.Queue([self,
              fn_resolve = arguments[0], fn_reject = arguments[1], startRegister = arguments[2]->GetUIntValue(), arr]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetPixelShaderConstantF))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(startRegister))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)arr.size()))
                  goto __error;
                for (int i = 0; i < arr.size(); ++i) {
                  if (!self->m_PipeServer.WriteSingle(arr[i]))
                    goto __error;
                }

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9SetPixelShaderConstantI",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size()
            && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsArray()) {
            size_t arrLen = arguments[3]->GetArrayLength();

            std::vector<int> arr(arrLen);

            bool bOk = true;
            for (int i = 0; i < arrLen; ++i) {
              auto arrVal = arguments[3]->GetValue(i);

              if (arrVal && arrVal->IsInt()) {
                arr[i] = arrVal->GetIntValue();
              } else {
                bOk = false;
                break;
              }
            }
            if (bOk) {
              self->m_PipeQueue.Queue([self,
              fn_resolve = arguments[0], fn_reject = arguments[1], startRegister = arguments[2]->GetUIntValue(), arr]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::D3d9SetPixelShaderConstantI))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32(startRegister))
                  goto __error;
                if (!self->m_PipeServer.WriteUInt32((UINT32)arr.size()))
                  goto __error;
                for (int i = 0; i < arr.size(); ++i) {
                  if (!self->m_PipeServer.WriteInt32(arr[i]))
                    goto __error;
                }

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj,
        "d3d9DrawPrimitive",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (5 <= arguments.size()
          && arguments[0]->IsFunction() && arguments[1]->IsFunction()
          && arguments[2]->IsUInt() &&
              arguments[3]->IsUInt() && arguments[4]->IsUInt()) {

        self->m_PipeQueue.Queue(
            [self, fn_resolve = arguments[0], fn_reject = arguments[1],
            primitiveType = arguments[2]->GetUIntValue(),
            startVertex = arguments[3]->GetUIntValue(),
            primitiveCount = arguments[4]->GetUIntValue()](){

            if (!self->m_InFlow)
              goto __error;

            if (!self->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::D3d9DrawPrimitive))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(primitiveType))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(startVertex))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(primitiveCount))
              goto __error;

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
                 });

           return true;
            }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(obj, 
        "d3d9DrawIndexedPrimitive",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (8 <= arguments.size()
            && arguments[0]->IsFunction() && arguments[1]->IsFunction() &&
              arguments[2]->IsUInt() && arguments[3]->IsUInt() &&
              arguments[4]->IsUInt() && arguments[5]->IsUInt() &&
              arguments[6]->IsUInt() && arguments[7]->IsUInt()) {

        self->m_PipeQueue.Queue(
            [self, fn_resolve = arguments[0], fn_reject = arguments[1],
            primitiveType = arguments[2]->GetUIntValue(),
            baseVertexIndex = arguments[3]->GetUIntValue(),
            minVertexIndex = arguments[4]->GetUIntValue(),
            numVertices = arguments[5]->GetUIntValue(),
            startIndex = arguments[6]->GetUIntValue(),
            primCount = arguments[7]->GetUIntValue()](){

            if (!self->m_InFlow)
              goto __error;

            if (!self->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::D3d9DrawIndexedPrimitive))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(primitiveType))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(baseVertexIndex))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(minVertexIndex))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(numVertices))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(startIndex))
              goto __error;
            if (!self->m_PipeServer.WriteUInt32(primCount))
              goto __error;

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                        V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                        CefV8Value::CreateUInt(lastError),
                                        V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });
              return true;
            }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, "waitForClientGpu",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (2 <= arguments.size() && arguments[0]->IsFunction() &&
    arguments[1]->IsFunction()) {

    self->m_PipeQueue.Queue(
      [self, fn_resolve = arguments[0], fn_reject = arguments[1]]() {

        if (!self->m_InFlow)
          goto __error;

        if (!self->m_PipeServer.WriteUInt32((UINT32)DrawingReply::WaitForGpu))
          goto __error;
        if (!self->m_PipeServer.Flush())
              goto __error;

        bool waited;
        if (!self->m_PipeServer.ReadBoolean(waited) ||
            !waited  // this currently has to be always true.
        )
          goto __error;

        CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();
                      fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                      self->m_Context->Exit();
                    }));
        return;

      __error:
        self->Close();

        CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                      if (nullptr == self->m_Context)
                        return;

                      self->m_Context->Enter();
                      fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                      self->m_Context->Exit();
                    }));
    });
              return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, 
        "d3d9DrawPrimitiveUP",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (6 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsUInt() &&
              arguments[3]->IsUInt() && (arguments[4]->IsArrayBuffer() || arguments[4]->IsNull()) && arguments[5]->IsUInt()) {

            CefRefPtr<CAfxData> vertexStreamZeroData =
                arguments[4]->IsNull()
                    ? nullptr
                    : static_cast<CAfxData*>(
                arguments[4]->GetArrayBufferReleaseCallback().get());

              self->m_PipeQueue.Queue(
                [self, fn_resolve = arguments[0], fn_reject = arguments[1], primitiveType = arguments[2]->GetUIntValue(), primitiveCount = arguments[3]->GetUIntValue(), vertexStreamZeroData,
                 vertexStreamZeroStride = arguments[5]->GetUIntValue()]() {
                if (!self->m_InFlow)
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::DrawPrimitiveUP))
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(primitiveType))
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(primitiveCount))
                  goto __error;

                if (!self->m_PipeServer.WriteUInt32(vertexStreamZeroStride))
                  goto __error;

                    if (nullptr == vertexStreamZeroData)
                    {
                  if (!self->m_PipeServer.WriteBoolean(false))
                    goto __error;
                }
                    else {
                      if (!self->m_PipeServer.WriteBoolean(true))
                        goto __error;
                      if (!self->m_PipeServer.WriteUInt32(
                              (UINT32)vertexStreamZeroData->GetSize()))
                        goto __error;
                      if (!self->m_PipeServer.WriteBytes(
                              vertexStreamZeroData->GetData(), 0,
                              (DWORD)vertexStreamZeroData->GetSize()))
                        goto __error;
                }

                if (!self->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Context)
                          return;

                        self->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));
                              fn_resolve->ExecuteFunction(NULL, args);
                              self->m_Context->Exit();
                            }));
                return;

              __error:
                self->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              });

              return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });


 CAfxObject::AddFunction(
        obj, 
        "beginCleanState",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (2 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1]]() {
              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::BeginCleanState))
                goto __error;

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
     obj, 
        "endCleanState",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
        CefString& exceptionoverride) {
       auto self =
           CAfxObject::As<AfxObjectType::DrawingInteropImpl, CDrawingInteropImpl>(
               object);
       if (self == nullptr) {
         exceptionoverride = g_szInvalidThis;
         return true;
       }
          if (2 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
            self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                     fn_reject = arguments[1]]() {
              if (!self->m_InFlow)
                goto __error;

              if (!self->m_PipeServer.WriteUInt32(
                      (UINT32)DrawingReply::EndCleanState))
                goto __error;

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
              return;

            __error:
              self->Close();

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                            if (nullptr == self->m_Context)
                              return;

                            self->m_Context->Enter();
                            fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));
            });

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, "createDrawingData",
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

  
    CAfxObject::AddFunction(
        obj, 
        "setSize",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsInt() &&
              arguments[3]->IsInt()) {
            self->m_InteropQueue.Queue([self, fn_resolve = arguments[0],
                                      fn_reject = arguments[1],
                                      width = arguments[2]->GetIntValue(),
                                      height = arguments[3]->GetIntValue()]() {
                  try {
                    self->WriteInt32((int)HostMessage::DrawingResized);
                    self->WriteInt32((int)width);
                    self->WriteInt32((int)height);
                    self->Flush();

                    CefPostTask(
                        TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));
                  } catch (const std::exception& e) {
                    CefPostTask(
                        TID_RENDERER,
                        new CAfxTask([self, fn_reject,
                                      error_msg = std::string(e.what())]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateString(error_msg));
                          fn_reject->ExecuteFunction(NULL, args);
                          self->m_Context->Exit();
                        }));

                  }
                });

            return true;
          }
          exceptionoverride = g_szInvalidArguments;
          return true;
        });


    CAfxObject::AddFunction(
        obj, 
        "sendExternalBeginFrame",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::DrawingInteropImpl,
                                   CDrawingInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
          if (2 <= arguments.size() && arguments[0]->IsFunction() &&
            arguments[1]->IsFunction()) {
            self->m_InteropQueue.Queue(
                [self, fn_resolve = arguments[0], fn_reject = arguments[1]]() {
                  try {
                    self->WriteInt32((int)HostMessage::RenderFrame);
                    self->Flush();

                    CefPostTask(
                        TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));

                  } catch (const std::exception& e) {
                    CefPostTask(
                        TID_RENDERER,
                        new CAfxTask([self, fn_reject,
                                      error_msg = std::string(e.what())]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateString(error_msg));
                          fn_reject->ExecuteFunction(NULL, args);
                          self->m_Context->Exit();
                        }));
                  }
                });

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

    CAfxObject::AddFunction(
        obj, 
        "d3dCompile2",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
           const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          if (10 == arguments.size() && (arguments[0]->IsArrayBuffer() || arguments[0]->IsNull()) && (arguments[1]->IsNull() || arguments[1]->IsString()) &&
              (arguments[2]->IsArrayBuffer() || arguments[2]->IsNull()) &&
              arguments[3]->IsNull() && (arguments[4]->IsNull() || arguments[4]->IsString()) &&
              arguments[5]->IsString() && arguments[6]->IsUInt() &&
              arguments[7]->IsUInt() && arguments[8]->IsUInt() &&
              (arguments[9]->IsArrayBuffer() || arguments[9]->IsNull())) {

            CefRefPtr<CAfxData> srcData =
                arguments[0]->IsNull() ? nullptr : static_cast<CAfxData*>(
                arguments[0]->GetArrayBufferReleaseCallback().get());
            CefRefPtr<CAfxData> defines =
                arguments[2]->IsNull()
                    ? nullptr
                    : static_cast<CAfxData*>(
                arguments[2]->GetArrayBufferReleaseCallback().get());
            auto target = arguments[5]->GetStringValue();
            auto flags1 = arguments[6]->GetUIntValue();
            auto flags2 = arguments[7]->GetUIntValue();
            auto secondaryDataFlags = arguments[8]->GetUIntValue();
            auto secondaryData =
                arguments[9]->IsNull()
                    ? nullptr
                    : static_cast<CAfxData*>(
                          arguments[9]->GetArrayBufferReleaseCallback().get());

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
    CAfxObject::AddFunction(
        obj, 
        "createNullHandle",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          retval = CAfxHandle::Create(NULL);
          return true;
        });
    CAfxObject::AddFunction(
        obj, 
        "createInvalidHandle",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          retval = CAfxHandle::Create(INVALID_HANDLE_VALUE);
          return true;
        });
    CAfxObject::AddFunction(
        obj, 
        "createHandleFromLoHi",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
                      const CefV8ValueList& arguments,
                      CefRefPtr<CefV8Value>& retval,
                      CefString& exceptionoverride) {
          if(2 == arguments.size() && arguments[0]->IsUInt() && arguments[1]->IsUInt())
          {
            retval = CAfxHandle::Create(CAfxHandle::ToHandle(arguments[0]->GetUIntValue(), arguments[1]->GetUIntValue()));
            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });
    //

    if (out)
      *out = self;
    
    return obj;
  }

  void Cancel() { CancelSynchronousIo(m_PipeQueue.GetNativeThreadHandle()); }
  
    void ClosePipes() {
    Close();

    try {
      this->Flush();
    } catch (const std::exception&) {
    }
    try {
      this->ClosePipe();
    } catch (const std::exception&) {
    }
  }

  virtual void CloseInterop() override {
    Cancel();
    ClosePipes();

    m_WaitConnectionQuit = true;
    if (m_WaitConnectionThread.joinable())
      m_WaitConnectionThread.join();

    m_PipeQueue.Abort();
    m_InteropQueue.Abort();

    m_Context = nullptr;

  }

 private:
  int m_BrowserId;

  struct Waiter_s {
    CefRefPtr<CefV8Value> fn_resolve;
    CefRefPtr<CefV8Value> fn_reject;

    Waiter_s(CefRefPtr<CefV8Value> fn_resolve, CefRefPtr<CefV8Value> fn_reject) : fn_resolve(fn_resolve), fn_reject(fn_reject) {

    }
  };

  std::queue<Waiter_s> m_Waiters;

 protected:
  CDrawingInteropImpl(int browserId)
      : CAfxObject(AfxObjectType::DrawingInteropImpl)
      , CAfxInterop("advancedfxInterop_drawing")
      , m_BrowserId(browserId) {
    m_WaitConnectionThread =
        std::thread(&CDrawingInteropImpl::WaitConnectionThreadHandler, this);
  }

  virtual CPipeServerConnectionThread* OnNewConnection(
      HANDLE handle) override {
    return new CDrawingInteropImplConnectionThread(handle, this);
  }  

  virtual void OnClose() override {
    m_InFlow = false;
  }

 private:

 void OnClientMessage_Message(int senderId, const std::string & message)
 {
    if (nullptr == m_Context)
      return;

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
   if (nullptr == m_Context)
     return;

   m_Context->Enter();

    CefV8ValueList execArgs;
    execArgs.push_back(CAfxHandle::Create(sharedTextureHandle));

    if (m_OnAcceleratedPaint->IsValid()) {
      m_OnAcceleratedPaint->ExecuteCallback(execArgs);
    }

    m_Context->Exit();
 }

  void OnClientMessage_ReleaseShareHandle(HANDLE sharedTextureHandle) {
   if (nullptr == m_Context)
     return;

   m_Context->Enter();

    CefV8ValueList execArgs;
    execArgs.push_back(CAfxHandle::Create(sharedTextureHandle));

    if (m_OnReleaseShareHandle->IsValid()) {
      m_OnReleaseShareHandle->ExecuteCallback(execArgs);
    }

    m_Context->Exit();
 } 


  void OnClientMessage_WaitedForGpu(bool bOk) {
   if (nullptr == m_Context)
     return;

    m_Context->Enter();

    if(bOk && 0 < m_Waiters.size())
    {
      m_Waiters.front().fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
    }
    else
    {
      while(0 < m_Waiters.size())
      {
        m_Waiters.front().fn_reject->ExecuteFunction(NULL, CefV8ValueList());
        m_Waiters.pop();
      }
    }

    m_Context->Exit();
 } 

 class CDrawingInteropImplConnectionThread
     : public advancedfx::interop::CPipeServerConnectionThread {
  public:
   CDrawingInteropImplConnectionThread(HANDLE handle, CDrawingInteropImpl* host)
       : advancedfx::interop::CPipeServerConnectionThread(handle),
         m_Host(host) {}

   ~CDrawingInteropImplConnectionThread() {
     m_Quit = true;
     Cancel();
     Join();
   }

   protected:
   virtual void ConnectionThread() override {
     try {
       while (!m_Quit) {
         ClientMessage message = (ClientMessage)ReadInt32();
         switch (message) {
           case ClientMessage::Message: {
             int senderId = ReadInt32();
             std::string argMessage;
             ReadStringUTF8(argMessage);

             CefPostTask(
                 TID_RENDERER,
                 base::Bind(&CDrawingInteropImpl::OnClientMessage_Message,
                            m_Host, senderId, argMessage));
           } break;
           case ClientMessage::TextureHandle: {
             HANDLE shared_handle = ReadHandle();
             CefPostTask(
                 TID_RENDERER,
                 base::Bind(
                     &CDrawingInteropImpl::OnClientMessage_CefFrameRendered,
                     m_Host, shared_handle));
           } break;
           case ClientMessage::WaitedForGpu: {
             bool bOk = ReadBoolean();
             CefPostTask(
                 TID_RENDERER,
                 base::Bind(&CDrawingInteropImpl::OnClientMessage_WaitedForGpu,
                            m_Host, bOk));
           } break;
           case ClientMessage::ReleaseTextureHandle: {
             HANDLE shared_handle = (HANDLE)ReadUInt64();
             CefPostTask(
                 TID_RENDERER,
                 base::Bind(
                     &CDrawingInteropImpl::OnClientMessage_ReleaseShareHandle,
                     m_Host, shared_handle));
           } break;
           default:
             throw "CHostPipeServerConnectionThread::ConnectionThread: Unknown message.";
         }
       }
     }
     catch (const std::exception& e) {
       DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                   << e.what();
     }
   }

  private:
   CDrawingInteropImpl* m_Host;
   bool m_Quit= false;
 };

private:

  class CAfxD3d9VertexDeclaration : public CAfxObject {

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> interop,
        CefRefPtr<CAfxD3d9VertexDeclaration>* out = nullptr) {

      auto obj = CAfxObject::Create(
          new CAfxD3d9VertexDeclaration(interop));
      CAfxObject::AddFunction(obj, "release", [](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exception) {
            auto self =
                CAfxObject::As<AfxObjectType::AfxD3d9VertexDeclaration, CAfxD3d9VertexDeclaration>(object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }

        if (2 <= arguments.size() && arguments[0]->IsFunction() &&
            arguments[1]->IsFunction()) {

          self->m_Interop->m_PipeQueue.Queue([self,
                                                 fn_resolve = arguments[0],
                                                 fn_reject = arguments[1]]() {
            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;

            if (!self->m_Interop->m_InFlow)
              goto __error;

            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9VertexDeclaration))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt64((UINT64)self->GetIndex()))
              goto __error;

            if (!self->m_Interop->m_PipeServer.Flush())
              goto __error;

            int hr;
            if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
              goto __error;

            if (FAILED(hr)) {
              unsigned int lastError;
              if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                goto __error;
              CefPostTask(TID_RENDERER,
                          new CAfxTask([self, fn_resolve, hr, lastError]() {
                            if (nullptr == self->m_Interop->m_Context)
                              return;

                            self->m_Interop->m_Context->Enter();

                            CefRefPtr<CefV8Value> result =
                                CefV8Value::CreateObject(nullptr, nullptr);
                            result->SetValue("hr", CefV8Value::CreateInt(hr),
                                             V8_PROPERTY_ATTRIBUTE_NONE);
                            result->SetValue("lastError",
                                             CefV8Value::CreateUInt(lastError),
                                             V8_PROPERTY_ATTRIBUTE_NONE);

                            CefV8ValueList args;
                            args.push_back(result);
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Interop->m_Context->Exit();
                          }));
              return;
            }

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_resolve, hr]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();

                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateInt(hr));

                          fn_resolve->ExecuteFunction(NULL, args);

                          self->m_Interop->m_Context->Exit();
                        }));
            return;

          __error:
            self->m_Interop->Close();

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_reject]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Interop->m_Context->Exit();
                        }));
          });
        }
        exception = g_szInvalidArguments;
        return true;
      });

      if (out)
        *out = CAfxObject::As<AfxObjectType::AfxD3d9VertexDeclaration, CAfxD3d9VertexDeclaration>(obj);

      return obj;
    }

    CAfxD3d9VertexDeclaration(CefRefPtr<CDrawingInteropImpl> interop)
        : CAfxObject(AfxObjectType::AfxD3d9VertexDeclaration), m_Interop(interop) {
    }

  private:
    bool m_DoReleased = false;
   CefRefPtr<CDrawingInteropImpl> m_Interop;

     IMPLEMENT_REFCOUNTING(CAfxD3d9VertexDeclaration);

  };

  class CAfxD3d9IndexBuffer : public CAfxObject {

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> interop,
        CefRefPtr<CAfxD3d9IndexBuffer>* out = nullptr) {

      auto obj = CAfxObject::Create(
          new CAfxD3d9IndexBuffer(interop));

      CAfxObject::AddFunction(
          obj, "release",
          [](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exception) {

          auto self =
                CAfxObject::As<AfxObjectType::AfxD3d9IndexBuffer, CAfxD3d9IndexBuffer>(object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }

        if (2 <= arguments.size() && arguments[0]->IsFunction() &&
            arguments[1]->IsFunction()) {
         
          self->m_Interop->m_PipeQueue.Queue([self,
                                                 fn_resolve = arguments[0],
                                                 fn_reject = arguments[1]]() {

            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;

            if (!self->m_Interop->m_InFlow)
              goto __error;

            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9IndexBuffer))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt64((UINT64)self->GetIndex()))
              goto __error;

            if (!self->m_Interop->m_PipeServer.Flush())
              goto __error;

            int hr;
            if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
              goto __error;

            if (FAILED(hr)) {
              unsigned int lastError;
              if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                goto __error;
              CefPostTask(TID_RENDERER,
                          new CAfxTask([self, fn_resolve, hr, lastError]() {
                            if (nullptr == self->m_Interop->m_Context)
                              return;

                            self->m_Interop->m_Context->Enter();

                            CefRefPtr<CefV8Value> result =
                                CefV8Value::CreateObject(nullptr, nullptr);
                            result->SetValue("hr", CefV8Value::CreateInt(hr),
                                             V8_PROPERTY_ATTRIBUTE_NONE);
                            result->SetValue("lastError",
                                             CefV8Value::CreateUInt(lastError),
                                             V8_PROPERTY_ATTRIBUTE_NONE);

                            CefV8ValueList args;
                            args.push_back(result);
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Interop->m_Context->Exit();
                          }));
              return;
            }

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_resolve, hr]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();

                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateInt(hr));

                          fn_resolve->ExecuteFunction(NULL, args);

                          self->m_Interop->m_Context->Exit();
                        }));
            return;

          __error:
            self->m_Interop->Close();

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_reject]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Interop->m_Context->Exit();
                        }));
          });
        }
        exception = g_szInvalidArguments;
        return true;
      });

      CAfxObject::AddFunction(
          obj, "update",
          [](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exception) {
            auto self = CAfxObject::As<AfxObjectType::AfxD3d9IndexBuffer,
                                     CAfxD3d9IndexBuffer>(object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }
      
          if (5 <= arguments.size()
          && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() &&
            arguments[3]->IsUInt() && arguments[4]->IsUInt()) {
          auto data = CAfxObject::As<AfxObjectType::AfxData, CAfxData>(arguments[2]);

          if (nullptr != data) {

              self->m_Interop->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       data,
                                       offsetToLock = (int)arguments[3]->GetUIntValue(),
                                       sizeToLock = (int)arguments[4]->GetUIntValue()](){

            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;
              
            if (!self->m_Interop->m_InFlow)
              goto __error;

            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::UpdateD3d9IndexBuffer))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt64(
                    (UINT64)self->GetIndex()))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)offsetToLock))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)sizeToLock))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteBytes(
                    (unsigned char*)data->GetData() + offsetToLock,
                    (DWORD)offsetToLock, (DWORD)sizeToLock))
              goto __error;

     if (!self->m_Interop->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Interop->m_Context)
                          return;

                        self->m_Interop->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Interop->m_Context->Exit();
                      }));
                  return;
                }

                  CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                                self->m_Interop->m_Context->Enter();
                                CefV8ValueList args;
                                args.push_back(CefV8Value::CreateInt(hr));
                                fn_resolve->ExecuteFunction(
                                    NULL, args);
                                self->m_Interop->m_Context->Exit();
                              }));
                return;

              __error:
                self->m_Interop->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                              self->m_Interop->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Interop->m_Context->Exit();
                            }));
              });
                                       return true;
        }
            }

          exception = g_szInvalidArguments;
          return true;
      });


      if (out)
        *out = CAfxObject::As<AfxObjectType::AfxD3d9IndexBuffer,
                            CAfxD3d9IndexBuffer>(obj);

      return obj;
    }

    CAfxD3d9IndexBuffer(CefRefPtr<CDrawingInteropImpl> interop)
        : CAfxObject(AfxObjectType::AfxD3d9IndexBuffer), m_Interop(interop) {
    }
      
   private:
    bool m_DoReleased = false;
    CefRefPtr<CDrawingInteropImpl> m_Interop;
    IMPLEMENT_REFCOUNTING(CAfxD3d9IndexBuffer);
  };

  class CAfxD3d9VertexBuffer : public CAfxObject {

   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> interop,
        CefRefPtr<CAfxD3d9VertexBuffer>* out = nullptr) {
      
      auto obj =
          CAfxObject::Create(new CAfxD3d9VertexBuffer(interop));
      CAfxObject::AddFunction(
          obj, "release",
          [](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
             CefString& exception) {
            auto self = CAfxObject::As<AfxObjectType::AfxD3d9VertexBuffer,
                                     CAfxD3d9VertexBuffer>(object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }

        if (2 <= arguments.size() && arguments[0]->IsFunction() &&
            arguments[1]->IsFunction()) {

          self->m_Interop->m_PipeQueue.Queue([self,
                                                 fn_resolve = arguments[0],
                                                 fn_reject = arguments[1]]() {

            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;
              
                          if (!self->m_Interop->m_InFlow)
              goto __error;

            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9VertexBuffer))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt64((UINT64)self->GetIndex()))
              goto __error;

            if (!self->m_Interop->m_PipeServer.Flush())
              goto __error;

            int hr;
            if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
              goto __error;

            if (FAILED(hr)) {
              unsigned int lastError;
              if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                goto __error;
              CefPostTask(TID_RENDERER,
                          new CAfxTask([self, fn_resolve, hr, lastError]() {
                            if (nullptr == self->m_Interop->m_Context)
                              return;

                            self->m_Interop->m_Context->Enter();

                            CefRefPtr<CefV8Value> result =
                                CefV8Value::CreateObject(nullptr, nullptr);
                            result->SetValue("hr", CefV8Value::CreateInt(hr),
                                             V8_PROPERTY_ATTRIBUTE_NONE);
                            result->SetValue("lastError",
                                             CefV8Value::CreateUInt(lastError),
                                             V8_PROPERTY_ATTRIBUTE_NONE);

                            CefV8ValueList args;
                            args.push_back(result);
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Interop->m_Context->Exit();
                          }));
              return;
            }

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_resolve, hr]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();

                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateInt(hr));

                          fn_resolve->ExecuteFunction(NULL, args);

                          self->m_Interop->m_Context->Exit();
                        }));
            return;

          __error:
            self->m_Interop->Close();

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_reject]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Interop->m_Context->Exit();
                        }));
          });
        }
        exception = g_szInvalidArguments;
        return true;
      });


      CAfxObject::AddFunction(
          obj, "update",
          [](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exception) {
            auto self = CAfxObject::As<AfxObjectType::AfxD3d9VertexBuffer,
                                     CAfxD3d9VertexBuffer>(object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }
      
          if (5 <= arguments.size()
          && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() &&
            arguments[3]->IsUInt() && arguments[4]->IsUInt()) {
          auto data = CAfxObject::As<AfxObjectType::AfxData, CAfxData>(arguments[2]);

          if (nullptr != data) {

              self->m_Interop->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       data,
                                       offsetToLock = (int)arguments[3]->GetUIntValue(),
                                       sizeToLock = (int)arguments[4]->GetUIntValue()](){

            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;
              
            if (!self->m_Interop->m_InFlow)
              goto __error;

            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::UpdateD3d9VertexBuffer))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt64(
                    (UINT64)self->GetIndex()))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)offsetToLock))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)sizeToLock))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteBytes(
                    (unsigned char*)data->GetData() + offsetToLock,
                    (DWORD)offsetToLock, (DWORD)sizeToLock))
              goto __error;

     if (!self->m_Interop->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Interop->m_Context)
                          return;

                        self->m_Interop->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Interop->m_Context->Exit();
                      }));
                  return;
                }

                  CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                                self->m_Interop->m_Context->Enter();
                                CefV8ValueList args;
                                args.push_back(CefV8Value::CreateInt(hr));
                                fn_resolve->ExecuteFunction(
                                    NULL, args);
                                self->m_Interop->m_Context->Exit();
                              }));
                return;

              __error:
                self->m_Interop->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                              self->m_Interop->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Interop->m_Context->Exit();
                            }));
              });
                                       return true;
        }
            }

          exception = g_szInvalidArguments;
          return true;
      });

      if (out)
        *out = CAfxObject::As<AfxObjectType::AfxD3d9VertexBuffer,
                            CAfxD3d9VertexBuffer>(obj);

      return obj;
    }

    CAfxD3d9VertexBuffer(CefRefPtr<CDrawingInteropImpl> interop)
        : CAfxObject(AfxObjectType::AfxD3d9VertexBuffer), m_Interop(interop) {
    }


   private:
    bool m_DoReleased = false;
    CefRefPtr<CDrawingInteropImpl> m_Interop;

        IMPLEMENT_REFCOUNTING(CAfxD3d9VertexBuffer);
  };

  class CAfxD3d9Texture : public CAfxObject {


   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> interop,
        CefRefPtr<CAfxD3d9Texture>* out = nullptr) {

      auto obj =
          CAfxObject::Create(new CAfxD3d9Texture(interop));

      CAfxObject::AddFunction(
          obj, "release",
          [](
                         const CefString& name, CefRefPtr<CefV8Value> object,
                         const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
             CefString& exception) {
            auto self = CAfxObject::As<AfxObjectType::AfxD3d9Texture, CAfxD3d9Texture>(
                    object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }

            if (2 <= arguments.size() && arguments[0]->IsFunction() &&
                arguments[1]->IsFunction()) {

              self->m_Interop->m_PipeQueue.Queue([self,
                                                     fn_resolve = arguments[0],
                                                     fn_reject = arguments[1]]() {

            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;
              
                          if (!self->m_Interop->m_InFlow)
                  goto __error;

                if (!self->m_Interop->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::ReleaseD3d9Texture))
                  goto __error;
                if (!self->m_Interop->m_PipeServer.WriteUInt64(
                        (UINT64)self->GetIndex()))
                  goto __error;

                if (!self->m_Interop->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                 if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                     TID_RENDERER,
                     new CAfxTask([self, fn_resolve, hr, lastError]() {
                               if (nullptr == self->m_Interop->m_Context)
                                 return;

                               self->m_Interop->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                               self->m_Interop->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                              self->m_Interop->m_Context->Enter();

                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));

                              fn_resolve->ExecuteFunction(NULL, args);

                              self->m_Interop->m_Context->Exit();
                            }));
                return;

              __error:
            self->m_Interop->Close();

                CefPostTask(
                    TID_RENDERER,
                    new CAfxTask([self, fn_reject]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Interop->m_Context->Exit();
                        }));
              });
            }
            exception = g_szInvalidArguments;
            return true;
          });

      CAfxObject::AddFunction(
        obj,
          "update", [](
                        const CefString& name, CefRefPtr<CefV8Value> object,
                        const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
             CefString& exception) {
            auto self =
                CAfxObject::As<AfxObjectType::AfxD3d9Texture, CAfxD3d9Texture>(
                    object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }

            if (10 <= arguments.size()
            && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()
              && arguments[2]->IsUInt() &&
            arguments[3]->IsObject() &&
            arguments[5]->IsUInt() && arguments[6]->IsUInt() &&
            arguments[7]->IsUInt() && arguments[8]->IsUInt() &&
            arguments[9]->IsUInt()) {
              auto data = CAfxObject::As<AfxObjectType::AfxData, CAfxData>(
                  arguments[2]);

          auto rectLeft = arguments[3]->GetValue("left");
          auto rectTop = arguments[3]->GetValue("top");
          auto rectRight = arguments[3]->GetValue("right");
          auto rectBottom = arguments[3]->GetValue("bottom");

          UINT32 rowOffsetBytes = arguments[5]->GetUIntValue();
          UINT32 columnOffsetBytes = arguments[6]->GetUIntValue();
          UINT32 dataBytesPerRow = arguments[7]->GetUIntValue();
          UINT32 totalBytesPerRow = arguments[8]->GetUIntValue();
          UINT32 numRows = arguments[9]->GetUIntValue();

          if (nullptr != rectLeft && nullptr != rectTop &&
              nullptr != rectRight && nullptr != rectBottom &&
              rectLeft->IsInt() && rectTop->IsInt() && rectRight->IsInt() &&
              rectBottom->IsInt()) {

              self->m_Interop->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       data,
                                       level = arguments[2]->GetUIntValue(),
                left = rectLeft->GetIntValue(),
                top = rectTop->GetIntValue(),
                right = rectRight->GetIntValue(),
                bottom = rectBottom->GetIntValue(),
                numRows,
                dataBytesPerRow, columnOffsetBytes,
                rowOffsetBytes,
                totalBytesPerRow]() {


            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;
              
            if (!self->m_Interop->m_InFlow)
              goto __error;

          if (!self->m_Interop->m_PipeServer.WriteUInt32(
                  (UINT32)DrawingReply::UpdateD3d9Texture))
            goto __error;
          if (!self->m_Interop->m_PipeServer.WriteUInt64(
                  (UINT64)self->GetIndex()))
            goto __error;
          if (!self->m_Interop->m_PipeServer.WriteUInt32(level))
            goto __error;

        if (!self->m_Interop->m_PipeServer.WriteBoolean(true))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(left))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(top))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(right))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt32(bottom))
              goto __error;

          if (!self->m_Interop->m_PipeServer.WriteUInt32((UINT32)numRows))
            goto __error;
          if (!self->m_Interop->m_PipeServer.WriteUInt32(
                  (UINT32)(dataBytesPerRow - columnOffsetBytes)))
            goto __error;

          void* pData = (unsigned char*)data->GetData() + rowOffsetBytes;

          for (unsigned int i = 0; i < numRows; ++i) {
            if (!self->m_Interop->m_PipeServer.WriteBytes(
                    (unsigned char*)pData + columnOffsetBytes, 0,
                    dataBytesPerRow - columnOffsetBytes))
              goto __error;

            pData = (unsigned char*)pData + totalBytesPerRow;
          }
     if (!self->m_Interop->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Interop->m_Context)
                          return;

                        self->m_Interop->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Interop->m_Context->Exit();
                      }));
                  return;
                }

                  CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                                self->m_Interop->m_Context->Enter();
                                CefV8ValueList args;
                                args.push_back(CefV8Value::CreateInt(hr));
                                fn_resolve->ExecuteFunction(
                                    NULL, args);
                                self->m_Interop->m_Context->Exit();
                              }));
                return;

              __error:
                self->m_Interop->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                              self->m_Interop->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Interop->m_Context->Exit();
                            }));
              });
                                       return true;

          } else {
           self->m_Interop->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       data,
                                       level = arguments[2]->GetUIntValue(),
                numRows,
                dataBytesPerRow, columnOffsetBytes,
                rowOffsetBytes,
                totalBytesPerRow]() {


            if (self->m_DoReleased)
                    goto __error;
                  else
                    self->m_DoReleased = true;

                  if (!self->m_Interop->m_InFlow)
                    goto __error;

          if (!self->m_Interop->m_PipeServer.WriteUInt32(
                  (UINT32)DrawingReply::UpdateD3d9Texture))
            goto __error;
          if (!self->m_Interop->m_PipeServer.WriteUInt64(
                  (UINT64)self->GetIndex()))
            goto __error;
          if (!self->m_Interop->m_PipeServer.WriteUInt32(level))
            goto __error;

        if (!self->m_Interop->m_PipeServer.WriteBoolean(false))
              goto __error;

          if (!self->m_Interop->m_PipeServer.WriteUInt32((UINT32)numRows))
            goto __error;
          if (!self->m_Interop->m_PipeServer.WriteUInt32(
                  (UINT32)(dataBytesPerRow - columnOffsetBytes)))
            goto __error;

          void* pData = (unsigned char*)data->GetData() + rowOffsetBytes;

          for (unsigned int i = 0; i < numRows; ++i) {
            if (!self->m_Interop->m_PipeServer.WriteBytes(
                    (unsigned char*)pData + columnOffsetBytes, 0,
                    dataBytesPerRow - columnOffsetBytes))
              goto __error;

            pData = (unsigned char*)pData + totalBytesPerRow;
          }
     if (!self->m_Interop->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
                  goto __error;

               if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER, new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Interop->m_Context)
                          return;

                        self->m_Interop->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Interop->m_Context->Exit();
                      }));
                  return;
                }

                  CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                                self->m_Interop->m_Context->Enter();
                                CefV8ValueList args;
                                args.push_back(CefV8Value::CreateInt(hr));
                                fn_resolve->ExecuteFunction(
                                    NULL, args);
                                self->m_Interop->m_Context->Exit();
                              }));
                return;

              __error:
                self->m_Interop->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                              self->m_Interop->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Interop->m_Context->Exit();
                            }));
              });
                                       return true;
          }

          return true;
        }

        exception = g_szInvalidArguments;
        return true;
      });
   

      if (out)
        *out =
            CAfxObject::As<AfxObjectType::AfxD3d9Texture,
                            CAfxD3d9Texture>(obj);
      return obj;
    }


    CAfxD3d9Texture(CefRefPtr<CDrawingInteropImpl> interop)
        : CAfxObject(AfxObjectType::AfxD3d9Texture), m_Interop(interop) {
    }

   private:
    bool m_DoReleased = false;
    CefRefPtr<CDrawingInteropImpl> m_Interop;

    IMPLEMENT_REFCOUNTING(CAfxD3d9Texture);
  };

  class CAfxD3d9PixelShader : public CAfxObject {
   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> interop,
        CefRefPtr<CAfxD3d9PixelShader>* out = nullptr) {

      auto obj = CAfxObject::Create(new CAfxD3d9PixelShader(interop));
      CAfxObject::AddFunction(
          obj, "release",
          [](
                                            const CefString& name,
                                            CefRefPtr<CefV8Value> object,
                                            const CefV8ValueList& arguments,
                                            CefRefPtr<CefV8Value>& retval,
                                            CefString& exception) {
            auto self =
                CAfxObject::As<AfxObjectType::AfxD3d9PixelShader, CAfxD3d9PixelShader>(
                    object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }
        if (2 <= arguments.size() && arguments[0]->IsFunction() &&
            arguments[1]->IsFunction()) {


          self->m_Interop->m_PipeQueue.Queue([self,
                                                 fn_resolve = arguments[0],
                                                 fn_reject = arguments[1]]() {
            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;
              
            if (!self->m_Interop->m_InFlow)
              goto __error;

            if (!self->m_Interop->m_PipeServer.WriteUInt32(
                    (UINT32)DrawingReply::ReleaseD3d9PixelShader))
              goto __error;
            if (!self->m_Interop->m_PipeServer.WriteUInt64((UINT64)self->GetIndex()))
              goto __error;

            if (!self->m_Interop->m_PipeServer.Flush())
              goto __error;

            int hr;
            if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
              goto __error;

            if (FAILED(hr)) {
              unsigned int lastError;
              if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                goto __error;
              CefPostTask(TID_RENDERER,
                          new CAfxTask([self, fn_resolve, hr, lastError]() {
                            if (nullptr == self->m_Interop->m_Context)
                              return;

                            self->m_Interop->m_Context->Enter();

                            CefRefPtr<CefV8Value> result =
                                CefV8Value::CreateObject(nullptr, nullptr);
                            result->SetValue("hr", CefV8Value::CreateInt(hr),
                                             V8_PROPERTY_ATTRIBUTE_NONE);
                            result->SetValue("lastError",
                                             CefV8Value::CreateUInt(lastError),
                                             V8_PROPERTY_ATTRIBUTE_NONE);

                            CefV8ValueList args;
                            args.push_back(result);
                            fn_resolve->ExecuteFunction(NULL, args);
                            self->m_Interop->m_Context->Exit();
                          }));
              return;
            }

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_resolve, hr]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();

                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateInt(hr));

                          fn_resolve->ExecuteFunction(NULL, args);

                          self->m_Interop->m_Context->Exit();
                        }));
            return;

          __error:
            self->m_Interop->Close();

            CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_reject]() {
                          if (nullptr == self->m_Interop->m_Context)
                            return;

                          self->m_Interop->m_Context->Enter();
                          fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Interop->m_Context->Exit();
                        }));
          });
        }
        exception = g_szInvalidArguments;
        return true;
      });
 
      if (out)
        *out = CAfxObject::As<AfxObjectType::AfxD3d9PixelShader,
                            CAfxD3d9PixelShader>(
            obj);

      return obj;
    }

    CAfxD3d9PixelShader(CefRefPtr<CDrawingInteropImpl> interop)
        : CAfxObject(AfxObjectType::AfxD3d9PixelShader), m_Interop(interop) {
    }


   private:
    bool m_DoReleased = false;
    CefRefPtr<CDrawingInteropImpl> m_Interop;

    IMPLEMENT_REFCOUNTING(CAfxD3d9PixelShader);
  };

 class CAfxD3d9VertexShader : public CAfxObject {
   public:
    static CefRefPtr<CefV8Value> Create(
        CefRefPtr<CDrawingInteropImpl> interop,
        CefRefPtr<CAfxD3d9VertexShader>* out = nullptr) {
      auto obj = CAfxObject::Create(new CAfxD3d9VertexShader(interop));
      CAfxObject::AddFunction(
          obj, "release",
          [](const CefString& name, CefRefPtr<CefV8Value> object,
             const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
             CefString& exception) {
            auto self = CAfxObject::As<AfxObjectType::AfxD3d9VertexShader,
                                     CAfxD3d9VertexShader>(object);
            if (self == nullptr) {
              exception = g_szInvalidThis;
              return true;
            }
            if (2 <= arguments.size() && arguments[0]->IsFunction() &&
                arguments[1]->IsFunction()) {

              self->m_Interop->m_PipeQueue.Queue([self,
                                                  fn_resolve = arguments[0],
                                                  fn_reject = arguments[1]]() {

            if (self->m_DoReleased)
              goto __error;
            else
              self->m_DoReleased = true;

                if (!self->m_Interop->m_InFlow)
                  goto __error;

                if (!self->m_Interop->m_PipeServer.WriteUInt32(
                        (UINT32)DrawingReply::ReleaseD3d9VertexShader))
                  goto __error;
                if (!self->m_Interop->m_PipeServer.WriteUInt64(
                        (UINT64)self->GetIndex()))
                  goto __error;

                if (!self->m_Interop->m_PipeServer.Flush())
                  goto __error;

                int hr;
                if (!self->m_Interop->m_PipeServer.ReadInt32(hr))
                  goto __error;

                if (FAILED(hr)) {
                  unsigned int lastError;
                  if (!self->m_Interop->m_PipeServer.ReadUInt32(lastError))
                    goto __error;
                  CefPostTask(
                      TID_RENDERER,
                      new CAfxTask([self, fn_resolve, hr, lastError]() {
                        if (nullptr == self->m_Interop->m_Context)
                          return;

                        self->m_Interop->m_Context->Enter();

                        CefRefPtr<CefV8Value> result =
                            CefV8Value::CreateObject(nullptr, nullptr);
                        result->SetValue("hr", CefV8Value::CreateInt(hr),
                                         V8_PROPERTY_ATTRIBUTE_NONE);
                        result->SetValue("lastError",
                                         CefV8Value::CreateUInt(lastError),
                                         V8_PROPERTY_ATTRIBUTE_NONE);

                        CefV8ValueList args;
                        args.push_back(result);
                        fn_resolve->ExecuteFunction(NULL, args);
                        self->m_Interop->m_Context->Exit();
                      }));
                  return;
                }

                CefPostTask(TID_RENDERER,
                            new CAfxTask([self, fn_resolve, hr]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                              self->m_Interop->m_Context->Enter();

                              CefV8ValueList args;
                              args.push_back(CefV8Value::CreateInt(hr));

                              fn_resolve->ExecuteFunction(NULL, args);

                              self->m_Interop->m_Context->Exit();
                            }));
                return;

              __error:
                self->m_Interop->Close();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                              if (nullptr == self->m_Interop->m_Context)
                                return;

                              self->m_Interop->m_Context->Enter();
                              fn_reject->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                              self->m_Interop->m_Context->Exit();
                            }));
              });
            }
            exception = g_szInvalidArguments;
            return true;
          });

      if (out)
        *out = CAfxObject::As<AfxObjectType::AfxD3d9VertexShader,
                            CAfxD3d9VertexShader>(obj);

      return obj;
    }

    CAfxD3d9VertexShader(CefRefPtr<CDrawingInteropImpl> interop)
        : CAfxObject(AfxObjectType::AfxD3d9VertexShader), m_Interop(interop) {}

   private:
    bool m_DoReleased = false;
    CefRefPtr<CDrawingInteropImpl> m_Interop;

        IMPLEMENT_REFCOUNTING(CAfxD3d9VertexShader);
  };

  bool m_InFlow = false;

  HANDLE m_ShareHandle = INVALID_HANDLE_VALUE;
  int m_Width = 640;
  int m_Height = 480;

  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;
  CefRefPtr<CAfxCallback> m_OnDeviceLost;
  CefRefPtr<CAfxCallback> m_OnDeviceReset;
  CefRefPtr<CAfxCallback> m_OnAcceleratedPaint;
  CefRefPtr<CAfxCallback> m_OnReleaseShareHandle;

   bool m_WaitConnectionQuit = false;
  std::thread m_WaitConnectionThread;

  void WaitConnectionThreadHandler(void) {
    std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_client_");
    strPipeName.append(std::to_string(GetCurrentProcessId()));
    strPipeName.append("_");
    strPipeName.append(std::to_string(m_BrowserId));

   while (!m_WaitConnectionQuit) {
      try {
       auto thread = this->WaitForConnection(strPipeName.c_str(), 500);
        if (thread != nullptr) {
            thread->Join();
            delete thread;
          break;
        }
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
      }
    }
  }

  int DoPumpBegin(int frameCount, unsigned int pass) {
       m_InFlow = false;

    while (true) {
      UINT32 drawingMessage;
      if (!m_PipeServer.ReadUInt32(drawingMessage))
        return __LINE__;

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
                        if (nullptr == m_Context)
                          return;

                        if (m_OnDeviceLost->IsValid())
                          m_OnDeviceLost->ExecuteCallback(CefV8ValueList());
                      }));

        }
          bContinue = true;
          break;
        case DrawingMessage::DeviceRestored: {
          CefPostTask(TID_RENDERER, new CAfxTask([this]() {
                        if (nullptr == m_Context)
                          return;
                        if (m_OnDeviceReset->IsValid())
                          m_OnDeviceReset->ExecuteCallback(CefV8ValueList());
                      }));        
        }
          bContinue = true;
          break;

        default:
          return __LINE__;
      }

      if (bContinue)
        continue;

      INT32 clientFrameCount;
      if(!m_PipeServer.ReadInt32(clientFrameCount))
          return __LINE__;

      UINT32 clientPass;
      if (!m_PipeServer.ReadUInt32(clientPass))
        return __LINE__;

      INT32 frameDiff = frameCount - clientFrameCount;

      if (frameDiff < 0 || frameDiff == 0 && pass < clientPass) {
        // Error: client is ahead, otherwise we would have correct
        // data by now.

        if (!m_PipeServer.WriteUInt32((INT32)DrawingReply::Retry))
          return __LINE__;

        if(!m_PipeServer.Flush())
            return __LINE__;
/*
         std::string code = std::to_string(clientFrameCount) + "/> " +
                           std::to_string(frameCount) + " @ " +
                           std::to_string(clientPass) + " / " +
                           std::to_string(pass);
        MessageBoxA(0, code.c_str(), "DrawingReply::Retry", MB_OK);*/

        return 0;

      } else if (frameDiff > 0) {
        // client is behind.

        if (!m_PipeServer.WriteUInt32((INT32)DrawingReply::Skip))
          return __LINE__;

        if(!m_PipeServer.Flush())
            return __LINE__;
/*
        std::string code = std::to_string(clientFrameCount) + " / " +
                           std::to_string(frameCount) + " @ " +
                           std::to_string(clientPass) + " / " +
                           std::to_string(pass);
        MessageBoxA(0, code.c_str(), "DrawingReply::Skip", MB_OK);*/
      } else {
        m_InFlow = pass == clientPass;

        if(!m_InFlow) 
        {
          if (!m_PipeServer.WriteUInt32((UINT32)DrawingReply::Finished))
            return __LINE__;

          if (!m_PipeServer.Flush())
            return __LINE__;
/*
          std::string code = std::to_string(clientFrameCount) + " == " +
                            std::to_string(frameCount) + " @ " +
                            std::to_string(clientPass) + " / " +
                            std::to_string(pass);
          MessageBoxA(0, code.c_str(), "DrawingReply::Finished", MB_OK);*/            
        }

        return 0;
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
                                           DWORD handlerId,
                                           CefRefPtr<CInterop>* out) {
  return CDrawingInteropImpl::Create(browser, frame, context, argStr, handlerId,
                                     out);
}

class CEngineInteropImpl : public CAfxObject,
                           public CInterop,
                           public CAfxInterop,
                           public CPipeServer,
                           public CPipeClient {
  IMPLEMENT_REFCOUNTING(CEngineInteropImpl);

 public:

  static CefRefPtr<CefV8Value> Create(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context,
                                     const CefString& argStr,
                                     DWORD handlerId,
                                     CefRefPtr<CInterop>* out = nullptr) {

   CefRefPtr<CEngineInteropImpl> self =
       new CEngineInteropImpl(browser->GetIdentifier());

   std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_handler_");
   strPipeName.append(std::to_string(handlerId));

   try {
      while(true)
      {
      bool bError = false;
      try {
        self->OpenPipe(strPipeName.c_str(), INFINITE);
        Sleep(100);
      }
      catch(const std::exception&)
      {
        bError = true;
      }

      if(!bError) break;
      }
     self->WriteUInt32(GetCurrentProcessId());
     self->WriteInt32(browser->GetIdentifier());
     self->Flush();
    } catch (const std::exception& e) {
     DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                 << e.what();
      if (out)
        *out = nullptr;
      return CefV8Value::CreateNull();
    }

    auto obj = CAfxObject::Create(self);

   //
   self->m_Context = context;

    CAfxObject::AddGetter(
       obj, "id",
       [](const CefString& name, const CefRefPtr<CefV8Value> object,
          CefRefPtr<CefV8Value>& retval, CefString& exception) {
         auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                  CEngineInteropImpl>(object);
         if (self == nullptr) {
           exception = g_szInvalidThis;
           return true;
         }

         retval = CefV8Value::CreateInt(self->m_BrowserId);
         return true;
       });

   CAfxObject::AddGetter(
       obj, "args",
       [argStr](const CefString& name, const CefRefPtr<CefV8Value> object,
                CefRefPtr<CefV8Value>& retval, CefString& exception) {
         retval = CefV8Value::CreateString(argStr);
         return true;
       });

   CAfxObject::AddFunction(
       obj, "sendMessage",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
          const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
          CefString& exceptionoverride) {
         auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                  CEngineInteropImpl>(object);
         if (self == nullptr) {
           exceptionoverride = g_szInvalidThis;
           return true;
         }

         if (4 <= arguments.size() && arguments[0]->IsFunction() &&
             arguments[1]->IsFunction() && arguments[2]->IsInt() &&
             arguments[3]->IsString()) {
           self->m_InteropQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1],
                                       id = arguments[2]->GetIntValue(),
                                       str = arguments[3]->GetStringValue()]() {
             try {
               self->WriteInt32((int)HostMessage::Message);
               self->WriteInt32(id);
               self->WriteStringUTF8(str.ToString().c_str());
               self->Flush();

               CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                             if (nullptr == self->m_Context)
                               return;
                             self->m_Context->Enter();
                             fn_resolve->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                             self->m_Context->Exit();
                           }));
             } catch (const std::exception& e) {
               CefPostTask(
                   TID_RENDERER,
                   new CAfxTask(
                       [self, fn_reject, error_msg = std::string(e.what())]() {
                         if (nullptr == self->m_Context)
                           return;

                         self->m_Context->Enter();
                         CefV8ValueList args;
                         args.push_back(CefV8Value::CreateString(error_msg));
                         fn_reject->ExecuteFunction(NULL, args);
                         self->m_Context->Exit();
                       }));
             }
           });

           return true;
         }

         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   self->m_OnMessage = CAfxObject::AddCallback(obj, "onMessage");

   self->m_OnError = CAfxObject::AddCallback(obj, "onError");

   CAfxObject::AddFunction(
       obj, "setPipeName",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
          const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
          CefString& exceptionoverride) {
         auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                  CEngineInteropImpl>(object);
         if (self == nullptr) {
           exceptionoverride = g_szInvalidThis;
           return true;
         }

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

CAfxObject::AddFunction(
       obj, "connect",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
          const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
          CefString& exceptionoverride) {
         auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                  CEngineInteropImpl>(object);
         if (self == nullptr) {
           exceptionoverride = g_szInvalidThis;
           return true;
         }

         if (2 <= arguments.size() && arguments[0]->IsFunction() &&
             arguments[1]->IsFunction()) {
           self->m_PipeQueue.Queue([self, fn_resolve = arguments[0],
                                       fn_reject = arguments[1]]() {
             if (self->Connection()) {
               CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                             if (nullptr == self->m_Context)
                               return;

                             self->m_Context->Enter();
                             fn_resolve->ExecuteFunction(NULL,
                                                         CefV8ValueList());
                             self->m_Context->Exit();
                           }));
             } else {
               CefPostTask(TID_RENDERER, new CAfxTask([self, fn_reject]() {
                             if (nullptr == self->m_Context)
                               return;

                             self->m_Context->Enter();
                             fn_reject->ExecuteFunction(NULL, CefV8ValueList());
                             self->m_Context->Exit();
                           }));
             }
           });
           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

  CAfxObject::AddFunction(
        obj, "cancel",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
         auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                  CEngineInteropImpl>(object);
         if (self == nullptr) {
           exceptionoverride = g_szInvalidThis;
           return true;
         }

          self->Cancel();

          return true;
        });

   CAfxObject::AddFunction(
       obj, "close",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
          const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
          CefString& exceptionoverride) {
         auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                  CEngineInteropImpl>(object);
         if (self == nullptr) {
           exceptionoverride = g_szInvalidThis;
           return true;
         }

         if (2 <= arguments.size() && arguments[0]->IsFunction() &&
             arguments[1]->IsFunction()) {

           self->m_PipeQueue.Queue([self, fn_resolve = arguments[0]]() {
             self->Close();
             CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                           if (nullptr == self->m_Context)
                             return;

                           self->m_Context->Enter();
                           fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                           self->m_Context->Exit();
                         }));
           });
           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

    CAfxObject::AddFunction(
        obj, "pump",
        [](const CefString& name,
                                          CefRefPtr<CefV8Value> object,
                                          const CefV8ValueList& arguments,
                                          CefRefPtr<CefV8Value>& retval,
          CefString& exceptionoverride) {
         auto self =
             CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(
                 object);
         if (self == nullptr) {
           exceptionoverride = g_szInvalidThis;
           return true;
         }

          if (3 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction()) {
        self->m_PipeQueue.Queue(
            [self, fn_resolve = arguments[0], fn_reject = arguments[1], filter = new CAfxValue(arguments[2]),
                                 obj = new CAfxValue(4 <= arguments.size() ? arguments[3] : nullptr)]() {

          int errorLine = __LINE__;

          if (self->GetConnected() &&
              0 == (errorLine = self->DoPump(filter, obj))) {
            if (nullptr == self->m_Context)
              return;

              CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          if (nullptr == self->m_Context)
                            return;

                            self->m_Context->Enter();
                            fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                            self->m_Context->Exit();
                          }));            
          } else {
              CefPostTask(TID_RENDERER,
                        new CAfxTask([self, fn_reject, errorLine]() {
                          if (nullptr == self->m_Context)
                            return;

                            self->m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(CefV8Value::CreateString("AfxInterop.cpp:"+std::to_string(errorLine)));
                            fn_reject->ExecuteFunction(NULL, args);
                            self->m_Context->Exit();
                          }));            
          }
        });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });


   CAfxObject::AddFunction(obj, "addCalcHandle",
             [](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
    auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(object);
    if (self == nullptr) {
      exceptionoverride = g_szInvalidThis;
      return true;
    }

               if (2 == arguments.size() && arguments[0]->IsString() && arguments[1]->IsFunction()) {
           retval = CCalcResult::Create(self->m_PipeQueue,             
                          &self->m_HandleCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });


   CAfxObject::AddFunction(obj,"addCalcVecAng",
             [](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
    auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(object);
    if (self == nullptr) {
      exceptionoverride = g_szInvalidThis;
      return true;
    }
                   if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                    CCalcResult::Create(self->m_PipeQueue,
                          &self->m_VecAngCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   CAfxObject::AddFunction(obj,"addCalcCam",
             [](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
    auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(object);
    if (self == nullptr) {
      exceptionoverride = g_szInvalidThis;
      return true;
    }
                   if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                     CCalcResult::Create(self->m_PipeQueue,
                          &self->m_CamCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }

               exceptionoverride = g_szInvalidArguments;
               return true;
             });
   CAfxObject::AddFunction(obj,"addCalcFov",
             [](
                 const CefString& name, CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                 CefString& exceptionoverride) {
    auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(object);
    if (self == nullptr) {
      exceptionoverride = g_szInvalidThis;
      return true;
    }
                   if (2 == arguments.size() &&
                   arguments[0]->IsString() && arguments[1]->IsFunction()) {
                 retval =
                     CCalcResult::Create(self->m_PipeQueue,
                          &self->m_FovCalcCallbacks,
                          arguments[0]->GetStringValue().ToString().c_str(),
                     new CAfxCallback(arguments[1]));

                 return true;
               }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   CAfxObject::AddFunction(obj,
       "addCalcBool",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exceptionoverride) {
    auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(object);
    if (self == nullptr) {
      exceptionoverride = g_szInvalidThis;
      return true;
    }
             if (2 == arguments.size() &&
             arguments[0]->IsString() && arguments[1]->IsFunction()) {
           std::string valName = arguments[0]->GetStringValue().ToString();

           retval = CCalcResult::Create(self->m_PipeQueue,
                         &self->m_BoolCalcCallbacks,
                         arguments[0]->GetStringValue().ToString().c_str(),
               new CAfxCallback(arguments[1]));

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
   CAfxObject::AddFunction(obj,
       "addCalcInt",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exceptionoverride) {
    auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(object);
    if (self == nullptr) {
      exceptionoverride = g_szInvalidThis;
      return true;
    }
             if (2 == arguments.size() &&
             arguments[0]->IsString() && arguments[1]->IsFunction()) {
           std::string valName = arguments[0]->GetStringValue().ToString();

           retval = CCalcResult::Create(self->m_PipeQueue,
                         &self->m_IntCalcCallbacks,
                         arguments[0]->GetStringValue().ToString().c_str(),
               new CAfxCallback(arguments[1]));

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

    CAfxObject::AddFunction(
        obj, 
       "gameEventAllowAdd",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                   CEngineInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

         if (1 == arguments.size() &&
             arguments[0]->IsString()) {

             self->m_PipeQueue.Queue(
               [self, val = arguments[0]->GetStringValue()]() {
                 std::string strVal = val.ToString();

                 self->m_GameEventsAllowDeletions.erase(strVal);
                 self->m_GameEventsAllowAdditions.insert(strVal);
               });

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
    CAfxObject::AddFunction(
        obj, 
       "gameEventAllowRemove",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                   CEngineInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

         if (1 == arguments.size() &&
             arguments[0]->IsString()) {

             self->m_PipeQueue.Queue(
               [self, val = arguments[0]->GetStringValue()]() {
                 std::string strVal = val.ToString();

        self->m_GameEventsAllowAdditions.erase(strVal);
                 self->m_GameEventsAllowDeletions.insert(strVal);
               });

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
    CAfxObject::AddFunction(
        obj, 
       "gameEventDenyAdd",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                   CEngineInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
         if (1 == arguments.size() &&
             arguments[0]->IsString()) {

             self->m_PipeQueue.Queue(
               [self, val = arguments[0]->GetStringValue()]() {
                 std::string strVal = val.ToString();

           self->m_GameEventsDenyDeletions.erase(strVal);
                 self->m_GameEventsDenyAdditions.insert(strVal);
               });

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
    CAfxObject::AddFunction(
        obj, "gameEventDenyRemove",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                   CEngineInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
         if (1 == arguments.size() &&
             arguments[0]->IsString()) {

           self->m_PipeQueue.Queue(
               [self, val = arguments[0]->GetStringValue()]() {
                 std::string strVal = val.ToString();


           self->m_GameEventsDenyAdditions.erase(strVal);
                 self->m_GameEventsDenyDeletions.insert(strVal);
               });


           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
    CAfxObject::AddFunction(
        obj, 
       "gameEventSetEnrichment",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                   CEngineInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }
         if (3 == arguments.size() && arguments[0]->IsString() &&
             arguments[1]->IsString() && arguments[2]->IsUInt()) {

           self->m_PipeQueue.Queue(
               [self, strEvent = arguments[0]->GetStringValue(),
                strProperty = arguments[1]->GetStringValue(),
                uiEnrichments = arguments[2]->GetUIntValue()]() {

           self->m_GameEventsEnrichmentsChanges[GameEventEnrichmentKey_s(
                 strEvent.ToString().c_str(), strProperty.ToString().c_str())] =
                 uiEnrichments;
               });

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });
    CAfxObject::AddFunction(
        obj, "gameEventSetTransmitClientTime",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                   CEngineInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

         if (1 == arguments.size() && arguments[0]->IsBool()) {
           self->m_PipeQueue.Queue(
               [self, value = arguments[0]->GetBoolValue()] {
                 self->m_GameEventsTransmitChanged =
                     self->m_GameEventsTransmitChanged ||
                     value != self->m_GameEventsTransmitClientTime;
                 self->m_GameEventsTransmitClientTime = value;
               });

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   CAfxObject::AddFunction(
        obj, 
       "gameEventSetTransmitTick",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self = CAfxObject::As<AfxObjectType::EngineInteropImpl,
                                   CEngineInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

         if (1 == arguments.size() && arguments[0]->IsBool()) {
           self->m_PipeQueue.Queue(
               [self, value = arguments[0]->GetBoolValue()] {
                 self->m_GameEventsTransmitChanged =
                     self->m_GameEventsTransmitChanged ||
                     value != self->m_GameEventsTransmitTick;
                 self->m_GameEventsTransmitTick = value;
               });

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   CAfxObject::AddFunction(
       obj, 
       "gameEventSetTransmitSystemTime",
       [](const CefString& name, CefRefPtr<CefV8Value> object,
              const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
          CefString& exceptionoverride) {
         auto self =
             CAfxObject::As<AfxObjectType::EngineInteropImpl, CEngineInteropImpl>(
                 object);
         if (self == nullptr) {
           exceptionoverride = g_szInvalidThis;
           return true;
         }

         if (1 == arguments.size() && arguments[0]->IsBool()) {
           self->m_PipeQueue.Queue(
               [self, value = arguments[0]->GetBoolValue()] {
                 self->m_GameEventsTransmitChanged =
                     self->m_GameEventsTransmitChanged ||
                     value != self->m_GameEventsTransmitSystemTime;
                 self->m_GameEventsTransmitSystemTime = value;
               });

           return true;
         }
         exceptionoverride = g_szInvalidArguments;
         return true;
       });

   //

   if (out)
     *out = self;

   return obj;
}

  void Cancel() { CancelSynchronousIo(m_PipeQueue.GetNativeThreadHandle()); }

    void ClosePipes() {
  Close();
  try {
    this->ClosePipe();
  } catch (const std::exception&) {
  }
}

 virtual void CloseInterop() override {
  Cancel();
  ClosePipes();

  m_WaitConnectionQuit = true;
  if (m_WaitConnectionThread.joinable())
    m_WaitConnectionThread.join();

    m_PipeQueue.Abort();
  m_InteropQueue.Abort();

    m_Context = nullptr;

 }

private:
    int m_BrowserId;
    CVersion m_ServerVersion;
    CVersion m_ClientVersion;

 protected:

     
  CEngineInteropImpl(int browserId)
      : CAfxObject(AfxObjectType::EngineInteropImpl), CAfxInterop("advancedfxInterop"),
        m_BrowserId(browserId),
        m_ServerVersion(7, 0, 0, 0) {
    m_WaitConnectionThread =
        std::thread(&CEngineInteropImpl::WaitConnectionThreadHandler, this);
  }

  
  virtual void OnConnect() OVERRIDE {
    m_PumpResumeAt = 0;
    m_NewConnection = true;
  }

  virtual CPipeServerConnectionThread* OnNewConnection(
      HANDLE handle) override {
    return new CEngineInteropImplConnectionThread(handle, this);
  }

    bool IsPumpFilter(CefRefPtr<CAfxValue> filter,
                                      const char* what) {
    if (filter != nullptr && filter->IsObject()) {
      auto value = filter->GetChild(what);
      if (value && value->IsV8Function()) {
        return true;
      }
    }
    return false;
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

  float m_Tx = 0;
  float m_Ty = 0;
  float m_Tz = 0;
  float m_Rx = 0;
  float m_Ry = 0;
  float m_Rz = 0;
  float m_Fov = 90;

  int DoPump(CefRefPtr<CAfxValue> filter, CefRefPtr<CAfxValue> obj) {
    int errorLine = 0;

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

      if (!m_PipeServer.WriteInt32(m_ServerVersion.GetMajor()))
        AFX_GOTO_ERROR
      
      if (!m_PipeServer.Flush())
        AFX_GOTO_ERROR


      bool versionSupported;
      if (!m_PipeServer.ReadBoolean(versionSupported))
        AFX_GOTO_ERROR

      if (!versionSupported)
        AFX_GOTO_ERROR

      // Supply server info required by client:

#if _WIN64
      if (!m_PipeServer.WriteBoolean(true))
        AFX_GOTO_ERROR
#else
      if (!m_PipeServer.WriteBoolean(false))
        AFX_GOTO_ERROR
#endif

      if (!m_PipeServer.WriteUInt32(m_ServerVersion.GetMinor())) {
        errorLine = __LINE__;
        goto error;
      }
      if (!m_PipeServer.WriteUInt32(m_ServerVersion.GetPatch())) {
        errorLine = __LINE__;
        goto error;
      }
      if (!m_PipeServer.WriteUInt32(m_ServerVersion.GetBuild())) {
        errorLine = __LINE__;
        goto error;
      }

      if (!m_PipeServer.Flush()) {
        errorLine = __LINE__;
        goto error;
      }

      unsigned int clientVersion[4];

      if (!m_PipeServer.ReadUInt32(clientVersion[0])) {
        errorLine = __LINE__;
        goto error;
      }
      if (!m_PipeServer.ReadUInt32(clientVersion[1])) {
        errorLine = __LINE__;
        goto error;
      }
      if (!m_PipeServer.ReadUInt32(clientVersion[2])) {
        errorLine = __LINE__;
        goto error;
      }
      if (!m_PipeServer.ReadUInt32(clientVersion[3])) {
        errorLine = __LINE__;
        goto error;
      }

      bool bClientAccepts;
      if (!m_PipeServer.ReadBoolean(bClientAccepts))
        AFX_GOTO_ERROR

      m_ClientVersion.Set(clientVersion[0], clientVersion[1], clientVersion[2],
                          clientVersion[3]);

      if (m_ClientVersion < CVersion(7,0,0,0)) {
        if (!m_PipeServer.WriteBoolean(false))
          AFX_GOTO_ERROR

        if (!m_PipeServer.Flush())
          AFX_GOTO_ERROR

        AFX_GOTO_ERROR
      }

      if (!m_PipeServer.WriteBoolean(true))
        AFX_GOTO_ERROR

      //

      if (!WriteGameEventSettings(false, filter))
        AFX_GOTO_ERROR

      if (!m_PipeServer.Flush())
        AFX_GOTO_ERROR

      auto onNewConnection = GetPumpFilter(filter, "onNewConnection");
      if (nullptr != onNewConnection) {
        m_PumpResumeAt = 1;
        CefPostTask(TID_RENDERER, new CAfxTask([this, onNewConnection]() {
                      if (nullptr == m_Context)
                        return;
                      m_Context->Enter();
                      onNewConnection->ExecuteFunction(nullptr, CefV8ValueList());
                      m_Context->Exit();
                    }));
        return 0;
      }
    }

    goto __1;
  }
 
  __1: {
      UINT32 engineMessage;
      if (!m_PipeServer.ReadUInt32(engineMessage))
        AFX_GOTO_ERROR

      switch ((EngineMessage)engineMessage) {
        case EngineMessage::BeforeFrameStart: {
          // Read incoming commands from client:
          {
            UINT32 commandIndex = 0;
            UINT32 commandCount;
            if (!m_PipeServer.ReadCompressedUInt32(commandCount))
              AFX_GOTO_ERROR

            CefRefPtr<CAfxValue> objCommands = CAfxValue::CreateArray((int)commandCount);

            while (0 < commandCount) {
              UINT32 argIndex = 0;
              UINT32 argCount;
              if (!m_PipeServer.ReadCompressedUInt32(argCount))
                AFX_GOTO_ERROR

              CefRefPtr<CAfxValue> objArgs =
                  CAfxValue::CreateArray((int)argCount);

              objCommands->SetArrayElement((int)commandIndex, objArgs);

              while (0 < argCount) {
                std::string str;

                if (!m_PipeServer.ReadStringUTF8(str))
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
                            if (nullptr == m_Context)
                              return;

                            m_Context->Enter();
                            CefV8ValueList args;
                            args.push_back(objCommands->ToV8Value());
                            onCommands->ExecuteFunction(nullptr, args);
                            m_Context->Exit();
                          }));
              return 0;
            }
          }
          if (!m_PipeServer.WriteCompressedUInt32((UINT32)(0)))
            AFX_GOTO_ERROR
        
          if (!m_PipeServer.Flush())
              AFX_GOTO_ERROR

          goto __1;     
        } break;

        case EngineMessage::BeforeFrameRenderStart: {
          if (!WriteGameEventSettings(true, filter))
            AFX_GOTO_ERROR
          if (!m_PipeServer.Flush())
            AFX_GOTO_ERROR

          goto __1;
        } break;

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

          if (!m_PipeServer.Flush())
            AFX_GOTO_ERROR

          if (!m_HandleCalcCallbacks.BatchUpdateResult(this, m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_VecAngCalcCallbacks.BatchUpdateResult(this, m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_CamCalcCallbacks.BatchUpdateResult(this, m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_FovCalcCallbacks.BatchUpdateResult(this, m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_BoolCalcCallbacks.BatchUpdateResult(this, m_PipeServer))
            AFX_GOTO_ERROR
          if (!m_IntCalcCallbacks.BatchUpdateResult(this, m_PipeServer))
            AFX_GOTO_ERROR

          goto __1;
        } break;

        case EngineMessage::OnRenderView: {
          struct RenderInfo_s renderInfo;

          if (!m_PipeServer.ReadInt32(renderInfo.FrameCount))
            AFX_GOTO_ERROR

          if (!m_PipeServer.ReadSingle(renderInfo.AbsoluteFrameTime))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.CurTime))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.FrameTime))
            AFX_GOTO_ERROR

          if (!m_PipeServer.ReadInt32(renderInfo.View.X))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadInt32(renderInfo.View.Y))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadInt32(renderInfo.View.Width))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadInt32(renderInfo.View.Height))
            AFX_GOTO_ERROR

          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M00))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M01))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M02))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M03))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M10))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M11))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M12))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M13))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M20))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M21))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M22))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M23))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M30))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M31))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M32))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ViewMatrix.M33))
            AFX_GOTO_ERROR

          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M00))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M01))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M02))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M03))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M10))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M11))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M12))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M13))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M20))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M21))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M22))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M23))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M30))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M31))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M32))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(renderInfo.View.ProjectionMatrix.M33))
            AFX_GOTO_ERROR

          auto onRenderViewBegin = GetPumpFilter(filter, "onRenderViewBegin");
          if (nullptr != onRenderViewBegin) {
            m_PumpResumeAt = 3;
            CefPostTask(TID_RENDERER,
                new CAfxTask([this, onRenderViewBegin,
                              renderInfo = CreateAfxRenderInfo(renderInfo)]() {
                  if (nullptr == m_Context)
                    return;

                          m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(renderInfo->ToV8Value());
                          onRenderViewBegin->ExecuteFunction(nullptr, args);
                          m_Context->Exit();
                        }));
            return 0;
          }

           goto __3;
        } break;

        case EngineMessage::OnRenderViewEnd: {
          auto onRenderViewEnd = GetPumpFilter(filter, "onRenderViewEnd");
          if (nullptr != onRenderViewEnd) {
            m_PumpResumeAt = 4;
            CefPostTask(TID_RENDERER, new CAfxTask([this, onRenderViewEnd]() {
                          if (nullptr == m_Context)
                            return;

                          m_Context->Enter();
                          onRenderViewEnd->ExecuteFunction(nullptr,
                                                           CefV8ValueList());
                          m_Context->Exit();
                        }));
            return 0;
          }
        } goto __4;

        case EngineMessage::BeforeHud: {
          auto onHudBegin = GetPumpFilter(filter, "onRenderViewHudBegin");
          if (nullptr != onHudBegin) {
            m_PumpResumeAt = 1;
            CefPostTask(TID_RENDERER, new CAfxTask([this, onHudBegin]() {
                          if (nullptr == m_Context)
                            return;

                          m_Context->Enter();
                          onHudBegin->ExecuteFunction(nullptr, CefV8ValueList());
                          m_Context->Exit();
                        }));
            return 0;
          }
        } goto __1;

        case EngineMessage::AfterHud: {
          auto onHudEnd = GetPumpFilter(filter, "onRenderViewHudEnd");
          if (nullptr != onHudEnd) {
            m_PumpResumeAt = 1;
            CefPostTask(TID_RENDERER, new CAfxTask([this, onHudEnd]() {
                          if (nullptr == m_Context)
                            return;

                          m_Context->Enter();
                          onHudEnd->ExecuteFunction(nullptr, CefV8ValueList());
                          m_Context->Exit();
                        }));
            return 0;
          }
        } goto __1;

        case EngineMessage::BeforeTranslucentShadow: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewBeforeTranslucentShadow", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return 0;
        }
          goto __1;
        case EngineMessage::AfterTranslucentShadow: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewAfterTranslucentShadow", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return 0;
        }
          goto __1;
        case EngineMessage::BeforeTranslucent: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewBeforeTranslucent", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return 0;
        }
          goto __1;
        case EngineMessage::AfterTranslucent: {
          bool bReturn;
          if (!DoRenderPass(filter, "onRenderViewAfterTranslucent", bReturn))
            AFX_GOTO_ERROR
          if (bReturn)
            return 0;
        }
          goto __1;

        case EngineMessage::OnViewOverride: {
          if (!m_PipeServer.ReadSingle(m_Tx))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(m_Ty))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(m_Tz))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(m_Rx))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(m_Ry))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(m_Rz))
            AFX_GOTO_ERROR
          if (!m_PipeServer.ReadSingle(m_Fov))
            AFX_GOTO_ERROR

          auto onViewOverride = GetPumpFilter(filter, "onViewOverride");
          if (nullptr != onViewOverride) {
            m_PumpResumeAt = 5;
            CefPostTask(TID_RENDERER,
                        new CAfxTask([this, onViewOverride, tx = m_Tx,
                                      ty = m_Ty, tz = m_Tz, rx = m_Rx,
                                      ry = m_Ry, rz = m_Rz, fov = m_Fov]() {
                          if (nullptr == m_Context)
                            return;

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
            return 0;
          }

          if (!m_PipeServer.WriteBoolean(false))
            AFX_GOTO_ERROR

          if (!m_PipeServer.Flush())
              AFX_GOTO_ERROR  // client is waiting

          goto __1;
        }

        case EngineMessage::GameEvent: {
          bool bReturn = false;
          m_PumpResumeAt = 1;
          if (!ReadGameEvent(filter, bReturn))
            AFX_GOTO_ERROR
          if (bReturn) {
            return 0;
          }
        } goto __1;
      }
    }

    AFX_GOTO_ERROR

__2: {
      int len = nullptr != obj && obj->IsArray() ? obj->GetArraySize() : 0;

    if (!m_PipeServer.WriteCompressedUInt32((UINT32)(len)))
      AFX_GOTO_ERROR

    for(int i = 0; i < len; ++i) {
      auto elem = obj->GetArrayElement(i);
      if (nullptr != elem && elem->IsString()) {
        if (!m_PipeServer.WriteStringUTF8(elem->GetString()))
          AFX_GOTO_ERROR
      }
    }

    if (!m_PipeServer.Flush())
      AFX_GOTO_ERROR

    goto __1;
}

__3 : {
  bool outBeforeTranslucentShadow =
      IsPumpFilter(filter, "onRenderViewBeforeTranslucentShadow");
  bool outAfterTranslucentShadow =
      IsPumpFilter(filter, "onRenderViewAfterTranslucentShadow");
  bool outBeforeTranslucent =
      IsPumpFilter(filter, "onRenderViewBeforeTranslucent");
  bool outAfterTranslucent =
      IsPumpFilter(filter, "onRenderViewAfterTranslucent");
  bool outBeforeHud =
      IsPumpFilter(filter, "onRenderViewHudBegin");
  bool outAfterHud = IsPumpFilter(filter, "onRenderViewHudEnd");
  bool outAfterRenderView =
      IsPumpFilter(filter, "onRenderViewEnd");

  if (!m_PipeServer.WriteBoolean(outBeforeTranslucentShadow))
    AFX_GOTO_ERROR
  if (!m_PipeServer.WriteBoolean(outAfterTranslucentShadow))
    AFX_GOTO_ERROR
  if (!m_PipeServer.WriteBoolean(outBeforeTranslucent))
    AFX_GOTO_ERROR
  if (!m_PipeServer.WriteBoolean(outAfterTranslucent))
    AFX_GOTO_ERROR
  if (!m_PipeServer.WriteBoolean(outBeforeHud))
    AFX_GOTO_ERROR
  if (!m_PipeServer.WriteBoolean(outAfterHud))
    AFX_GOTO_ERROR
  if (!m_PipeServer.WriteBoolean(outAfterRenderView))
    AFX_GOTO_ERROR

  /*bool done = !(true || outBeforeTranslucentShadow || outAfterTranslucentShadow ||
                outBeforeTranslucent || outAfterTranslucent || outBeforeHud ||
                outAfterHud || outAfterRenderView);

  if (done)
    goto __4;
  */
  goto __1;
 
}

__4 : {
  m_PumpResumeAt = 1;
  auto onDone = GetPumpFilter(filter, "onDone");
  if (nullptr != onDone) {
    CefPostTask(TID_RENDERER, new CAfxTask([this, onDone]() {
                  if (nullptr == m_Context)
                    return;

                  m_Context->Enter();
                  onDone->ExecuteFunction(nullptr, CefV8ValueList());
                  m_Context->Exit();
                }));
    return 0;
  }
  goto __1;
}

__5 : {
      bool overriden = false;

      if (nullptr != obj && obj->IsObject()) {
        CefRefPtr<CAfxValue> v8Tx = obj->GetChild("tX");
        if (nullptr != v8Tx && v8Tx->IsDouble()) {
          m_Tx = (float)v8Tx->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Ty = obj->GetChild("tY");
        if (nullptr != v8Ty && v8Ty->IsDouble()) {
          m_Ty = (float)v8Ty->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Tz = obj->GetChild("tZ");
        if (nullptr != v8Tz && v8Tz->IsDouble()) {
          m_Tz = (float)v8Tz->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Rx = obj->GetChild("rX");
        if (nullptr != v8Rx && v8Rx->IsDouble()) {
          m_Rx = (float)v8Rx->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Ry = obj->GetChild("rY");
        if (nullptr != v8Ry && v8Ry->IsDouble()) {
          m_Ry = (float)v8Ry->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Rz = obj->GetChild("rZ");
        if (nullptr != v8Rz && v8Rz->IsDouble()) {
          m_Rz = (float)v8Rz->GetDouble();
          overriden = true;
        }

        CefRefPtr<CAfxValue> v8Fov = obj->GetChild("fov");
        if (nullptr != v8Fov && v8Fov->IsDouble()) {
          m_Fov = (float)v8Fov->GetDouble();
          overriden = true;
        }
      }

      if (overriden) {
        if (!m_PipeServer.WriteBoolean(true))
          AFX_GOTO_ERROR

        if (!m_PipeServer.WriteSingle(m_Tx))
          AFX_GOTO_ERROR
        if (!m_PipeServer.WriteSingle(m_Ty))
          AFX_GOTO_ERROR
        if (!m_PipeServer.WriteSingle(m_Tz))
          AFX_GOTO_ERROR
        if (!m_PipeServer.WriteSingle(m_Rx))
          AFX_GOTO_ERROR
        if (!m_PipeServer.WriteSingle(m_Ry))
          AFX_GOTO_ERROR
        if (!m_PipeServer.WriteSingle(m_Rz))
          AFX_GOTO_ERROR
        if (!m_PipeServer.WriteSingle(m_Fov))
          AFX_GOTO_ERROR
      } else {
        if (!m_PipeServer.WriteBoolean(false))
          AFX_GOTO_ERROR
      }

    if (!m_PipeServer.Flush())
        AFX_GOTO_ERROR  // client is waiting

  }   goto __1;

  error:
    m_PumpResumeAt = 0;
    return errorLine;
  }

 private:

 void OnClientMessage_Message(int senderId, const std::string & message) {
    if (nullptr == m_Context)
      return;

  m_Context->Enter();

  CefV8ValueList execArgs;
  execArgs.push_back(CefV8Value::CreateInt(senderId));
  execArgs.push_back(CefV8Value::CreateString(message));

  if (m_OnMessage->IsValid())
    m_OnMessage->ExecuteCallback(execArgs);

  m_Context->Exit();
 }


 class CEngineInteropImplConnectionThread
     : public advancedfx::interop::CPipeServerConnectionThread {
  public:
   CEngineInteropImplConnectionThread(HANDLE handle, CEngineInteropImpl* host)
       : advancedfx::interop::CPipeServerConnectionThread(handle),
         m_Host(host) {}

   ~CEngineInteropImplConnectionThread() {
     m_Quit = true;
     Cancel();
     Join();
   }

  protected:
   virtual void ConnectionThread() override {
     try {
       while (!m_Quit) {
         ClientMessage message = (ClientMessage)ReadInt32();
         switch (message) {
           case ClientMessage::Message: {
             int senderId = ReadInt32();
             std::string argMessage;
             ReadStringUTF8(argMessage);
             CefPostTask(
                 TID_RENDERER,
                 base::Bind(&CEngineInteropImpl::OnClientMessage_Message,
                            m_Host, senderId, argMessage));
           } break;
           default:
             throw "CEngineInteropImplConnectionThread::ConnectionThread: Unknown message.";
         }
       }
     } catch (const std::exception& e) {
       DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                   << e.what();
     }
   }

  private:
   CEngineInteropImpl* m_Host;
   bool m_Quit = false;
 };

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

  CHandleCalcCallbacks m_HandleCalcCallbacks;
  CVecAngCalcCallbacks m_VecAngCalcCallbacks;
  CCamCalcCallbacks m_CamCalcCallbacks;
  CFovCalcCallbacks m_FovCalcCallbacks;
  CBoolCalcCallbacks m_BoolCalcCallbacks;
  CIntCalcCallbacks m_IntCalcCallbacks;


  int m_Width = 640;
  int m_Height = 480;

  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;

  bool m_WaitConnectionQuit = false;
  std::thread m_WaitConnectionThread;

  void WaitConnectionThreadHandler(void) {
    std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_client_");
    strPipeName.append(std::to_string(GetCurrentProcessId()));
    strPipeName.append("_");
    strPipeName.append(std::to_string(m_BrowserId));

    while (!m_WaitConnectionQuit) {
      try {
        auto thread = this->WaitForConnection(strPipeName.c_str(), 500);
        if (thread != nullptr) {
          thread->Join();
          delete thread;
          break;
        }
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
      }
    }
  }

  bool DoRenderPass(CefRefPtr<CAfxValue> filter, const char* what, bool & bReturn) {

    bReturn = false;

    View_s view;

    if (!m_PipeServer.ReadInt32(view.X))
      return false;
    if (!m_PipeServer.ReadInt32(view.Y))
      return false;
    if (!m_PipeServer.ReadInt32(view.Width))
      return false;
    if (!m_PipeServer.ReadInt32(view.Height))
      return false;

    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M00))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M01))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M02))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M03))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M10))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M11))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M12))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M13))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M20))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M21))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M22))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M23))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M30))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M31))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M32))
      return false;
    if (!m_PipeServer.ReadSingle(view.ViewMatrix.M33))
      return false;

    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M00))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M01))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M02))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M03))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M10))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M11))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M12))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M13))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M20))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M21))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M22))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M23))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M30))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M31))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M32))
      return false;
    if (!m_PipeServer.ReadSingle(view.ProjectionMatrix.M33))
      return false;

    auto onRenderPass = GetPumpFilter(filter, what);
    if (nullptr != onRenderPass) {
      m_PumpResumeAt = 1;
      bReturn = true;
      CefPostTask(TID_RENDERER, new CAfxTask([this, onRenderPass,
                                              argView = CreateAfxView(view)]() {
                    if (nullptr == m_Context)
                      return;

                    m_Context->Enter();
                    CefV8ValueList args;
                    args.push_back(argView->ToV8Value());
                    onRenderPass->ExecuteFunction(nullptr, args);
                    m_Context->Exit();
                  }));
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

  bool ReadGameEvent(CefRefPtr<CAfxValue> filter, bool & bReturn) {
    
    bReturn = false;

    int iEventId;
    if (!m_PipeServer.ReadInt32(iEventId))
      return false;

    std::map<int, KnownGameEvent_s>::iterator itKnown;
    if (0 == iEventId) {
      if (!m_PipeServer.ReadInt32(iEventId))
        return false;

      auto resultEmplace = m_KnownGameEvents.emplace(std::piecewise_construct,
                                                     std::make_tuple(iEventId),
                                                     std::make_tuple());

      if (!resultEmplace.second)
        return false;

      itKnown = resultEmplace.first;

      std::string strName;
      if (!m_PipeServer.ReadStringUTF8(itKnown->second.Name))
        return false;

      while (true) {
        bool bHasNext;
        if (!m_PipeServer.ReadBoolean(bHasNext))
          return false;
        if (!bHasNext)
          break;

        std::string strKey;
        if (!m_PipeServer.ReadStringUTF8(strKey))
          return false;

        int iEventType;
        if (!m_PipeServer.ReadInt32(iEventType))
          return false;

        itKnown->second.Keys.emplace_back(strKey,
                                          (GameEventFieldType)iEventType);
      }
    } else {
      itKnown = m_KnownGameEvents.find(iEventId);
    }

    if (itKnown == m_KnownGameEvents.end())
      return false;

    CefRefPtr<CAfxValue> objEvent = CAfxValue::CreateObject();

    objEvent->SetChild("name", CAfxValue::CreateString(itKnown->second.Name));

    if (m_GameEventsTransmitClientTime) {
      float clientTime;
      if (!m_PipeServer.ReadSingle(clientTime))
        return false;
      objEvent->SetChild("clientTime", CAfxValue::CreateDouble(clientTime));
    }

    if (m_GameEventsTransmitTick) {
      int tick;
      if (!m_PipeServer.ReadInt32(tick))
        return false;
      objEvent->SetChild("tick", CAfxValue::CreateInt(tick));
    }

    if (m_GameEventsTransmitSystemTime) {
      uint64_t systemTime;
      if (!m_PipeServer.ReadUInt64(systemTime))
        return false;
      objEvent->SetChild("systemTime",
                         CAfxValue::CreateTime((time_t)systemTime));
    }

    std::string tmpString;
    float tmpFloat;
    int tmpLong;
    short tmpShort;
    unsigned char tmpByte;
    bool tmpBool;
    unsigned __int64 tmpUint64;

   CefRefPtr<CAfxValue> objKeys = CAfxValue::CreateObject();

    for (auto itKey = itKnown->second.Keys.begin();
         itKey != itKnown->second.Keys.end(); ++itKey) {
     CefRefPtr<CAfxValue> objKey = CAfxValue::CreateObject();

      objKey->SetChild("type", CAfxValue::CreateInt((int)itKey->Type));

      switch (itKey->Type) {
        case GameEventFieldType::CString:
          if (!m_PipeServer.ReadStringUTF8(tmpString))
            return false;
          objKey->SetChild("value", CAfxValue::CreateString(tmpString));
          break;
        case GameEventFieldType::Float:
          if (!m_PipeServer.ReadSingle(tmpFloat))
            return false;
          objKey->SetChild("value", CAfxValue::CreateDouble(tmpFloat));
          break;
        case GameEventFieldType::Long:
          if (!m_PipeServer.ReadInt32(tmpLong))
            return false;
          objKey->SetChild("value", CAfxValue::CreateInt(tmpLong));
          break;
        case GameEventFieldType::Short:
          if (!m_PipeServer.ReadInt16(tmpShort))
            return false;
          objKey->SetChild("value", CAfxValue::CreateInt(tmpShort));
          break;
        case GameEventFieldType::Byte:
          if (!m_PipeServer.ReadByte(tmpByte))
            return false;
          objKey->SetChild("value", CAfxValue::CreateUInt(tmpByte));
          break;
        case GameEventFieldType::Bool:
          if (!m_PipeServer.ReadBoolean(tmpBool))
            return false;
          objKey->SetChild("value", CAfxValue::CreateBool(tmpBool));
          break;
        case GameEventFieldType::Uint64:
          if (!m_PipeServer.ReadUInt64(tmpUint64))
            return false;
          CefRefPtr<CAfxValue> objValue = CAfxValue::CreateArray(2);
          objValue->SetArrayElement(0, CAfxValue::CreateUInt(
                                    (unsigned int)(tmpUint64 & 0x0ffffffff)));
          objValue->SetArrayElement(
              1,
              CAfxValue::CreateUInt(
                                    (unsigned int)(tmpUint64 & 0x0ffffffff)));
          objKey->SetChild("value", objValue);
          break;
      }

      auto itEnrichment = m_GameEventsEnrichments.find(
          GameEventEnrichmentKey_s(itKnown->second.Name, itKey->Key));
      if (itEnrichment != m_GameEventsEnrichments.end()) {
        int enrichmentType = itEnrichment->second;

        CefRefPtr<CAfxValue> objEnrichments =
            CAfxValue::CreateObject();

        if (enrichmentType & (1 << 0)) {
          uint64_t value;
          if (!m_PipeServer.ReadUInt64(value))
            return false;

          CefRefPtr<CAfxValue> objValue = CAfxValue::CreateArray(2);
          objValue->SetArrayElement(
              0, CAfxValue::CreateUInt((unsigned int)(value & 0x0ffffffff)));
          objValue->SetArrayElement(1, CAfxValue::CreateUInt(
                                 (unsigned int)((value >> 32) & 0x0ffffffff)));

          objEnrichments->SetChild("userIdWithSteamId", objValue);
        }

        if (enrichmentType & (1 << 1)) {
          Vector_s value;
          if (!m_PipeServer.ReadSingle(value.X))
            return false;
          if (!m_PipeServer.ReadSingle(value.Y))
            return false;
          if (!m_PipeServer.ReadSingle(value.Z))
            return false;

          CefRefPtr<CAfxValue> objValue =
              CAfxValue::CreateObject();
          objValue->SetChild("x", CAfxValue::CreateDouble(value.X));
          objValue->SetChild("y", CAfxValue::CreateDouble(value.Y));
          objValue->SetChild("z", CAfxValue::CreateDouble(value.Z));
          objEnrichments->SetChild("entnumWithOrigin", objValue);
        }

        if (enrichmentType & (1 << 2)) {
          QAngle_s value;
          if (!m_PipeServer.ReadSingle(value.Pitch))
            return false;
          if (!m_PipeServer.ReadSingle(value.Yaw))
            return false;
          if (!m_PipeServer.ReadSingle(value.Roll))
            return false;
          CefRefPtr<CAfxValue> objValue =
              CAfxValue::CreateObject();
          objValue->SetChild("pitch", CAfxValue::CreateDouble(value.Pitch));
          objValue->SetChild("yaw", CAfxValue::CreateDouble(value.Yaw));
          objValue->SetChild("roll", CAfxValue::CreateDouble(value.Roll));

          objEnrichments->SetChild("entnumWithAngles", objValue);
        }

        if (enrichmentType & (1 << 3)) {
          Vector_s value;
          if (!m_PipeServer.ReadSingle(value.X))
            return false;
          if (!m_PipeServer.ReadSingle(value.Y))
            return false;
          if (!m_PipeServer.ReadSingle(value.Z))
            return false;

          CefRefPtr<CAfxValue> objValue =
              CAfxValue::CreateObject();
          objValue->SetChild("x", CAfxValue::CreateDouble(value.X));
          objValue->SetChild("y", CAfxValue::CreateDouble(value.Y));
          objValue->SetChild("z", CAfxValue::CreateDouble(value.Z));
          objEnrichments->SetChild("useridWithEyePosition", objValue);
        }

        if (enrichmentType & (1 << 4)) {
          QAngle_s value;
          if (!m_PipeServer.ReadSingle(value.Pitch))
            return false;
          if (!m_PipeServer.ReadSingle(value.Yaw))
            return false;
          if (!m_PipeServer.ReadSingle(value.Roll))
            return false;
          CefRefPtr<CAfxValue> objValue =
              CAfxValue::CreateObject();
          objValue->SetChild("pitch", CAfxValue::CreateDouble(value.Pitch));
          objValue->SetChild("yaw", CAfxValue::CreateDouble(value.Yaw));
          objValue->SetChild("roll", CAfxValue::CreateDouble(value.Roll));

          objEnrichments->SetChild("useridWithEyeAngels", objValue);
        }

        objKey->SetChild("enrichments", objEnrichments);
      }

      objKeys->SetChild(itKey->Key.c_str(), objKey);
    }

    objEvent->SetChild("keys", objKeys);

    auto onGameEvent = GetPumpFilter(filter, "onGameEvent");
    if (nullptr != onGameEvent) {
      bReturn = true;
      CefPostTask(TID_RENDERER, new CAfxTask([this, onGameEvent, objEvent]() {
                    if (nullptr == m_Context)
                      return;

                m_Context->Enter();
                    CefV8ValueList args;
                args.push_back(objEvent->ToV8Value());
                    onGameEvent->ExecuteFunction(nullptr, args);
                m_Context->Exit();
                       }));
    }

    return true;
  }

  bool WriteGameEventSettings(bool delta, CefRefPtr<CAfxValue> filter) {

    auto onGameEvent = GetPumpFilter(filter, "onGameEvent");

    if (!m_PipeServer.WriteBoolean(nullptr != onGameEvent ? true
                                                                   : false))
      return false;

    if (nullptr == onGameEvent)
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

      if (!m_PipeServer.WriteBoolean(bChanged))
        return false;

      if (!bChanged)
        return true;
    }

    if (!m_PipeServer.WriteBoolean(m_GameEventsTransmitClientTime))
      return false;
    if (!m_PipeServer.WriteBoolean(m_GameEventsTransmitTick))
      return false;
    if (!m_PipeServer.WriteBoolean(m_GameEventsTransmitSystemTime))
      return false;

    m_GameEventsTransmitChanged = false;

    // Allow removals:
    if (delta) {
      if (!m_PipeServer.WriteCompressedUInt32(
              (UINT32)m_GameEventsAllowDeletions.size()))
        return false;
    }
    while (!m_GameEventsAllowDeletions.empty()) {
      auto it = m_GameEventsAllowDeletions.begin();
      if (!(!delta || m_PipeServer.WriteStringUTF8(*it)))
        return false;
      m_GameEventsAllow.erase(*it);
      m_GameEventsAllowDeletions.erase(it);
    }

    // Allow additions:
    if (!m_PipeServer.WriteCompressedUInt32(
            (UINT32)m_GameEventsAllowAdditions.size() +
            (UINT32)(delta ? 0 : m_GameEventsAllow.size())))
      return false;
    if (!delta) {
      for (auto it = m_GameEventsAllow.begin(); it != m_GameEventsAllow.end(); ++it) {
        if (!m_PipeServer.WriteStringUTF8(*it))
          return false;
      }
    }
    while (!m_GameEventsAllowAdditions.empty()) {
      auto it = m_GameEventsAllowAdditions.begin();
      if (!m_PipeServer.WriteStringUTF8(*it))
        return false;
      m_GameEventsAllow.insert(*it);
      m_GameEventsAllowAdditions.erase(it);
    }

   // Deny removals:
    if (delta) {
      if (!m_PipeServer.WriteCompressedUInt32(
              (UINT32)m_GameEventsDenyDeletions.size()))
        return false;
    }
    while (!m_GameEventsDenyDeletions.empty()) {
      auto it = m_GameEventsDenyDeletions.begin();
      if (!(!delta || m_PipeServer.WriteStringUTF8(*it)))
        return false;
      m_GameEventsDeny.erase(*it);
      m_GameEventsDenyDeletions.erase(it);
    }

    // Deny additions:
    if (!m_PipeServer.WriteCompressedUInt32(
            (UINT32)(m_GameEventsDenyAdditions.size() +
            (UINT32)(delta ? 0 : m_GameEventsDeny.size()))))
      return false;
    if (!delta) {
      for (auto it = m_GameEventsDeny.begin();
           it != m_GameEventsDeny.end(); ++it) {
        if (!m_PipeServer.WriteStringUTF8(*it))
          return false;
      }
    }
    while (!m_GameEventsDenyAdditions.empty()) {
      auto it = m_GameEventsDenyAdditions.begin();
      if (!m_PipeServer.WriteStringUTF8(*it))
        return false;
      m_GameEventsDeny.insert(*it);
      m_GameEventsDenyAdditions.erase(it);
    }

    // Write enrichments:
    if (!m_PipeServer.WriteCompressedUInt32(
            (UINT32)(m_GameEventsEnrichmentsChanges.size() +
                     (delta ? 0 : m_GameEventsEnrichments.size()))))
      return false;
    if (!delta) {
      for (auto itEnrichment = m_GameEventsEnrichments.begin();
           itEnrichment != m_GameEventsEnrichments.end(); ++itEnrichment) {
        if (!m_PipeServer.WriteStringUTF8(itEnrichment->first.EventName.c_str()))
          return false;
        if (!m_PipeServer.WriteStringUTF8(itEnrichment->first.EventPropertyName.c_str()))
          return false;
        if (!m_PipeServer.WriteUInt32(itEnrichment->second))
          return false;
      }
    }
    while (!m_GameEventsEnrichmentsChanges.empty()) {
      auto itEnrichment = m_GameEventsEnrichmentsChanges.begin();

      unsigned int enrichmentType = itEnrichment->second;

      if (!m_PipeServer.WriteStringUTF8(itEnrichment->first.EventName.c_str()))
        return false;
      if (!m_PipeServer.WriteStringUTF8(
              itEnrichment->first.EventPropertyName.c_str()))
        return false;
      if (!m_PipeServer.WriteUInt32(itEnrichment->second))
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
       static CefRefPtr<CefV8Value> Create(CThreadedQueue & pipeQueue, CCalcCallbacksGuts* guts,
           const char* name,
           CefRefPtr<CAfxCallback> callback) {

         CefRefPtr<CCalcResult> self = new CCalcResult(pipeQueue, guts, name, callback);

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
    CThreadedQueue & m_PipeQueue;

   CCalcResult(CThreadedQueue & pipeQueue, CCalcCallbacksGuts* guts,
                const char* name,
                CefRefPtr<CAfxCallback> callback)
        : m_Guts(guts), m_Name(name), m_Callback(callback), m_PipeQueue(pipeQueue) {
      m_Fn_Release = CefV8Value::CreateFunction("release", this);

      m_PipeQueue.Queue([this]{
        m_Guts->Add(m_Name.c_str(), m_Callback);
      });
    }

    virtual ~CCalcResult() { ReleaseCalc(); }

    CefRefPtr<CefV8Value> m_Fn_Release;
    bool m_ReleasedCalc = false;

    bool ReleaseCalc() {
      if (m_ReleasedCalc)
        return false;

      m_PipeQueue.Queue([this]{
        m_ReleasedCalc = true;
        m_Guts->Remove(m_Name.c_str(), m_Callback.get());
        m_Callback = nullptr;
      });

        return true;
    }
  };
};


CefRefPtr<CefV8Value> CreateEngineInterop(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefV8Context> context,
                                          const CefString& argStr,
                                          DWORD handlerId,
                                          CefRefPtr<CInterop>* out) {
  return CEngineInteropImpl::Create(browser, frame, context, argStr, handlerId,
                                    out);
}

class CInteropImpl : public CAfxObject,
    public CInterop,
                     public CPipeServer,
                     public CPipeClient{
  IMPLEMENT_REFCOUNTING(CInteropImpl);
 public:
  static CefRefPtr<CefV8Value> Create(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context,
                                      const CefString& argStr,
                                      DWORD handlerId,
                                      CefRefPtr<CInterop>* out = nullptr) {
    CefRefPtr<CInteropImpl> self = new CInteropImpl(browser->GetIdentifier());

    std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_handler_");
    strPipeName.append(std::to_string(handlerId));

    try {
      while(true)
      {
      bool bError = false;
      try {
        self->OpenPipe(strPipeName.c_str(), INFINITE);
        Sleep(100);
      }
      catch(const std::exception&)
      {
        bError = true;
      }

      if(!bError) break;
      }
      self->WriteUInt32(GetCurrentProcessId());
      self->WriteInt32(browser->GetIdentifier());
      self->Flush();
    } catch (const std::exception& e) {
      DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                  << e.what();

      if (out)
        *out = nullptr;
      return CefV8Value::CreateNull();
    }

    auto obj = CAfxObject::Create(self);

    self->m_Context = context;

    CAfxObject::AddGetter(
        obj, "id",
        [](const CefString& name, const CefRefPtr<CefV8Value> object,
           CefRefPtr<CefV8Value>& retval, CefString& exception) {
          auto self =
              CAfxObject::As<AfxObjectType::InteropImpl,
                                   CInteropImpl>(object);
          if (self == nullptr) {
            exception = g_szInvalidThis;
            return true;
          }

          retval = CefV8Value::CreateInt(self->m_BrowserId);
          return true;
        });

    CAfxObject::AddGetter(
        obj, "args",
        [argStr](const CefString& name, const CefRefPtr<CefV8Value> object,
                 CefRefPtr<CefV8Value>& retval, CefString& exception) {
          retval = CefV8Value::CreateString(argStr);
          return true;
        });

    CAfxObject::AddFunction(
        obj, "sendMessage",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
           const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self =
              CAfxObject::As<AfxObjectType::InteropImpl, CInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsInt() &&
              arguments[3]->IsString()) {
            self->m_InteropQueue.Queue(
                [self, fn_resolve = arguments[0], fn_reject = arguments[1],
                 id = arguments[2]->GetIntValue(),
                 str = arguments[3]->GetStringValue()]() {
                  try {
                    self->WriteInt32((int)HostMessage::Message);
                    self->WriteInt32(id);
                    self->WriteStringUTF8(str.ToString().c_str());
                    self->Flush();

                    CefPostTask(
                        TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));
                  } catch (const std::exception& e) {
                    CefPostTask(
                        TID_RENDERER,
                        new CAfxTask([self, fn_reject,
                                      error_msg = std::string(e.what())]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateString(error_msg));
                          fn_reject->ExecuteFunction(NULL, args);
                          self->m_Context->Exit();
                        }));
                  }
                });

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });

CAfxObject::AddFunction(
       obj, 
        "createDrawingInterop",
        [](const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
           CefString& exceptionoverride) {
          auto self =
              CAfxObject::As<AfxObjectType::InteropImpl, CInteropImpl>(object);
          if (self == nullptr) {
            exceptionoverride = g_szInvalidThis;
            return true;
          }

          if (4 <= arguments.size() && arguments[0]->IsFunction() &&
              arguments[1]->IsFunction() && arguments[2]->IsString() &&
              arguments[3]->IsString()) {
            self->m_InteropQueue.Queue([self, fn_resolve = arguments[0],
                              fn_reject = arguments[1],
                              argUrl = arguments[2]->GetStringValue(),
                              argStr = arguments[3]->GetStringValue()]() {
                  try {
                    self->WriteInt32((int)HostMessage::CreateDrawing);
                    self->WriteStringUTF8(argUrl);
                    self->WriteStringUTF8(argStr);
                    self->Flush();

                    CefPostTask(
                        TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          fn_resolve->ExecuteFunction(NULL, CefV8ValueList());
                          self->m_Context->Exit();
                        }));
                  } catch (const std::exception& e) {
                    CefPostTask(
                        TID_RENDERER,
                        new CAfxTask([self, fn_reject,
                                      error_msg = std::string(e.what())]() {
                          if (nullptr == self->m_Context)
                            return;

                          self->m_Context->Enter();
                          CefV8ValueList args;
                          args.push_back(CefV8Value::CreateString(error_msg));
                          fn_reject->ExecuteFunction(NULL, args);
                          self->m_Context->Exit();
                        }));
                  }
                });

            return true;
          }

          exceptionoverride = g_szInvalidArguments;
          return true;
        });


CAfxObject::AddFunction(
    obj, 
    "createEngineInterop",
    [](const CefString& name, CefRefPtr<CefV8Value> object,
           const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
       CefString& exceptionoverride) {
      auto self =
          CAfxObject::As<AfxObjectType::InteropImpl, CInteropImpl>(object);
      if (self == nullptr) {
        exceptionoverride = g_szInvalidThis;
        return true;
      }

      if (4 <= arguments.size() && arguments[0]->IsFunction() &&
          arguments[1]->IsFunction() && arguments[2]->IsString() &&
          arguments[3]->IsString()) {
        self->m_InteropQueue.Queue([self, fn_resolve = arguments[0],
                                  fn_reject = arguments[1],
                                  argUrl = arguments[2]->GetStringValue(),
                                  argStr = arguments[3]->GetStringValue()]() {
              try {
                self->WriteInt32((int)HostMessage::CreateEngine);
                self->WriteStringUTF8(argUrl);
                self->WriteStringUTF8(argStr);
                self->Flush();

                CefPostTask(TID_RENDERER, new CAfxTask([self, fn_resolve]() {
                              if (nullptr == self->m_Context)
                                return;

                              self->m_Context->Enter();
                              fn_resolve->ExecuteFunction(NULL,
                                                          CefV8ValueList());
                              self->m_Context->Exit();
                            }));
              } catch (const std::exception& e) {
                CefPostTask(
                    TID_RENDERER,
                    new CAfxTask([self, fn_reject, error_msg = std::string(e.what())]() {
                          if (nullptr == self->m_Context)
                            return;

                      self->m_Context->Enter();
                      CefV8ValueList args;
                      args.push_back(CefV8Value::CreateString(error_msg));
                      fn_reject->ExecuteFunction(NULL, args);
                      self->m_Context->Exit();
                    }));
              }
            });

        return true;
      }

      exceptionoverride = g_szInvalidArguments;
      return true;
    });

    self->m_OnMessage = CAfxObject::AddCallback(obj, "onMessage");

    self->m_OnError = CAfxObject::AddCallback(obj, "onError");

    //

    if (out)
      *out = self;

    return obj;
  }

  
    void ClosePipes() {

    try {
      this->ClosePipe();
    } catch (const std::exception&) {
    }
  }

 virtual void CloseInterop() override {
    ClosePipes();

    m_WaitConnectionQuit = true;
    if (m_WaitConnectionThread.joinable())
      m_WaitConnectionThread.join();

    m_InteropQueue.Abort();

    m_Context = nullptr;

  }

 private:
  int m_BrowserId;
  CefRefPtr<CefV8Context> m_Context;

public:
   CInteropImpl(int browserId) : CAfxObject(AfxObjectType::InteropImpl), m_BrowserId(browserId) {
     m_WaitConnectionThread =
         std::thread(&CInteropImpl::WaitConnectionThreadHandler, this);
   }

   virtual CPipeServerConnectionThread* OnNewConnection(
       HANDLE handle) override {
     return new CInteropImplConnectionThread(handle, this);
   }

 private:
   int m_Width = 640;
   int m_Height = 480;

  CefRefPtr<CAfxCallback> m_OnMessage;
  CefRefPtr<CAfxCallback> m_OnError;

bool m_WaitConnectionQuit = false;
  std::thread m_WaitConnectionThread;

  void WaitConnectionThreadHandler(void) {
    std::string strPipeName("\\\\.\\pipe\\afx-cefhud-interop_client_");
    strPipeName.append(std::to_string(GetCurrentProcessId()));
    strPipeName.append("_");
    strPipeName.append(std::to_string(m_BrowserId));

   while (!m_WaitConnectionQuit) {
      try {
       auto thread = this->WaitForConnection(strPipeName.c_str(), 500);
        if (thread != nullptr) {
          thread->Join();
          delete thread;
          break;
        }
      } catch (const std::exception& e) {
        DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                    << e.what();
      }
    }
  }


  void OnClientMessage_Message(int senderId, const std::string & message) {
    if (nullptr == m_Context)
      return;

  m_Context->Enter();

  CefV8ValueList execArgs;
  execArgs.push_back(CefV8Value::CreateInt(senderId));
  execArgs.push_back(CefV8Value::CreateString(message));

  if (m_OnMessage->IsValid())
    m_OnMessage->ExecuteCallback(execArgs);

  m_Context->Exit();
 }

 class CInteropImplConnectionThread
     : public advancedfx::interop::CPipeServerConnectionThread {
  public:
   CInteropImplConnectionThread(HANDLE handle, CInteropImpl* host)
       : advancedfx::interop::CPipeServerConnectionThread(handle),
         m_Host(host) {}

   ~CInteropImplConnectionThread() {
     m_Quit = true;
     Cancel();
     Join();
   }

  protected:
   virtual void ConnectionThread() override {
     try {
       while (!m_Quit) {
         ClientMessage message = (ClientMessage)ReadInt32();
         switch (message) {
           case ClientMessage::Message: {
             int senderId = ReadInt32();
             std::string argMessage;
             ReadStringUTF8(argMessage);
             CefPostTask(TID_RENDERER,
                         base::Bind(&CInteropImpl::OnClientMessage_Message,
                                    m_Host, senderId, argMessage));
           } break;
           default:
             throw "CInteropImplConnectionThread::ConnectionThread: Unknown message.";
         }
       }
     } catch (const std::exception& e) {
       DLOG(ERROR) << "Error in " << __FILE__ << ":" << __LINE__ << ": "
                   << e.what();
     }
   }

  private:
   CInteropImpl* m_Host;
   bool m_Quit = false;
 };

};

CefRefPtr<CefV8Value> CreateInterop(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context,
                                    const CefString& argStr,
                                    DWORD handlerId,
                                    CefRefPtr<CInterop>* out) {
  return CInteropImpl::Create(browser, frame, context, argStr, handlerId, out);
}

}  // namespace interop
}  // namespace advancedfx
