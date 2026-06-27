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

#include "glfuncs.c"

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
    load_gl_functions();

    // compile trangle shaders and create triangle program
    GLuint triangle_vert_shader = pglCreateShader(GL_VERTEX_SHADER);
    pglShaderSource(
            triangle_vert_shader,
            1,
            &triangle_vertex_shader_source,
            NULL
            );
    pglCompileShader(triangle_vert_shader);

    GLint vertex_compiled;
    pglGetShaderiv(triangle_vert_shader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        pglGetShaderInfoLog(triangle_vert_shader, 1024, &log_length, message);
        // Write the error to a log
        fprintf(stderr, "triangle vertex shader compile error: %s", message);
        exit(1);
    }

    GLuint triangle_frag_shader = pglCreateShader(GL_FRAGMENT_SHADER);
    pglShaderSource(
            triangle_frag_shader,
            1,
            &triangle_fragment_shader_source,
            NULL
            );
    pglCompileShader(triangle_frag_shader);

    GLint fragment_compiled;
    pglGetShaderiv(triangle_frag_shader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        pglGetShaderInfoLog(triangle_frag_shader, 1024, &log_length, message);
        // Write the error to a log
        fprintf(stderr, "triangle frag shader compile error: %s", message);
        exit(1);
    }

    GLuint triangle_program = pglCreateProgram();
    pglAttachShader(triangle_program, triangle_vert_shader);
    pglAttachShader(triangle_program, triangle_frag_shader);
    pglLinkProgram(triangle_program);

    GLint triangle_program_linked;
    pglGetProgramiv(triangle_program, GL_LINK_STATUS, &triangle_program_linked);
    if (triangle_program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        pglGetProgramInfoLog(triangle_program, 1024, &log_length, message);
        // Write the error to a log
        fprintf(stderr, "triangle program link error: %s", message);
        exit(1);
    }

    // - the VBO stores bytes on the GPU
    // - the VAO stores the rules for how to read those bytes
    GLuint triangle_vao, triangle_vbo;

    pglGenVertexArrays(1, &triangle_vao);
    pglGenBuffers(1, &triangle_vbo);
    pglBindVertexArray(triangle_vao);
    pglBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    pglBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

    pglVertexAttribPointer(
            0, // location 0 in the shader
            2, // vec2
            GL_FLOAT,
            GL_FALSE,
            2*sizeof(float), // stride
            (void*)0
            );
    pglEnableVertexAttribArray(0);



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

        pglUseProgram(triangle_program);
        pglBindVertexArray(triangle_vao);
        pglDrawArrays(GL_TRIANGLES, 0, 3);

        glXSwapBuffers(display, window);
    }

    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
