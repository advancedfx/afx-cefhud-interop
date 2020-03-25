#pragma once

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


class __declspec(novtable) IRenderCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void RenderCallback(const struct RenderInfo_s& renderInfo,
                                bool& outBeforeTranslucentShadow,
                                bool& outAfterTranslucentShadow,
                                bool& outBeforeTranslucent,
                                bool& outAfterTranslucent,
                                bool& outBeforeHud,
                                bool& outAfterHud) = 0;
};

class __declspec(novtable) IRenderPassCallback abstract {
 public:
  virtual void AddRef() = 0;

  virtual void Release() = 0;

  virtual void RenderPassCallback(enum RenderPassType_e pass,
                                    const struct View_s& view) = 0;
};

struct HandleCalcResult_s {
  int IntHandle;
};

struct Vector_s {
  float X;
  float Y;
  float Z;
};

struct QAngle_s {
  float Pitch;
  float Yaw;
  float Roll;
};

struct VecAngCalcResult_s {
  struct Vector_s Vector;
  struct QAngle_s QAngle;
};

struct CamCalcResult_s {
  struct Vector_s Vector;
  struct QAngle_s QAngle;
  float Fov;
};

struct FovCalcResult_s {
  float Fov;
};

struct BoolCalcResult_s {
  bool Result;
};

struct IntCalcResult_s {
  int Result;
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

  virtual void SetRenderCallback(class IRenderCallback* renderCallback) = 0;

  virtual void ScheduleCommand(const char* command) = 0;

  virtual void SetOnViewOverrideCallback(
      class IOnViewOverrideCallback* onViewOverrideCallback) = 0;


  virtual void SetRenderPassCallback(
      class IRenderPassCallback* renderPassCallback) = 0;

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
