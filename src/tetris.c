#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>

#include "glfuncs.c"

// includes math.c for now
#include "camera.c"
#include "base.c"

const int window_width = 1920;
const int window_height = 1080;

// 10 x 20 board: relative to block_size
const int block_size = 32;
const int board_width = 320;
const int board_height = 640;

int vert_draw_length = 0;

const char *triangle_vertex_shader_source =
"#version 460 core\n"
"layout(location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"uniform mat4 uMVP;\n"
"out vec4 uColor;\n"
"void main() {\n"
"   gl_Position = uMVP * vec4(aPos, 0.0, 1.0);\n"
"   uColor = aColor;\n"
"}";

const char *triangle_fragment_shader_source =
"#version 460 core\n"
"out vec4 FragColor;\n"
"in vec4 uColor;\n"
"void main() {\n"
"    FragColor = uColor;\n"
"}";

float triangle_vertices[1024];// = {
     // 100.0f,  100.5f, // left
     // 200.5f,  100.5f, // right
     // 150.5f,  50.5f,  // top
// };

static float board_space_x(float x)
{
    return (float)(((float)window_width / 2) - ((float)board_width/2)) + (x*block_size);
}

static float board_space_y(float y)
{
    return (float)(((float)window_height / 2) - ((float)board_height/2)) + (y*block_size);
}

static float *make_quad(
        float *out,
        float x,
        float y,
        float width,
        float height,
        RGBA color
        )
{
    float x0 = x;
    float y0 = y;
    float x1 = x + width;
    float y1 = y + height;

    // triangle 1
    out[0]  = x0;
    out[1]  = y0;
    out[2] = color.r;
    out[3] = color.g;
    out[4] = color.b;
    out[5] = color.a;

    out[6]  = x0;
    out[7]  = y1;
    out[8] = color.r;
    out[9] = color.g;
    out[10] = color.b;
    out[11] = color.a;

    out[12]  = x1;
    out[13]  = y0;
    out[14] = color.r;
    out[15] = color.g;
    out[16] = color.b;
    out[17] = color.a;

    // triangle 2
    out[18]  = x1;
    out[19]  = y0;
    out[20] = color.r;
    out[21] = color.g;
    out[22] = color.b;
    out[23] = color.a;

    out[24]  = x0;
    out[25]  = y1;
    out[26] = color.r;
    out[27] = color.g;
    out[28] = color.b;
    out[29] = color.a;

    out[30] = x1;
    out[31] = y1;
    out[32] = color.r;
    out[33] = color.g;
    out[34] = color.b;
    out[35] = color.a;

    // @cleanup: just for now but it shouldn't be here
    vert_draw_length += 36;

    return out;
}

static float *make_quad_with_border(
        float *out,
        float x,
        float y,
        float width,
        float height,
        float border,
        RGBA border_color,
        RGBA fill_color
        )
{
    float inner_width = width - (border);
    float inner_height = height - (border);

    make_quad(out, x, y, width, height, border_color);

    if (inner_width > 0.0f && inner_height > 0.0f) {
        make_quad(
                out + 36,
                x + border,
                y + border,
                inner_width,
                inner_height,
                fill_color
                );
    }

    return out;
}

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
            window_width, window_height,
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

    // make screen
    // board size 320 x 640 (10x20)
    RGBA board_color = {0,0,0,0};
    float *board = make_quad(
            triangle_vertices,
            (float)(((float)window_width / 2) - ((float)board_width/2)),
            (float)(((float)window_height / 2) - ((float)board_height/2)),
            board_width,
            board_height,
            board_color
            );

    RGBA block_border_color = {0.05f, 0.05f, 0.05f, 1.0f};
    RGBA b1_color = {1.0f, 0.0f, 0.0f, 1.0f};

    // @todo: transform peices to board space, not screenspace
    float *b1a = make_quad_with_border(
            triangle_vertices+vert_draw_length,
            board_space_x(0),
            board_space_y(0),
            32,
            32,
            1.0f,
            block_border_color,
            b1_color
            );
    float *b1b = make_quad_with_border(
            triangle_vertices+vert_draw_length,
            board_space_x(0),
            board_space_y(1),
            32,
            32,
            1.0f,
            block_border_color,
            b1_color
            );
    float *b1c = make_quad_with_border(
            triangle_vertices+vert_draw_length,
            board_space_x(0),
            board_space_y(2),
            32,
            32,
            1.0f,
            block_border_color,
            b1_color
            );
    float *b1d = make_quad_with_border(
            triangle_vertices+vert_draw_length,
            board_space_x(1),
            board_space_y(2),
            32,
            32,
            1.0f,
            block_border_color,
            b1_color
            );

    pglBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_DYNAMIC_DRAW);

    pglVertexAttribPointer(
            0, // location 0 in the shader
            2, // vec2
            GL_FLOAT,
            GL_FALSE,
            6*sizeof(float), // stride
            (void*)0
            );
    pglEnableVertexAttribArray(0);

    pglVertexAttribPointer(
            1, // location 0 in the shader
            4, // vec4
            GL_FLOAT,
            GL_FALSE,
            6*sizeof(float), // stride
            (void*)(sizeof(float)*2)
            );
    pglEnableVertexAttribArray(1);

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

        // GLint colorLoc = pglGetUniformLocation(triangle_program, "uColor");
        // pglUniform4f(colorLoc, 1.0f, 0.2f, 0.2f, 1.0f);

        // get uniform location
        GLint mvpLoc = pglGetUniformLocation(triangle_program, "uMVP");

        Mat4 proj = setup_orthographic_camera((float)window_width, (float)window_height);

        // then assign it a value
        pglUniformMatrix4fv(mvpLoc, 1, GL_FALSE, proj.m);

        pglBindVertexArray(triangle_vao);

        pglDrawArrays(GL_TRIANGLES, 0, vert_draw_length / 6);

        glXSwapBuffers(display, window);
    }

    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
