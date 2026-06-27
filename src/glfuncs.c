#include "glfuncs.h"

#define X(type, name) \
    ((type)glXGetProcAddressARB((const GLubyte *)(name)))

GLCREATESHADERPROC pglCreateShader = NULL;
GLSHADERSOURCEPROC pglShaderSource = NULL;
GLCOMPILESHADERPROC pglCompileShader = NULL;
GLGETSHADERIVPROC pglGetShaderiv = NULL;
GLGETSHADERINFOLOGPROC pglGetShaderInfoLog = NULL;
GLCREATEPROGRAMPROC pglCreateProgram = NULL;
GLATTACHSHADERPROC pglAttachShader = NULL;
GLLINKPROGRAMPROC pglLinkProgram = NULL;
GLGETPROGRAMIVPROC pglGetProgramiv = NULL;
GLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog = NULL;
GLGENVERTEXARRAYSPROC pglGenVertexArrays = NULL;
GLGENBUFFERSPROC pglGenBuffers = NULL;
GLBINDVERTEXARRAYPROC pglBindVertexArray = NULL;
GLBINDBUFFERPROC pglBindBuffer = NULL;
GLBUFFERDATAPROC pglBufferData = NULL;
GLVERTEXATTRIBPOINTERPROC pglVertexAttribPointer = NULL;
GLENABLEVERTEXATTRIBARRAYPROC pglEnableVertexAttribArray = NULL;
GLDRAWARRAYSPROC pglDrawArrays = NULL;
GLUSEPROGRAMPROC pglUseProgram = NULL;
GLGETUNIFORMLOCATIONPROC pglGetUniformLocation = NULL;
GLUNIFORM4FPROC pglUniform4f = NULL;
GLUNIFORMMATRIX4FVPROC pglUniformMatrix4fv = NULL;
GLUNIFORM1FPROC pglUniform1f = NULL;

static void load_gl_functions()
{
    pglCreateShader = X(GLCREATESHADERPROC, "glCreateShader");

    pglShaderSource = X(GLSHADERSOURCEPROC, "glShaderSource");

    pglCompileShader = X(GLCOMPILESHADERPROC, "glCompileShader");

    pglGetShaderiv = X(GLGETSHADERIVPROC, "glGetShaderiv");

    pglGetShaderInfoLog = X(GLGETSHADERINFOLOGPROC, "glGetShaderInfoLog");

    pglCreateProgram = X(GLCREATEPROGRAMPROC, "glCreateProgram");

    pglAttachShader = X(GLATTACHSHADERPROC, "glAttachShader");

    pglLinkProgram = X(GLLINKPROGRAMPROC, "glLinkProgram");

    pglGetProgramiv = X(GLGETPROGRAMIVPROC, "glGetProgramiv");

    pglGetProgramInfoLog = X(GLGETPROGRAMINFOLOGPROC, "glGetProgramInfoLog");

    pglGenVertexArrays = X(GLGENVERTEXARRAYSPROC, "glGenVertexArrays");

    pglGenBuffers = X(GLGENBUFFERSPROC, "glGenBuffers");

    pglBindVertexArray = X(GLBINDVERTEXARRAYPROC, "glBindVertexArray");

    pglBindBuffer = X(GLBINDBUFFERPROC, "glBindBuffer");

    pglBufferData = X(GLBUFFERDATAPROC, "glBufferData");

    pglVertexAttribPointer = X(GLVERTEXATTRIBPOINTERPROC, "glVertexAttribPointer");

    pglEnableVertexAttribArray = X(GLENABLEVERTEXATTRIBARRAYPROC, "glEnableVertexAttribArray");

    pglDrawArrays = X(GLDRAWARRAYSPROC, "glDrawArrays");

    pglUseProgram = X(GLUSEPROGRAMPROC, "glUseProgram");

    pglGetUniformLocation = X(GLGETUNIFORMLOCATIONPROC, "glGetUniformLocation");

    pglUniform4f = X(GLUNIFORM4FPROC, "glUniform4f");

    pglUniformMatrix4fv = X(GLUNIFORMMATRIX4FVPROC, "glUniformMatrix4fv");

    pglUniform1f = X(GLUNIFORM1FPROC, "glUniform1f");

}
