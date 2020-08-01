#include "AfxInterop.h"

#include <d3d11.h>
#include <tchar.h>

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <vector>

namespace advancedfx {
namespace interop {

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

  bool WriteHandle(HANDLE value) {
    DWORD value32 = HandleToULong(value);

    return WriteBytes(&value32, 0, sizeof(value32));
  }

  bool WriteSingle(FLOAT value) { return WriteBytes(&value, 0, sizeof(value)); }

  bool WriteStringUTF8(const std::string value) {
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

class CCommands : public CRefCounted, public ICommands {
 public:
  virtual void AddRef() { CRefCounted::AddRef();
  }

  virtual void Release() { CRefCounted::Release(); }


  virtual size_t GetCommands() const override { return m_Commands.size(); }

  virtual size_t GetArgs(size_t index) const override {
    if (index >= m_Commands.size())
      return 0;

    return m_Commands[index].GetArgs();
  }

  virtual const char* GetArg(size_t index, size_t argIndex) const override {
    if (index >= m_Commands.size())
      return "";

    return m_Commands[index].GetArg(argIndex);
  }

  void SetCommands(size_t numCommands) { m_Commands.resize(numCommands); }

  void SetArgs(size_t index, size_t numArgs) {
    m_Commands[index].SetArgs(numArgs);
  }

  std::string& GetArgString(size_t index, size_t argIndex) {
    return m_Commands[index].GetArgString(argIndex);
  }

 private:
  std::vector<CAfxCommand> m_Commands;
};


template <typename T, typename R>
class CCalcCallbacks abstract {
 public:
  ~CCalcCallbacks() {
    while (!m_Map.empty()) {
      while (!m_Map.begin()->second.empty()) {
        T* callback = (*m_Map.begin()->second.begin());
        callback->Release();
        m_Map.begin()->second.erase(m_Map.begin()->second.begin());
      }
      m_Map.erase(m_Map.begin());
    }
  }

  void Add(const char* name, T* callback) {
      callback->AddRef();
    m_Map[name].emplace(callback);
  }

  void Remove(const char* name, T* callback) {
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
  
  bool BatchUpdateRequest(CNamedPipeServer* pipeServer) {
    if (!pipeServer->WriteCompressedUInt32((UINT32)m_Map.size()))
      return false;
    for (typename std::map<std::string, std::set<T*>>::iterator it =
             m_Map.begin();
         it != m_Map.end(); ++it) {
      if (!pipeServer->WriteStringUTF8(it->first))
        return false;
    }

    return true;
  }

  bool BatchUpdateResult(CNamedPipeServer* pipeServer) {
    R result;

    for (typename std::map<std::string, std::set<T*>>::iterator it =
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

      for (typename std::set<T*>::iterator setIt = (*it).second.begin();
           setIt != (*it).second.end(); ++setIt) {
        CallResult(*setIt, resultPtr);
      }
    }

    return true;
  }

 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer, R& outResult) = 0;
  virtual void CallResult(T* callback, R* result) = 0;

 private:
  typename typedef std::set<T*> Callbacks_t;
  typedef std::map<std::string, Callbacks_t> NameToCallbacks_t;
  NameToCallbacks_t m_Map;
};

class CHandleCalcCallbacks
    : public CCalcCallbacks<class IHandleCalcCallback,
                                      struct HandleCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          HandleCalcResult_s& outResult) override {
    if (!pipeServer->ReadInt32(outResult.IntHandle))
      return false;

    return true;
  }

  virtual void CallResult(class IHandleCalcCallback* callback,
                          struct HandleCalcResult_s* result) override {
    callback->HandleCalcCallback(result);
  }
};

class CVecAngCalcCallbacks
    : public CCalcCallbacks<class IVecAngCalcCallback,
                                      struct VecAngCalcResult_s> {
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

