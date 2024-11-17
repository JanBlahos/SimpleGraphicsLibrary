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

//macro to prevent listing 2-3 exceptions manually during every sgl call
#define TRY_HANDLE_EXCEPTIONS(expr)                                 \
    try {                                                           \
        expr                                                        \
    } catch (const SGLInvalidValueException& ex1) {                 \
        setErrCode(sglEErrorCode::SGL_INVALID_VALUE);               \
        std::cerr << "Exception caught: " << ex1.what() << "\n";    \
    } catch (const SGLInvalidEnumException& ex2) {                  \
        setErrCode(sglEErrorCode::SGL_INVALID_ENUM);                \
        std::cerr << "Exception caught: " << ex2.what() << "\n";    \
    } catch (const SGLInvalidOperationException& ex3) {             \
        setErrCode(sglEErrorCode::SGL_INVALID_OPERATION);           \
        std::cerr << "Exception caught: " << ex3.what() << "\n";    \
    } catch (const SGLOutOfResourcesException& ex4) {               \
        setErrCode(sglEErrorCode::SGL_OUT_OF_RESOURCES);            \
        std::cerr << "Exception caught: " << ex4.what() << "\n";    \
    } catch (const SGLInternalErrorException& ex5) {                \
        setErrCode(sglEErrorCode::SGL_INTERNAL_ERROR);              \
        std::cerr << "Exception caught: " << ex5.what() << "\n";    \
    } catch (const MatrixStackOverflowException& ex6) {             \
        setErrCode(sglEErrorCode::SGL_STACK_OVERFLOW);              \
        std::cerr << "Exception caught: " << ex6.what() << "\n";    \
    } catch (const MatrixStackUnderflowException& ex7) {            \
        setErrCode(sglEErrorCode::SGL_STACK_UNDERFLOW);             \
        std::cerr << "Exception caught: " << ex7.what() << "\n";    \
    } catch (const OutOfMemoryException& ex8) {                     \
        setErrCode(sglEErrorCode::SGL_OUT_OF_MEMORY);               \
        std::cerr << "Exception caught: " << ex8.what() << "\n";    \
    } catch (const std::bad_alloc& ex9) {                           \
        setErrCode(sglEErrorCode::SGL_OUT_OF_MEMORY);               \
        std::cerr << "Exception caught: " << ex9.what() << "\n";    \
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
    TRY_HANDLE_EXCEPTIONS({
        cm = ContextManager();
    })
}

void sglFinish(void) {}

int sglCreateContext(int width, int height) {
    int ret = -1;
    TRY_HANDLE_EXCEPTIONS({
        ret = cm.CreateContext(width, height);
    })
    return ret;
}

void sglDestroyContext(int id) {
    TRY_HANDLE_EXCEPTIONS({
        cm.DestroyContext(id);
    })
}

void sglSetContext(int id) {
    TRY_HANDLE_EXCEPTIONS({
        cm.SetContext(id);
    })
}

int sglGetContext(void) {
    int ret = -1;
    TRY_HANDLE_EXCEPTIONS({
        ret = cm.GetContext();
    })
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

    TRY_HANDLE_EXCEPTIONS({
        cc->SetClearColor(r, g, b);
    })
}

void sglClear(unsigned what) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }
    TRY_HANDLE_EXCEPTIONS({
        cc->ClearBuffer(what);
    })
}

void sglBegin(sglEElementType mode) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    TRY_HANDLE_EXCEPTIONS({
        cc->BeginDrawing(mode);
    })
}

void sglEnd(void) {
    //sglEErrorCode error = SGL_NO_ERROR;
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    TRY_HANDLE_EXCEPTIONS({
        cc->EndDrawing();
    })
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

    TRY_HANDLE_EXCEPTIONS({
        cc->BresenhamCircle(x, y, z, radius);
    })
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

    TRY_HANDLE_EXCEPTIONS({
        cc->DrawEllipse(x, y, z, a, b);
    })
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

    TRY_HANDLE_EXCEPTIONS({
        cc->DrawArc(x, y, z, radius, from, to);
    })
}

//---------------------------------------------------------------------------
// Transform functions
//---------------------------------------------------------------------------

