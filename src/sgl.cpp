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
#include "exceptions.h"
#include "matrix.h"
#include "matrixStack.h"
#include <iostream>

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
    int ret = -1;
    try {
        ret = cm.CreateContext(width, height);
    }
    catch (const OutOfMemoryException& ex1) {
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    catch (const SGLOutOfResourcesException& ex2) {
        setErrCode(SGL_OUT_OF_RESOURCES);
    }

    return ret;
}

void sglDestroyContext(int id) {
    try {
        cm.DestroyContext(id);
    }
    catch (const SGLInvalidValueException& ex1) {
        setErrCode(SGL_INVALID_VALUE);
    }
    catch (const SGLInvalidOperationException& ex2) {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglSetContext(int id) {
    try {
        cm.SetContext(id);
    }
    catch (const SGLInvalidValueException& ex) {
        setErrCode(SGL_INVALID_VALUE);
    }
    
}

int sglGetContext(void) {
    int ret = -1;
    try {
        ret = cm.GetContext();
    }
    catch (const SGLInvalidOperationException& ex) {
        setErrCode(SGL_INVALID_OPERATION);
    }
    return ret;
}

float *sglGetColorBufferPointer(void) {
    return cm.GetColorBufferPtr();
}

//---------------------------------------------------------------------------
// Drawing functions
//---------------------------------------------------------------------------

void sglClearColor(float r, float g, float b, float alpha) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    try {
        cc->SetClearColor(r, g, b);
    }
    catch (const SGLInvalidOperationException& ex) {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglClear(unsigned what) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    try {
        cc->ClearBuffer(what);
    }
    catch (const SGLInvalidOperationException& ex1) {
        setErrCode(SGL_INVALID_OPERATION);
    }
    catch (const SGLInvalidValueException& ex2) {
        setErrCode(SGL_INVALID_VALUE);
    }
}

void sglBegin(sglEElementType mode) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    try {
        cc->BeginDrawing(mode);
    }
    catch (const SGLInvalidEnumException& ex1) {
        setErrCode(SGL_INVALID_ENUM);
    }
    catch (const SGLInvalidOperationException& ex2) {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglEnd(void) {
    //sglEErrorCode error = SGL_NO_ERROR;
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    try {
        cc->EndDrawing();
    }
    catch (const SGLInvalidOperationException& ex)
    {
        setErrCode(SGL_INVALID_OPERATION);
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

void sglCircle(float x, float y, float z, float radius) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }
    if (radius <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    try {

        cc->BresenhamCircle(x, y, z, radius);
    }
    catch (const SGLInvalidOperationException& ex) {
        std::cerr << "In circle" << std::endl;
        std::cerr << ex.what() << std::endl;
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglEllipse(float x, float y, float z, float a, float b) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }
    if (a <= 0 || b <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    try {

        cc->DrawEllipse(x, y, z, a, b);
    }
    catch (const SGLInvalidOperationException& ex) {
        std::cerr << "In ellipse" << std::endl;
        std::cerr << ex.what() << std::endl;
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglArc(float x, float y, float z, float radius, float from, float to) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }
    if (radius <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    try {

        cc->DrawArc(x, y, z, radius, from, to);
    }
    catch (const SGLInvalidOperationException& ex) {
        std::cerr << "In arc" << std::endl;
        std::cerr << ex.what() << std::endl;
        setErrCode(SGL_INVALID_OPERATION);
    }
}

//---------------------------------------------------------------------------
// Transform functions
//---------------------------------------------------------------------------

void sglMatrixMode(sglEMatrixMode mode) {
    //TODO missing exceptions
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        MatrixStack& ms = cc->matrix_stack;
        switch (mode) {

        case(sglEMatrixMode::SGL_PROJECTION):
            ms.SetMode(SGL_PROJECTION);
            break;
        default:
            ms.SetMode(SGL_MODELVIEW);
            break;
        }
    } else { //no context allocated yet
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglPushMatrix(void) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;
    try {
        ms.Duplicate();
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (MatrixStackOverflowException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_STACK_OVERFLOW);
    }
}

void sglPopMatrix(void) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;
    try {
        ms.Pop();
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
}

void sglLoadIdentity(void) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        //TODO perhaps this should change stack top?
        ms.Push(Matrix::Eye(4));
    }
    catch (OutOfMemoryException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
}

void sglLoadMatrix(const float *matrix) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        ms.Push(Matrix(4, 4, matrix));
    }
    catch (OutOfMemoryException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
}

void sglMultMatrix(const float *matrix) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        //const Matrix& mat = ms.Top();
        //TODO implement multiplication by (const float* matrix) to avoid copying
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix(4, 4, matrix));
        ms.Pop();
        ms.Push(new_mat);

    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    catch (BadDimensionException& ex3) {
        std::cerr << ex3.what() << std::endl;
        setErrCode(SGL_INTERNAL_ERROR);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
  }

void sglTranslate(float x, float y, float z) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        //const Matrix& mat = ms.Top();
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Translation3D(x, y, z));
        ms.Pop();
        ms.Push(new_mat);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    catch (BadDimensionException& ex3) {
        std::cerr << ex3.what() << std::endl;
        setErrCode(SGL_INTERNAL_ERROR);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
}

void sglScale(float scalex, float scaley, float scalez) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        //const Matrix& mat = ms.Top();
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Scale(scalex, scaley, scalez));
        ms.Pop();
        ms.Push(new_mat);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    catch (BadDimensionException& ex3) {
        std::cerr << ex3.what() << std::endl;
        setErrCode(SGL_INTERNAL_ERROR);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
}

void sglRotate2D(float angle, float centerx, float centery) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }
    MatrixStack& ms = cc->matrix_stack;

    try {
        //const Matrix& mat = ms.Top();
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::RotateAroundCenter(centerx, centery, angle));
        ms.Pop();
        ms.Push(new_mat);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    catch (BadDimensionException& ex3) {
        std::cerr << ex3.what() << std::endl;
        setErrCode(SGL_INTERNAL_ERROR);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
}

