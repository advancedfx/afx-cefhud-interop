#pragma once

#include <list>
#include <string>

#include <malloc.h>

namespace advancedfx {
namespace interop {

class CRefCounted {
 public:
  CRefCounted() : m_RefCount(1) {}

  virtual void AddRef() { ++m_RefCount; }

  virtual void Release() {
    --m_RefCount;
    if (0 == m_RefCount)
      delete this;
  }

 protected:
  virtual ~CRefCounted() {}

 private:
  int m_RefCount;
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

class __declspec(novtable) ICommands abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual size_t GetCommands() const = 0;

  virtual size_t GetArgs(size_t index) const = 0;

  virtual const char* GetArg(size_t index, size_t argIndex) const = 0;
};

class __declspec(novtable) ICommandsCallback abstract{
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void CommandsCallback(const class ICommands* commands) = 0;
};


class __declspec(novtable) IRenderViewBeginCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void RenderViewBeginCallback(const struct RenderInfo_s& renderInfo,
                                bool& outBeforeTranslucentShadow,
                                bool& outAfterTranslucentShadow,
                                bool& outBeforeTranslucent,
                                bool& outAfterTranslucent,
                                bool& outBeforeHud,
                                bool& outAfterHud,
      bool& outAfterRenderView) = 0;
};

class __declspec(novtable) IRenderPassCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void RenderPassCallback(enum RenderPassType_e pass,
                                    const struct View_s& view) = 0;
};

class __declspec(novtable) IEventCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void EventCallback() = 0;
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

struct GameEventField_s {
  const char* Key = "";

  GameEventFieldType Type = GameEventFieldType::Local;

  union {
    char* CString;
    float Float;
    long Long;
    short Short;
    unsigned char Byte;
    bool Bool;
    uint64_t UInt64;
  } Value;

  bool HasEnrichmentUseridWithSteamId = false;
  uint64_t EnrichmentUseridWithSteamId = 0;

  bool HasEnrichmentEntnumWithOrigin = false;
  struct Vector_s EnrichmentEntnumWithOrigin;

  bool HasEnrichmentEntnumWithAngles = false;
  struct QAngle_s EnrichmentEntnumWithAngles;

  bool HasEnrichmentUseridWithEyePosition = false;
  struct Vector_s EnrichmentUseridWithEyePosition;

  bool HasEnrichmentUseridWithEyeAngels = false;
  struct QAngle_s EnrichmentUseridWithEyeAngels;

  GameEventField_s(const char* fieldKey, const std::string& fieldValue)
      : Key(fieldKey), Type(GameEventFieldType::CString) {
    if (nullptr != (Value.CString = (char*)malloc(sizeof(char) * (fieldValue.length() + 1))))
      memcpy(Value.CString, fieldValue.c_str(), fieldValue.length() + 1);
  }
  GameEventField_s(const char* fieldKey, float fieldValue)
      : Key(fieldKey), Type(GameEventFieldType::Float) {
    Value.Float = fieldValue;
  }
  GameEventField_s(const char* fieldKey, long fieldValue)
      : Key(fieldKey), Type(GameEventFieldType::Long) {
    Value.Long = fieldValue;
  }
  GameEventField_s(const char* fieldKey, short fieldValue)
      : Key(fieldKey), Type(GameEventFieldType::Short) {
    Value.Short = fieldValue;
  }
  GameEventField_s(const char* fieldKey, unsigned char fieldValue)
      : Key(fieldKey), Type(GameEventFieldType::Byte) {
    Value.Byte = fieldValue;
  }
  GameEventField_s(const char* fieldKey, bool fieldValue)
      : Key(fieldKey), Type(GameEventFieldType::Bool) {
    Value.Bool = fieldValue;
  }
  GameEventField_s(const char* fieldKey, unsigned __int64 fieldValue)
      : Key(fieldKey), Type(GameEventFieldType::Uint64) {
    Value.UInt64 = fieldValue;
  }

  ~GameEventField_s() {
    if (Type == GameEventFieldType::CString)
      free(Value.CString);
  }
};

struct GameEvent_s {
  const char* Name = "";

  bool HasClientTime = false;
  float ClientTime = 0;

  bool HasTick = false;
  int Tick = 0;

  bool HasSystemTime = false;
  uint64_t SystemTime = 0;

  std::list<GameEventField_s> Keys;
};

class __declspec(novtable) IGameEventCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void GameEventCallback(const struct GameEvent_s* result) = 0;
};

class __declspec(novtable) IHandleCalcCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void HandleCalcCallback(const struct HandleCalcResult_s* result) = 0;
};

class __declspec(novtable) IVecAngCalcCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void VecAngCalcCallback(
      const struct VecAngCalcResult_s* result) = 0;
};