void sglMatrixMode(sglEMatrixMode mode) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {
        if (cc->IsDrawing()) {
            setErrCode(SGL_INVALID_OPERATION);
            return;
        }

        MatrixStack& ms = cc->matrix_stack;
        switch (mode) {

        case(sglEMatrixMode::SGL_PROJECTION):
            TRY_HANDLE_EXCEPTIONS({
                ms.SetMode(SGL_PROJECTION);
            })
            break;
        default:
            TRY_HANDLE_EXCEPTIONS({
                ms.SetMode(SGL_MODELVIEW);
            })
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

    TRY_HANDLE_EXCEPTIONS({
        ms.Duplicate();
    })
}

void sglPopMatrix(void) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        ms.Pop();
    })    
}

void sglLoadIdentity(void) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        ms.Push(Matrix::Eye(4));
    })

}

void sglLoadMatrix(const float *matrix) {
    //TODO implement matrix mult by const float* so that
    //we avoid copying matrix
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        ms.Push(Matrix(4, 4, matrix));
    })
}

void sglMultMatrix(const float *matrix) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        //TODO implement multiplication by (const float* matrix) to avoid copying
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix(4, 4, matrix));
        ms.Pop();
        ms.Push(new_mat);
    })
  }

void sglTranslate(float x, float y, float z) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;


    TRY_HANDLE_EXCEPTIONS({
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Translation3D(x, y, z));
        ms.Pop();
        ms.Push(new_mat);
    })
}

void sglScale(float scalex, float scaley, float scalez) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Scale(scalex, scaley, scalez));
        ms.Pop();
        ms.Push(new_mat);
    })
}

void sglRotate2D(float angle, float centerx, float centery) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }
    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::RotateAroundCenter(centerx, centery, angle));
        ms.Pop();
        ms.Push(new_mat);
    })
}

void sglRotateY(float angle) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Rotation3D(angle, sglAxis::Y_AXIS));
        ms.Pop();
        ms.Push(new_mat);
    })
}

void sglOrtho(float left, float right, float bottom, float top, float near, float far) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Orthographic3D(left, right, top, bottom, near, far));
        ms.Pop();
        ms.Push(new_mat);
    })
}

void sglFrustum(float left, float right, float bottom, float top, float near, float far) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        Matrix new_mat = Matrix::Matmul(ms.Top(), Matrix::Frustum3D(left, right, bottom, top, near, far));
        ms.Pop();
        ms.Push(new_mat);
        })
}

void sglViewport(int x, int y, int width, int height) {
    Context* cc = cm.current_context;
    if (cc == nullptr) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    MatrixStack& ms = cc->matrix_stack;

    TRY_HANDLE_EXCEPTIONS({
        ms.SetViewport(Matrix::Viewport(x, y, width, height));
    })
}

//---------------------------------------------------------------------------
// Attribute functions
//---------------------------------------------------------------------------

void sglColor3f(float r, float g, float b) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {

        TRY_HANDLE_EXCEPTIONS({
            cc->SetDrawingColor(r, g, b);
        })
    } else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglAreaMode(sglEAreaMode mode) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {

        TRY_HANDLE_EXCEPTIONS({
            cc->SetAreaMode(mode);
            })
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglPointSize(float size) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {

        TRY_HANDLE_EXCEPTIONS({
            cc->SetPointSize(size);
        })
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglEnable(sglEEnableFlags cap) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {

        TRY_HANDLE_EXCEPTIONS({
            cc->Enable(cap);
            })
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglDisable(sglEEnableFlags cap) {
    Context* cc = cm.current_context;
    if (cc != nullptr) {

        TRY_HANDLE_EXCEPTIONS({
            cc->Disable(cap);
            })
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

//---------------------------------------------------------------------------
// RayTracing oriented functions
//---------------------------------------------------------------------------

void sglBeginScene() {
    Context* cc = cm.current_context;
    if (cc != nullptr) {

        TRY_HANDLE_EXCEPTIONS({
            cc->BeginScene();
            })
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

void sglEndScene() {
    Context* cc = cm.current_context;
    if (cc != nullptr) {

        TRY_HANDLE_EXCEPTIONS({
            cc->EndScene();
            })
    }
    else {
        setErrCode(SGL_INVALID_OPERATION);
    }
}

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
