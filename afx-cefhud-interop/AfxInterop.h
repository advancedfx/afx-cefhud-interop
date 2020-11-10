#pragma once

#include <include/cef_v8.h>

#include <list>
#include <string>

#include <malloc.h>

#include <windows.h>
#include <d3d9types.h>

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

class __declspec(novtable) ISendProcessMessage abstract {
 public:
  virtual void SendProcessMessage(
      CefRefPtr<CefListValue> args) = 0;
};


class __declspec(novtable) IManagedD3d9DataObject {
 public:
  virtual void* GetData() = 0;

  virtual size_t GetDataSize() = 0;

  virtual void SetData(void* value) = 0;

  virtual void SetDataSize(size_t value) = 0;
};

class __declspec(novtable) IManagedD3d9TextureDataObject {
 public:
  virtual void* GetData(unsigned int level) = 0;

  virtual size_t GetDataRowCount(unsigned int level) = 0;

  virtual size_t GetDataRowBytes(unsigned int level) = 0;

  virtual void SetData(unsigned int level, void* value) = 0;

  virtual void SetDataRowCount(unsigned int level, size_t value) = 0;

  virtual void SetDataRowBytes(unsigned int level, size_t value) = 0;
};

class CEngineInterop : public CefV8Accessor, public CefV8Handler {
 public:
  virtual bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) = 0;
};

class CEngineInterop* CreateEngineInterop(
      CefRefPtr<CefBrowser> const& browser,
      CefRefPtr<CefFrame> const& frame,
    CefRefPtr<CefV8Context> const& context,
    const char* pipeName);


class __declspec(novtable) IDrawingInterop abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message,
                                        CefRefPtr<CefListValue> args) = 0;

  virtual void SetShardHandle(void * handle) = 0;
};

class IDrawingInterop* CreateDrawingInterop(const char* pipeName);

}  // namespace interop
}  // namespace advancedfx
