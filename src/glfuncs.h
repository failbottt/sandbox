#include <GL/gl.h>
#include <GL/glx.h>

typedef GLuint (*GLCREATESHADERPROC)(GLenum type);

typedef void (*GLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar **string, GLint *length);

typedef void (*GLCOMPILESHADERPROC)(GLuint shader);

typedef void (*GLGETSHADERIVPROC)(GLuint shader,
        GLenum pname,
        GLint *params
        );
typedef void (*GLGETSHADERINFOLOGPROC)(
        GLuint shader,
        GLsizei maxLength,
        GLsizei *length,
        GLchar *infoLog
        );

typedef GLuint (*GLCREATEPROGRAMPROC)(void);

typedef void (*GLATTACHSHADERPROC)(
        GLuint program,
        GLuint shader
        );

typedef void (*GLLINKPROGRAMPROC)(GLuint program);

typedef void (*GLGETPROGRAMIVPROC)(
        GLuint program,
        GLenum pname,
        GLint *params
        );

typedef void (*GLGETPROGRAMINFOLOGPROC)(
        GLuint program,
        GLsizei maxLength,
        GLsizei *length,
        GLchar *infoLog
        );

typedef void (*GLGENVERTEXARRAYSPROC) (
        GLsizei n,
        GLuint *arrays
        );

typedef void (*GLGENBUFFERSPROC) (
        GLsizei n,
        GLuint * buffers
        );

typedef void (*GLBINDVERTEXARRAYPROC)(GLuint array);

typedef void (*GLBINDBUFFERPROC)(
        GLenum target,
        GLuint buffer
        );

typedef void (*GLBUFFERDATAPROC) (
        GLenum target,
        GLsizeiptr size,
        const GLvoid * data,
        GLenum usage
        );

typedef void (*GLVERTEXATTRIBPOINTERPROC)(
        GLuint index,
        GLint size,
        GLenum type,
        GLboolean normalized,
        GLsizei stride,
        const GLvoid * pointer
        );

typedef void (*GLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);

typedef void (*GLDRAWARRAYSPROC)(
        GLenum mode,
        GLint first,
        GLsizei count
        );

typedef void (*GLUSEPROGRAMPROC)(GLuint program);

typedef GLint (*GLGETUNIFORMLOCATIONPROC)(
        GLuint program,
        const GLchar *name
        );

typedef void (*GLUNIFORM4FPROC)(
        GLint location,
        GLfloat v0,
        GLfloat v1,
        GLfloat v2,
        GLfloat v3
        );

typedef void (*GLUNIFORMMATRIX4FVPROC)(
        GLint location,
        GLsizei count,
        GLboolean transpose,
        const GLfloat *value
        );

typedef void (*GLUNIFORM1FPROC)(GLuint program, GLfloat value);
