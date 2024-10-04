//---------------------------------------------------------------------------
// sgl.cpp
// Empty implementation of the SGL (Simple Graphics Library)
// Date:  2016/10/24
// Author: Jaroslav Sloup
// Author: Jaroslav Krivanek, Jiri Bittner, CTU Prague
// Edited: Jakub Hendrich, Daniel Meister, CTU Prague
//---------------------------------------------------------------------------

#include "sgl.h"

#include "contextManager.h"
#include "context.h"

ContextManager cm;

/// Current error code.
static sglEErrorCode _libStatus = SGL_NO_ERROR;

static inline void setErrCode(sglEErrorCode c)
{
  if (_libStatus == SGL_NO_ERROR)
    _libStatus = c;
}

//---------------------------------------------------------------------------
// sglGetError()
//---------------------------------------------------------------------------
sglEErrorCode sglGetError(void)
{
  sglEErrorCode ret = _libStatus;
  _libStatus = SGL_NO_ERROR;
  return ret;
}

//---------------------------------------------------------------------------
// sglGetErrorString()
//---------------------------------------------------------------------------
const char* sglGetErrorString(sglEErrorCode error)
{
  static const char *errStrigTable[] =
  {
    "Operation succeeded",
    "Invalid argument(s) to a call",
    "Invalid enumeration argument(s) to a call",
    "Invalid call",
    "Quota of internal resources exceeded",
    "Internal library error",
    "Matrix stack overflow",
    "Matrix stack underflow",
    "Insufficient memory to finish the requested operation"
  };

  if ((int)error < (int)SGL_NO_ERROR || (int)error > (int)SGL_OUT_OF_MEMORY)
    return "Invalid value passed to sglGetErrorString()";

  return errStrigTable[(int)error];
}

//---------------------------------------------------------------------------
// Initialization functions
//---------------------------------------------------------------------------

void sglInit(void) {
    try {
        cm = ContextManager();
    }
    catch (const std::bad_alloc& ex) {
        setErrCode(SGL_OUT_OF_MEMORY);
    }
}

void sglFinish(void) {}

int sglCreateContext(int width, int height) {
    sglEErrorCode error = SGL_NO_ERROR;
    int ret = cm.CreateContext(width, height, error);
    if (error > SGL_NO_ERROR) setErrCode(error);
    return ret;
}

void sglDestroyContext(int id) {
    sglEErrorCode error= SGL_NO_ERROR;
    cm.DestroyContext(id, error);
    if (error > SGL_NO_ERROR) setErrCode(error);
}

void sglSetContext(int id) {
    sglEErrorCode error = SGL_NO_ERROR;
    cm.SetContext(id, error);
    if (error > SGL_NO_ERROR) setErrCode(error);
}

int sglGetContext(void) {
    sglEErrorCode error = SGL_NO_ERROR;
    int ret = cm.GetContext(error);
    if (error > SGL_NO_ERROR) setErrCode(error);
    return ret;
}

float *sglGetColorBufferPointer(void) {
    return cm.GetColorBufferPtr();
}

//---------------------------------------------------------------------------
// Drawing functions
//---------------------------------------------------------------------------

void sglClearColor(float r, float g, float b, float alpha) {
    sglEErrorCode error = SGL_NO_ERROR;
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        cc->SetClearColor(r, g, b, error);
        if (error > SGL_NO_ERROR) setErrCode(error);
    } else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglClear(unsigned what) {
    sglEErrorCode error = SGL_NO_ERROR;
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        cc->ClearBuffer(what, error);
        if (error > SGL_NO_ERROR) setErrCode(error);
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglBegin(sglEElementType mode) {
    //TODO this can be called before context initialization as explained
    // but is probably not the case
    sglEErrorCode error = SGL_NO_ERROR;
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        cc->BeginDrawing(mode, error);
        if (error > SGL_NO_ERROR) setErrCode(error);
    } else {
        //Note: this is not specified hence remains commented for now
        //setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglEnd(void) {
    sglEErrorCode error = SGL_NO_ERROR;
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        cc->EndDrawing(error);
        if (error > SGL_NO_ERROR) setErrCode(error);
    }
    else {
        //Note: this is not specified hence remains commented for now
        //setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglVertex4f(float x, float y, float z, float w) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        cc->BufferVertex4f(x, y, z, w);
    }
}

void sglVertex3f(float x, float y, float z) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        cc->BufferVertex3f(x, y, z);
    }
}

void sglVertex2f(float x, float y) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        cc->BufferVertex2f(x, y);
    }
}

void sglCircle(float x, float y, float z, float radius) {}

void sglEllipse(float x, float y, float z, float a, float b) {}

void sglArc(float x, float y, float z, float radius, float from, float to) {}

//---------------------------------------------------------------------------
// Transform functions
//---------------------------------------------------------------------------

void sglMatrixMode(sglEMatrixMode mode) {}

void sglPushMatrix(void) {}

void sglPopMatrix(void) {}

void sglLoadIdentity(void) {}

void sglLoadMatrix(const float *matrix) {}

void sglMultMatrix(const float *matrix) {}

void sglTranslate(float x, float y, float z) {}

void sglScale(float scalex, float scaley, float scalez) {}

void sglRotate2D(float angle, float centerx, float centery) {}

void sglRotateY(float angle) {}

void sglOrtho(float left, float right, float bottom, float top, float near, float far) {}

void sglFrustum(float left, float right, float bottom, float top, float near, float far) {}

void sglViewport(int x, int y, int width, int height) {}

//---------------------------------------------------------------------------
// Attribute functions
//---------------------------------------------------------------------------

void sglColor3f(float r, float g, float b) {}

void sglAreaMode(sglEAreaMode mode) {}

void sglPointSize(float size) {}

void sglEnable(sglEEnableFlags cap) {}

void sglDisable(sglEEnableFlags cap) {}

//---------------------------------------------------------------------------
// RayTracing oriented functions
//---------------------------------------------------------------------------

void sglBeginScene() {}

void sglEndScene() {}

void sglSphere(const float x,
               const float y,
               const float z,
               const float radius)
{}

void sglMaterial(const float r,
                 const float g,
                 const float b,
                 const float kd,
                 const float ks,
                 const float shine,
                 const float T,
                 const float ior)
{}

void sglPointLight(const float x,
                   const float y,
                   const float z,
                   const float r,
                   const float g,
                   const float b)
{}

void sglRayTraceScene() {}

void sglRasterizeScene() {}

void sglEnvironmentMap(const int width,
                       const int height,
                       float *texels)
{}

void sglEmissiveMaterial(const float r,
                         const float g,
                         const float b,
                         const float c0,
                         const float c1,
                         const float c2)
{}
