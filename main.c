// this is a minimal - at least minimal to me at this point - example
// of how to:
//
// 1. open an x11 window
// 2. create and add an opengl context to that window
// 3. draw a triangle
//
// build command: gcc -g main.c -o exe -lX11 -lGL

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>

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

const char *triangle_vertex_shader_source =
"#version 460 core\n"
"layout(location = 0) in vec2 aPos;\n"
"void main() {\n"
"   gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}";

const char *triangle_fragment_shader_source =
"#version 460 core\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = vec4(1.0, 0.6, 0.2, 1.0);\n"
"}";

float triangle_vertices[] = {
     0.0f,  0.5f,
    -0.5f, -0.5f,
     0.5f, -0.5f
};

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    /* Pick a framebuffer config that supports RGBA and double buffering */
    int fbAttribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8,
        GLX_BLUE_SIZE,     8,
        GLX_ALPHA_SIZE,    8,
        GLX_DEPTH_SIZE,    24,
        GLX_DOUBLEBUFFER,  True,
        None
    };

    int fbCount = 0;
    GLXFBConfig *fbConfigs = glXChooseFBConfig(display, screen, fbAttribs,
            &fbCount);
    if (!fbConfigs || fbCount == 0) {
        fprintf(stderr, "No suitable GLXFBConfig found\n");
        XCloseDisplay(display);
        return 1;
    }

    GLXFBConfig fbConfig = fbConfigs[0];
    XFree(fbConfigs);

    XVisualInfo *visual = glXGetVisualFromFBConfig(display, fbConfig);
    if (!visual) {
        fprintf(stderr, "Failed to get XVisualInfo\n");
        XCloseDisplay(display);
        return 1;
    }

    Colormap colormap = XCreateColormap(display, root, visual->visual, AllocNone);

    XSetWindowAttributes swa;
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    Window window = XCreateWindow(
            display,
            root,
            100, 100,
            800, 600,
            0,
            visual->depth,
            InputOutput,
            visual->visual,
            CWColormap | CWEventMask,
            &swa
            );

    XFree(visual);

    XStoreName(display, window, "X11 OpenGL Window");
    XMapWindow(display, window);

    /* Request a modern OpenGL context */
    typedef GLXContext (*glXCreateContextAttribsARBProc)(
            Display*, GLXFBConfig, GLXContext, Bool, const int*
            );

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
        (glXCreateContextAttribsARBProc)glXGetProcAddressARB(
                (const GLubyte *)"glXCreateContextAttribsARB"
                );

    if (!glXCreateContextAttribsARB) {
        fprintf(stderr, "glXCreateContextAttribsARB not available\n");
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        return 1;
    }

    int contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    GLXContext context = glXCreateContextAttribsARB(
            display, fbConfig, NULL, True, contextAttribs
            );

    if (!context) {
        fprintf(stderr, "Failed to create GLX context\n");
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        return 1;
    }

    if (!glXMakeCurrent(display, window, context)) {
        fprintf(stderr, "glXMakeCurrent failed\n");
        glXDestroyContext(display, context);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        return 1;
    }

    // load gl functions
    GLCREATESHADERPROC glCreateShader = (GLCREATESHADERPROC)glXGetProcAddressARB((const GLubyte*)"glCreateShader");
    GLSHADERSOURCEPROC glShaderSource = (GLSHADERSOURCEPROC)glXGetProcAddressARB((const GLubyte*)"glShaderSource");
    GLCOMPILESHADERPROC glCompileShader = (GLCOMPILESHADERPROC)glXGetProcAddressARB((const GLubyte*)"glCompileShader");
    GLGETSHADERIVPROC glGetShaderiv = (GLGETSHADERIVPROC)glXGetProcAddressARB((const GLubyte*)"glGetShaderiv");
    GLGETSHADERINFOLOGPROC glGetShaderInfoLog = (GLGETSHADERINFOLOGPROC)glXGetProcAddressARB((const GLubyte*)"glGetShaderInfoLog");
    GLCREATEPROGRAMPROC glCreateProgram = (GLCREATEPROGRAMPROC)glXGetProcAddressARB((const GLubyte*)"glCreateProgram");
    GLATTACHSHADERPROC glAttachShader = (GLATTACHSHADERPROC)glXGetProcAddressARB((const GLubyte*)"glAttachShader");
    GLLINKPROGRAMPROC glLinkProgram = (GLLINKPROGRAMPROC)glXGetProcAddressARB((const GLubyte*)"glLinkProgram");
    GLGETPROGRAMIVPROC glGetProgramiv = (GLGETPROGRAMIVPROC)glXGetProcAddressARB((const GLubyte*)"glGetProgramiv");
    GLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = (GLGETPROGRAMINFOLOGPROC)glXGetProcAddressARB((const GLubyte*)"glGetProgramInfoLog");

    GLGENVERTEXARRAYSPROC glGenVertexArrays = (GLGENVERTEXARRAYSPROC)glXGetProcAddressARB((const GLubyte*)"glGenVertexArrays");

    GLGENBUFFERSPROC glGenBuffers = (GLGENBUFFERSPROC)glXGetProcAddressARB((const GLubyte*)"glGenBuffers");

    GLBINDVERTEXARRAYPROC glBindVertexArray = (GLBINDVERTEXARRAYPROC)glXGetProcAddressARB((const GLubyte*)"glBindVertexArray");

    GLBINDBUFFERPROC glBindBuffer = (GLBINDBUFFERPROC)glXGetProcAddressARB((const GLubyte*)"glBindBuffer");

    GLBUFFERDATAPROC glBufferData = (GLBUFFERDATAPROC)glXGetProcAddressARB((const GLubyte*)"glBufferData");

    GLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = (GLVERTEXATTRIBPOINTERPROC)glXGetProcAddressARB((const GLubyte*)"glVertexAttribPointer");

    GLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = (GLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddressARB((const GLubyte*)"glEnableVertexAttribArray");

    GLDRAWARRAYSPROC glDrawArrays = (GLDRAWARRAYSPROC)glXGetProcAddressARB((const GLubyte*)"glDrawArrays");

    GLUSEPROGRAMPROC glUseProgram = (GLUSEPROGRAMPROC)glXGetProcAddressARB((const GLubyte*)"glUseProgram");

    // compile trangle shaders and create triangle program
    GLuint triangle_vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(
            triangle_vert_shader,
            1,
            &triangle_vertex_shader_source,
            NULL
            );
    glCompileShader(triangle_vert_shader);

    GLint vertex_compiled;
    glGetShaderiv(triangle_vert_shader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(triangle_vert_shader, 1024, &log_length, message);
        // Write the error to a log
        fprintf(stderr, "triangle vertex shader compile error: %s", message);
        exit(1);
    }

    GLuint triangle_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(
            triangle_frag_shader,
            1,
            &triangle_fragment_shader_source,
            NULL
            );
    glCompileShader(triangle_frag_shader);

    GLint fragment_compiled;
    glGetShaderiv(triangle_frag_shader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(triangle_frag_shader, 1024, &log_length, message);
        // Write the error to a log
        fprintf(stderr, "triangle frag shader compile error: %s", message);
        exit(1);
    }

    GLuint triangle_program = glCreateProgram();
    glAttachShader(triangle_program, triangle_vert_shader);
    glAttachShader(triangle_program, triangle_frag_shader);
    glLinkProgram(triangle_program);

    GLint triangle_program_linked;
    glGetProgramiv(triangle_program, GL_LINK_STATUS, &triangle_program_linked);
    if (triangle_program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(triangle_program, 1024, &log_length, message);
        // Write the error to a log
        fprintf(stderr, "triangle program link error: %s", message);
        exit(1);
    }

    // - the VBO stores bytes on the GPU
    // - the VAO stores the rules for how to read those bytes
    GLuint triangle_vao, triangle_vbo;

    glGenVertexArrays(1, &triangle_vao);
    glGenBuffers(1, &triangle_vbo);
    glBindVertexArray(triangle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(
            0, // location 0 in the shader
            2, // vec2
            GL_FLOAT,
            GL_FALSE,
            2*sizeof(float), // stride
            (void*)0
            );
    glEnableVertexAttribArray(0);



    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    int running = 1;
    while (running) {
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                running = 0;
            } else if (event.type == ConfigureNotify) {
                glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(triangle_program);
        glBindVertexArray(triangle_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glXSwapBuffers(display, window);
    }

    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