  virtual void CallResult(class IVecAngCalcCallback* callback,
                          struct VecAngCalcResult_s* result) override {
    callback->VecAngCalcCallback(result);
  }
};

class CCamCalcCallbacks : public CCalcCallbacks<class ICamCalcCallback,
                                      struct CamCalcResult_s> {
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

  virtual void CallResult(ICamCalcCallback* callback,
                          struct CamCalcResult_s* result) override {
    callback->CamCalcCallback(result);
  }
};

class CFovCalcCallbacks
    : public CCalcCallbacks<class IFovCalcCallback,
                                      struct FovCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          struct FovCalcResult_s& outResult) override {
    if (!pipeServer->ReadSingle(outResult.Fov))
      return false;

    return true;
  }

  virtual void CallResult(class IFovCalcCallback* callback,
                          struct FovCalcResult_s* result) override {
    callback->FovCalcCallback(result);
  }
};

class CBoolCalcCallbacks
    : public CCalcCallbacks<class IBoolCalcCallback,
                                      struct BoolCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          struct BoolCalcResult_s& outResult) override {
    bool result;
    if (!pipeServer->ReadBoolean(result))
      return false;

    outResult.Result = result;

    return true;
  }

  virtual void CallResult(class IBoolCalcCallback* callback,
                          struct BoolCalcResult_s* result) override {
    callback->BoolCalcCallback(result);
  }
};

class CIntCalcCallbacks
    : public CCalcCallbacks<class IIntCalcCallback,
                                      struct IntCalcResult_s> {
 protected:
  virtual bool ReadResult(CNamedPipeServer* pipeServer,
                          struct IntCalcResult_s& outResult) override {
    INT32 result;
    if (!pipeServer->ReadInt32(result))
      return false;

    outResult.Result = result;

    return true;
  }

  virtual void CallResult(class IIntCalcCallback* callback,
                          struct IntCalcResult_s* result) override {
    callback->IntCalcCallback(result);
  }
};

class CInterop : public CRefCounted {
 public:
  const INT32 Version = 7;

  CInterop(const char* pipeName) : m_PipeName(pipeName) {}

