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
#include <iostream>

ContextManager cm;
MatrixStack ms;

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
        ms = MatrixStack();
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

void sglCircle(float x, float y, float z, float radius) {
    
}

void sglEllipse(float x, float y, float z, float a, float b) {}

void sglArc(float x, float y, float z, float radius, float from, float to) {}

//---------------------------------------------------------------------------
// Transform functions
//---------------------------------------------------------------------------

void sglMatrixMode(sglEMatrixMode mode) {
    switch (mode) {
    case(sglEMatrixMode::SGL_PROJECTION):
        ms.SetMode(false);
        break;
    default:
        ms.SetMode(true);
        break;
    }
}

void sglPushMatrix(void) {
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
    try {
        ms.Pop();
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
}

void sglLoadIdentity(void) {
    try {
        auto matrix = Matrix::Eye(4);
        ms.Push(matrix);
    }
    catch (OutOfMemoryException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
}

void sglLoadMatrix(const float *matrix) {
    try {
        auto mat = std::make_shared<Matrix>(4, 4, matrix);
        ms.Push(mat);
    }
    catch (OutOfMemoryException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
}

void sglMultMatrix(const float *matrix) {
    std::shared_ptr<Matrix> mat;
    std::shared_ptr<Matrix> temp_mat;
    try {
        mat = ms.Top();
        temp_mat = std::make_shared<Matrix>(4, 4, matrix);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
    mat->Matmul(temp_mat);
  }

void sglTranslate(float x, float y, float z) {
    std::shared_ptr<Matrix> mat;
    std::shared_ptr<Matrix> temp_mat;
    try {
        mat = ms.Top();
        temp_mat = Matrix::Translation3D(x, y, z);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
    mat->Matmul(temp_mat);
    //Matrix::PrintMatrix(mat);
}

void sglScale(float scalex, float scaley, float scalez) {
    std::shared_ptr<Matrix> mat;
    std::shared_ptr<Matrix> temp_mat;
    try {
        mat = ms.Top();
        temp_mat = Matrix::Scale(3, scalex, scaley, scalez);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
    mat->Matmul(temp_mat);
    //Matrix::PrintMatrix(mat);
}

void sglRotate2D(float angle, float centerx, float centery) {
    std::shared_ptr<Matrix> mat;
    std::shared_ptr<Matrix> temp_mat;
    try {
        mat = ms.Top();
        //to perform rotation with a point given as center first translate to point to 
        // be at the origin of coordinate system, rotate and then translate back
        // however, because of matrix transformations being applied from right
        // to left we actually need to do this in reverse order
        temp_mat = Matrix::Translation3D(centerx, centery, 0);
        temp_mat->Matmul(Matrix::Rotation3D(angle, sglAxis::Z_AXIS));
        temp_mat->Matmul(Matrix::Translation3D(-centerx, -centery, 0));
        //Matrix::PrintMatrix(temp_mat);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
    mat->Matmul(temp_mat);
    //Matrix::PrintMatrix(mat);
}

void sglRotateY(float angle) {
    std::shared_ptr<Matrix> mat;
    std::shared_ptr<Matrix> temp_mat;
    try {
        mat = ms.Top();
        temp_mat = Matrix::Rotation3D(angle, sglAxis::Y_AXIS);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
    mat->Matmul(temp_mat);
}

void sglOrtho(float left, float right, float bottom, float top, float near, float far) {
    std::shared_ptr<Matrix> mat;
    std::shared_ptr<Matrix> temp_mat;
    try {
        mat = ms.Top();
        temp_mat = Matrix::Orthographic3D(left, right, bottom, top, near, far);
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
    mat->Matmul(temp_mat);
}

void sglFrustum(float left, float right, float bottom, float top, float near, float far) {}

void sglViewport(int x, int y, int width, int height) {
    std::shared_ptr<Matrix> mat;
    try {
        mat = Matrix::Viewport(x, y, width, height);
    }
    catch (OutOfMemoryException& ex2) {
        std::cerr << ex2.what() << std::endl;
        setErrCode(SGL_OUT_OF_MEMORY);
    }
    if (sglGetError() > SGL_NO_ERROR) {
        return;
    }
    ms.SetViewport(mat);
    //Matrix::PrintMatrix(mat);
}

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
{
    //scale and transform the coordinates first the radius first
    try {
        auto mat = ms.Top();
        //square root of determinant of the upper left 2x2 part of the matrix
        // times radius is the new radius
        auto new_radius = radius * sqrt((*mat)(0, 0) * (*mat)(1, 1) - ((*mat)(1, 0) * (*mat)(0, 1)));
        std::shared_ptr<Vec4> original_vec(new Vec4(x, y, z, 1));
        auto temp_vec = mat->Matmul(original_vec);
        auto new_x = temp_vec->x;
        auto new_y = temp_vec->y;
        // draw the "first octant" starting point
        int current_x = round(new_x);
        //if y == 0 is at the bottom of the screen otherwise it would be added
        int current_y = round(new_y + new_radius);
        //initialize the Bressenham algorithm constants
        int dvex = 3 + 2 * new_x;
        int dvey = 2 * current_y - 2;
        //starting decision constant if you put current_x and 
        // current_y into the parametric circle equation
        int p = (current_x * current_x) + 2 * current_x + 1 
            + (new_y * new_y) + 2 * (new_y * new_radius)
             - new_y - new_radius;
        // (current_x * current_x) + 2 * current_x + 1 
        //+(new_y * new_y) - 2 * (new_y * new_radius)
        // - new_y + new_radius; if y == 0 is at the top of the screen

        while (current_x <= current_y) {
            //draw the 8 symmetrical vertices
            sglVertex4f(current_x, current_y, z, 1);
            sglVertex4f(new_x - current_x, current_y, z, 1);
            sglVertex4f(current_x, new_y - current_y, z, 1);
            sglVertex4f(new_x  - current_x, new_y - current_y, z, 1);
            sglVertex4f(current_y, current_x, z, 1);
            sglVertex4f(new_y - current_y, current_x, z, 1);
            sglVertex4f(current_y, new_x - current_x, z, 1);
            sglVertex4f(new_y - current_y, new_x - current_x, z, 1);
            if (p > 0) {
                p = p - dvey;
                dvey = dvey - 2;
                current_y = current_y - 2;
            }
            p = p + dvex;
            dvex = dvex + 2;
            current_x = current_x + 1;

        }
    }
    catch (MatrixStackUnderflowException& ex1) {
        std::cerr << ex1.what() << std::endl;
        setErrCode(SGL_STACK_UNDERFLOW);
    }
}

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