class __declspec(novtable) ICamCalcCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void CamCalcCallback(
      const struct CamCalcResult_s* result) = 0;
};

class __declspec(novtable) IFovCalcCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void FovCalcCallback(const struct FovCalcResult_s* result) = 0;
};

class __declspec(novtable) IBoolCalcCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void BoolCalcCallback(
      const struct BoolCalcResult_s* result) = 0;
};

class __declspec(novtable) IIntCalcCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void IntCalcCallback(
      const struct IntCalcResult_s* result) = 0;
};

class __declspec(novtable) IOnViewOverrideCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual bool OnViewOverrideCallback(float& Tx,
                                      float& Ty,
                                      float& Tz,
                                      float& Rx,
                                      float& Ry,
                                      float& Rz,
                                      float& Fov) = 0;
};

class __declspec(novtable) IEngineInterop abstract
{
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual bool Connection() = 0;

  virtual bool Connected() = 0;

  virtual void Close() = 0;

  virtual const char* GetPipeName() const = 0;

  virtual void SetPipeName(const char* value) = 0;

  virtual void SetCommandsCallback(class ICommandsCallback* commandsCallback) = 0;

  virtual void ScheduleCommand(const char* command) = 0;

  virtual void SetNewConnectionCallback(
      class IEventCallback* eventCallback) = 0;

  virtual void SetOnViewOverrideCallback(
      class IOnViewOverrideCallback* onViewOverrideCallback) = 0;

  virtual void SetRenderViewBeginCallback(class IRenderViewBeginCallback* renderCallback) = 0;

  virtual void SetRenderPassCallback(
      class IRenderPassCallback* renderPassCallback) = 0;

  virtual void SetHudBeginCallback(
      class IEventCallback* eventCallback) = 0;

  virtual void SetHudEndCallback(
      class IEventCallback* eventCallback) = 0;

  virtual void SetRenderViewEndCallback(
      class IEventCallback* eventCallback) = 0;

  virtual void AddHandleCalcCallback(const char* name,
                                     class IHandleCalcCallback* callback) = 0;

  virtual void RemoveHandleCalcCallback(
      const char* name,
      class IHandleCalcCallback* callback) = 0;

  virtual void AddVecAngCalcCallback(const char* name,
                                     class IVecAngCalcCallback* callback) = 0;

  virtual void RemoveVecAngCalcCallback(const char* name,
                                     class IVecAngCalcCallback* callback) = 0;

  virtual void AddCamCalcCallback(const char* name,
                                     class ICamCalcCallback* callback) = 0;

  virtual void RemoveCamCalcCallback(const char* name,
                                  class ICamCalcCallback* callback) = 0;

  virtual void AddFovCalcCallback(const char* name,
                                  class IFovCalcCallback* callback) = 0;

  virtual void RemoveFovCalcCallback(const char* name,
                                     class IFovCalcCallback* callback) = 0;

  virtual void AddBoolCalcCallback(const char* name,
                                   class IBoolCalcCallback* callback) = 0;

  virtual void RemoveBoolCalcCallback(const char* name,
                                   class IBoolCalcCallback* callback) = 0;

  virtual void AddIntCalcCallback(const char* name,
                                  class IIntCalcCallback* callback) = 0;
  virtual void RemoveIntCalcCallback(const char* name,
                                  class IIntCalcCallback* callback) = 0;


  virtual void GameEvents_AllowAdd(const char* pszEvent) = 0;
  virtual void GameEvents_AllowRemove(const char* pszEvent) = 0;
  virtual void GameEvents_DenyAdd(const char* pszEvent) = 0;
  virtual void GameEvents_DenyRemove(const char* pszEvent) = 0;
  virtual void GameEvents_SetEnrichment(const char* pszEvent,
                                        const char* pszProperty,
                                        unsigned int uiEnrichments) = 0;

  virtual void SetGameEventCallback(
      class IGameEventCallback* gameEventCallback) = 0;

  virtual void GameEvents_SetTransmitClientTime(bool value) = 0;
  virtual void GameEvents_SetTransmitTick(bool value) = 0;
  virtual void GameEvents_SetTransmitSystemTime(bool value) = 0;
};

class IEngineInterop* CreateEngineInterop(const char * pipeName);

class __declspec(novtable) IDrawingInterop abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual bool Connection() = 0;

  virtual bool Connection(int frameCount,
                          void* sharedTextureHandle,
                          int width,
                          int height) = 0;

  virtual bool Connected() = 0;

  virtual void Close() = 0;

  virtual const char* GetPipeName() const = 0;

  virtual void SetPipeName(const char* value) = 0;
};

class IDrawingInterop* CreateDrawingInterop(const char* pipeName);

}  // namespace interop
}  // namespace advancedfx