  virtual bool Connection(void) {
    if (nullptr == m_PipeServer) {
      std::string pipeName(m_PipeName);

      m_PipeServer = new CNamedPipeServer(pipeName.c_str());
      m_Connecting = true;
    }

    CNamedPipeServer::State drawingState = m_PipeServer->Connect();

    switch (drawingState) {
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

 private:
  bool m_Connecting = false;
};

class CEngineInterop : public CInterop, public IEngineInterop {
 public:
  CEngineInterop(const char* pipeName)
      : CInterop(pipeName) {}

 virtual void AddRef() override { CInterop::AddRef(); }

 virtual void Release() override { CInterop::Release(); }

 virtual bool Connection() override { return CInterop::Connection(); }

 virtual bool Connected() override { return CInterop::Connected(); }

 virtual void Close() { CInterop::Close(); }

 virtual const char* GetPipeName() const { return CInterop::GetPipeName(); }

 virtual void SetPipeName(const char* value) { CInterop::SetPipeName(value); }

 virtual void SetCommandsCallback(
      class ICommandsCallback* commandsCallback) override {
    if (m_CommandsCallback)
      m_CommandsCallback->Release();

    m_CommandsCallback = commandsCallback;

    if (m_CommandsCallback)
      m_CommandsCallback->AddRef();
 }

  virtual void SetRenderViewBeginCallback(class IRenderViewBeginCallback* renderCallback) {
   if (m_RenderViewBeginCallback)
     m_RenderViewBeginCallback->Release();

   m_RenderViewBeginCallback = renderCallback;

   if (m_RenderViewBeginCallback)
     m_RenderViewBeginCallback->AddRef();
  }

  virtual void ScheduleCommand(const char* command) { m_Commands.emplace(command);
  }

  virtual void SetNewConnectionCallback(class IEventCallback* eventCallback) {
    if (m_NewConnectionCallback)
      m_NewConnectionCallback->Release();

    m_NewConnectionCallback = eventCallback;

    if (m_NewConnectionCallback)
      m_NewConnectionCallback->AddRef();
  }

  virtual void SetOnViewOverrideCallback(
      class IOnViewOverrideCallback* onViewOverrideCallback) {
    if (m_OnViewOverrideCallback)
      m_OnViewOverrideCallback->Release();

    m_OnViewOverrideCallback = onViewOverrideCallback;

    if (m_OnViewOverrideCallback)
      m_OnViewOverrideCallback->AddRef();
  }

  virtual void SetRenderPassCallback(
      class IRenderPassCallback* renderPassCallback) {
    if (m_RenderPassCallback)
      m_RenderPassCallback->Release();

    m_RenderPassCallback = renderPassCallback;

    if (m_RenderPassCallback)
      m_RenderPassCallback->AddRef();
  }

  virtual void SetHudBeginCallback(class IEventCallback* eventCallback) {
    if (m_HudBeginCallback)
      m_HudBeginCallback->Release();

    m_HudBeginCallback = eventCallback;
 }

  virtual void SetHudEndCallback(class IEventCallback* eventCallback) {
   if (m_HudEndCallback)
      m_HudEndCallback->Release();

   m_HudEndCallback = eventCallback;

   if (m_HudEndCallback)
     m_HudEndCallback->AddRef();
  }

  virtual void SetRenderViewEndCallback(class IEventCallback* eventCallback) {
    if (m_RenderViewEndCallback)
      m_RenderViewEndCallback->Release();

    m_RenderViewEndCallback = eventCallback;

    if (m_RenderViewEndCallback)
      m_RenderViewEndCallback->AddRef();
  }

  virtual void AddHandleCalcCallback(const char* name,
                                     class IHandleCalcCallback* callback) {
    m_HandleCalcCallbacks.Add(name, callback);
  }

  virtual void RemoveHandleCalcCallback(const char* name,
                                        class IHandleCalcCallback* callback) {
    m_HandleCalcCallbacks.Remove(name, callback);
  }

  virtual void AddVecAngCalcCallback(const char* name,
                                     class IVecAngCalcCallback* callback) {
    m_VecAngCalcCallbacks.Add(name, callback);
   }

  virtual void RemoveVecAngCalcCallback(const char* name,
                                         class IVecAngCalcCallback* callback) {
     m_VecAngCalcCallbacks.Remove(name, callback);
   }

  virtual void AddCamCalcCallback(const char* name,
                                   class ICamCalcCallback* callback) {
     m_CamCalcCallbacks.Add(name, callback);
  }

  virtual void RemoveCamCalcCallback(const char* name,
                                     class ICamCalcCallback* callback) {
    m_CamCalcCallbacks.Remove(name, callback);
  }

  virtual void AddFovCalcCallback(const char* name,
                                  class IFovCalcCallback* callback) {
    m_FovCalcCallbacks.Add(name, callback);
  }

  virtual void RemoveFovCalcCallback(const char* name,
                                     class IFovCalcCallback* callback) {
    m_FovCalcCallbacks.Remove(name, callback);
  }

  virtual void AddBoolCalcCallback(const char* name,
                                   class IBoolCalcCallback* callback) {
    m_BoolCalcCallbacks.Add(name, callback);
  }

  virtual void RemoveBoolCalcCallback(const char* name,
                                      class IBoolCalcCallback* callback) {
    m_BoolCalcCallbacks.Remove(name, callback);
  }

  virtual void AddIntCalcCallback(const char* name,
                                  class IIntCalcCallback* callback) {
    m_IntCalcCallbacks.Add(name, callback);
  }

  virtual void RemoveIntCalcCallback(const char* name,
                                     class IIntCalcCallback* callback) {
    m_IntCalcCallbacks.Remove(name, callback);
  }

 protected:
  ~CEngineInterop() {
    if (m_CommandsCallback)
      m_CommandsCallback->Release();
    if (m_RenderViewBeginCallback)
      m_RenderViewBeginCallback->Release();
    if (m_NewConnectionCallback)
      m_NewConnectionCallback->Release();
    if (m_OnViewOverrideCallback)
      m_OnViewOverrideCallback->Release();
    if (m_RenderPassCallback)
      m_RenderPassCallback->Release();
    if (m_HudBeginCallback)
      m_HudBeginCallback->Release();
    if (m_HudEndCallback)
      m_HudEndCallback->Release();
    if (m_RenderViewEndCallback)
      m_RenderViewEndCallback->Release();
  }


  virtual bool OnConnection(void) override {
    bool done = false;

    bool beforeTranslucentShadow;
    bool afterTranslucentShadow;
    bool beforeTranslucent;
    bool afterTranslucent;
    bool beforeHud;
    bool afterHud;
    bool afterRenderView;

    while (!done) {
      INT32 engineMessage;
      if (!m_PipeServer->ReadInt32(engineMessage))
        return false;

      switch (engineMessage) {
        case EngineMessage_BeforeFrameStart: {
          // Read incoming commands from client:
          {
            CCommands* commands = new CCommands();

            UINT32 commandIndex = 0;
            UINT32 commandCount;
            if (!m_PipeServer->ReadCompressedUInt32(commandCount))
              return false;

            commands->SetCommands(commandCount);

            while (0 < commandCount) {
              UINT32 argIndex = 0;
              UINT32 argCount;
              if (!m_PipeServer->ReadCompressedUInt32(argCount))
                return false;

              commands->SetArgs(commandIndex, argCount);

              while (0 < argCount) {
                if (!m_PipeServer->ReadStringUTF8(
                        commands->GetArgString(commandIndex, argIndex)))
                  return false;

                --argCount;
                ++argIndex;
              }

              --commandCount;
              ++commandIndex;
            }

            if(m_CommandsCallback) m_CommandsCallback->CommandsCallback(commands);
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

        case EngineMessage_BeforeFrameRenderStart: {
        } break;

        case EngineMessage_AfterFrameRenderStart: {
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

        case EngineMessage_OnRenderView: {
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

          beforeTranslucentShadow = false;
          afterTranslucentShadow = false;
          beforeTranslucent = false;
          afterTranslucent = false;
          beforeHud = false;
          afterHud = false;
          afterRenderView = false;

          if(m_RenderViewBeginCallback) m_RenderViewBeginCallback->RenderViewBeginCallback(
              renderInfo,
              beforeTranslucentShadow, afterTranslucentShadow,
              beforeTranslucent, afterTranslucent, beforeHud, afterHud,
              afterRenderView);

          if (!m_PipeServer->WriteBoolean(beforeTranslucentShadow))
            return false;
          if (!m_PipeServer->WriteBoolean(afterTranslucentShadow))
            return false;
          if (!m_PipeServer->WriteBoolean(beforeTranslucent))
            return false;
          if (!m_PipeServer->WriteBoolean(afterTranslucent))
            return false;
          if (!m_PipeServer->WriteBoolean(beforeHud))
            return false;
          if (!m_PipeServer->WriteBoolean(afterHud))
            return false;
          if (!m_PipeServer->WriteBoolean(afterRenderView))
            return false;

          if (!(beforeTranslucentShadow || afterTranslucentShadow ||
                beforeTranslucent || afterTranslucent || beforeHud || afterHud || afterRenderView))
            done = true;

          if (!done) {
            // Handle render pass here if needed ...
          }
        } break;

        case EngineMessage_OnRenderViewEnd:
          if (m_RenderViewEndCallback)
            m_RenderViewEndCallback->EventCallback();
          done = true;
          break;

        case EngineMessage_BeforeHud:
          if (m_HudBeginCallback)
            m_HudBeginCallback->EventCallback();
          break;

        case EngineMessage_AfterHud:
          if (m_HudEndCallback)
            m_HudEndCallback->EventCallback();
          break;

        case EngineMessage_BeforeTranslucentShadow:
          if (!DoRenderPass(RenderPassType_BeforeTranslucentShadow))
            return false;
          break;
        case EngineMessage_AfterTranslucentShadow:
          if (!DoRenderPass(RenderPassType_AfterTranslucentShadow))
            return false;
          break;
        case EngineMessage_BeforeTranslucent:
          if (!DoRenderPass(RenderPassType_BeforeTranslucent))
            return false;
          break;
        case EngineMessage_AfterTranslucent:
          if (!DoRenderPass(RenderPassType_AfterTranslucent))
            return false;
          break;

        case EngineMessage_OnViewOverride: {
          if (m_OnViewOverrideCallback) {
            float Tx, Ty, Tz, Rx, Ry, Rz, Fov;
            if (m_OnViewOverrideCallback->OnViewOverrideCallback(Tx, Ty, Tz, Rx, Ry, Rz, Fov)) {
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

    if (!m_PipeServer->Flush())
      return false;

    if (m_NewConnectionCallback)
      m_NewConnectionCallback->EventCallback();

    return true;
  }

 private:
  enum EngineMessage {
    EngineMessage_Invalid = 0,
    EngineMessage_LevelInitPreEntity = 1,
    EngineMessage_LevelShutDown = 2,
    EngineMessage_BeforeFrameStart = 3,
    EngineMessage_OnRenderView = 4,
    EngineMessage_OnRenderViewEnd = 5,
    EngineMessage_BeforeFrameRenderStart = 6,
    EngineMessage_AfterFrameRenderStart = 7,
    EngineMessage_OnViewOverride = 8,
    EngineMessage_BeforeTranslucentShadow = 9,
    EngineMessage_AfterTranslucentShadow = 10,
    EngineMessage_BeforeTranslucent = 11,
    EngineMessage_AfterTranslucent = 12,
    EngineMessage_BeforeHud = 13,
    EngineMessage_AfterHud = 14
  };

  class ICommandsCallback* m_CommandsCallback = nullptr;
  class IRenderViewBeginCallback* m_RenderViewBeginCallback = nullptr;

  std::queue<std::string> m_Commands;

  class IEventCallback* m_NewConnectionCallback = nullptr;
  class IOnViewOverrideCallback* m_OnViewOverrideCallback = nullptr;
  class IRenderPassCallback* m_RenderPassCallback = nullptr;
  class IEventCallback* m_HudBeginCallback = nullptr;
  class IEventCallback* m_HudEndCallback = nullptr;
  class IEventCallback* m_RenderViewEndCallback = nullptr;

  CHandleCalcCallbacks m_HandleCalcCallbacks;
  CVecAngCalcCallbacks m_VecAngCalcCallbacks;
  CCamCalcCallbacks m_CamCalcCallbacks;
  CFovCalcCallbacks m_FovCalcCallbacks;
  CBoolCalcCallbacks m_BoolCalcCallbacks;
  CIntCalcCallbacks m_IntCalcCallbacks;

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

    if(m_RenderPassCallback) m_RenderPassCallback->RenderPassCallback(pass, view);

    return true;
  }
};

class IEngineInterop* CreateEngineInterop(const char* pipeName) {
  return new CEngineInterop(pipeName);
}

class CDrawingInterop : public CInterop, public IDrawingInterop {
 public:
  CDrawingInterop(const char* pipeName) : CInterop(pipeName) {}

  virtual void AddRef() override { CInterop::AddRef(); }

  virtual void Release() override { CInterop::Release(); }

  virtual bool Connection() override { return CInterop::Connection();
  }

  virtual bool Connection(int frameCount, void* sharedTextureHandle, int width, int height) override {
    m_HasData = true;
    m_FrameCount = frameCount;
    m_Width = width;
    m_Height = height;
    m_SharedTextureHandle = sharedTextureHandle;
    return CInterop::Connection();
  }


  virtual bool Connected() override { return CInterop::Connected(); }

  virtual void Close() { CInterop::Close(); }

  virtual const char* GetPipeName() const { return CInterop::GetPipeName(); }

  virtual void SetPipeName(const char* value) { CInterop::SetPipeName(value); }

 private:
  enum DrawingMessage {
    DrawingMessage_Invalid = 0,
    DrawingMessage_PreapareDraw = 1,
    DrawingMessage_BeforeTranslucentShadow = 2,
    DrawingMessage_AfterTranslucentShadow = 3,
    DrawingMessage_BeforeTranslucent = 4,
    DrawingMessage_AfterTranslucent = 5,
    DrawingMessage_BeforeHud = 6,
    DrawingMessage_AfterHud = 7,
    DrawingMessage_OnRenderViewEnd = 8
  };

  enum PrepareDrawReply {
    PrepareDrawReply_Skip = 1,
    PrepareDrawReply_Retry = 2,
    PrepareDrawReply_Continue = 3
  };

  bool m_HasData = false;
  int m_FrameCount = -1;
  unsigned int m_Width = 0;
  unsigned int m_Height = 0;
  HANDLE m_SharedTextureHandle = nullptr;

  virtual bool OnConnection() override {
    while (true) {
      INT32 drawingMessage;
      if (!m_PipeServer->ReadInt32(drawingMessage))
        return false;

      switch (drawingMessage) {
        case DrawingMessage_BeforeTranslucentShadow:
        case DrawingMessage_AfterTranslucentShadow:
        case DrawingMessage_BeforeTranslucent:
        case DrawingMessage_AfterTranslucent:
        case DrawingMessage_BeforeHud:
        case DrawingMessage_AfterHud:
        case DrawingMessage_OnRenderViewEnd:
          break;
        
        default:
          return false; // Unsupported event
      }

      INT32 clientFrameCount;
      if (!m_PipeServer->ReadInt32(clientFrameCount))
        return false;

      if (!m_HasData) {
        if (!m_PipeServer->WriteInt32(PrepareDrawReply_Skip))
          return false;
        if (!m_PipeServer->Flush())
          return false;

        return true;
      }

      INT32 frameDiff = m_FrameCount - clientFrameCount;

      if (frameDiff < 0) {
        // Error: client is ahead, otherwise we would have correct data
        // by now.

        if (!m_PipeServer->WriteInt32(PrepareDrawReply_Retry))
          return false;
        if (!m_PipeServer->Flush())
          return false;

        return true;
      } else if (frameDiff > 0) {
        // client is behind.

        if (!m_PipeServer->WriteInt32(PrepareDrawReply_Skip))
          return false;
        if (!m_PipeServer->Flush())
          return false;
      } else {
        // we are right on.

        if (!m_PipeServer->WriteInt32(PrepareDrawReply_Continue))
          return false;
        if (!m_PipeServer->WriteHandle(m_SharedTextureHandle))
          return false;
        if (!m_PipeServer->WriteUInt32(m_Width))
          return false;
        if (!m_PipeServer->WriteUInt32(m_Height))
          return false;
        if (!m_PipeServer->WriteUInt32(21)) // D3DFMT_A8R8G8B8
          return false;
        if (!m_PipeServer->Flush())
          return false;

        bool finished;
        if (!m_PipeServer->ReadBoolean(finished))
          return false;
        if (!finished)
          return false;

        return true;
      }
    }
  }
};

class IDrawingInterop* CreateDrawingInterop(const char* pipeName) {
    return new CDrawingInterop(pipeName);
}

}  // namespace interop
}  // namespace advancedfx