void sglRotateY(float angle) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        //const Matrix& mat = ms.Top();
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Rotation3D(angle, sglAxis::Y_AXIS));
        ms.Pop();
        ms.Push(new_mat);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    catch (BadDimensionException& ex3) {
        std::cerr << ex3.what() << std::endl;
        setErrCode(SGL_INTERNAL_ERROR);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
}

void sglOrtho(float left, float right, float bottom, float top, float near, float far) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        //const Matrix& mat = ms.Top();
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Orthographic3D(left, right, top, bottom, near, far));
        ms.Pop();
        ms.Push(new_mat);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    catch (BadDimensionException& ex3) {
        std::cerr << ex3.what() << std::endl;
        setErrCode(SGL_INTERNAL_ERROR);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
}

void sglFrustum(float left, float right, float bottom, float top, float near, float far) {}

void sglViewport(int x, int y, int width, int height) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    try {
        ms.SetViewport(Matrix::Viewport(x, y, width, height));
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
}

//---------------------------------------------------------------------------
// Attribute functions
//---------------------------------------------------------------------------

void sglColor3f(float r, float g, float b) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        try {
            cc->SetDrawingColor(r, g, b);
        }
        catch (const SGLInvalidOperationException& ex) {
            setErrCode(SGL_INVALID_OPERATION);
        }
    } else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglAreaMode(sglEAreaMode mode) {
    //TODO hw2
}

void sglPointSize(float size) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        try {
            cc->SetPointSize(size);
        }
        catch (const SGLInvalidOperationException& ex1) {
            setErrCode(SGL_INVALID_OPERATION);
        }
        catch (const SGLInvalidValueException& ex2) {
            setErrCode(SGL_INVALID_VALUE);
        }
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglEnable(sglEEnableFlags cap) {
    //TODO for depthbuffer test
}

void sglDisable(sglEEnableFlags cap) {
    //TODO for depthbuffer test
}

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
