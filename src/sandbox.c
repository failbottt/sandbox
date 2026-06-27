#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>

#include "glfuncs.c"
#include "math.c"
#include "time.c"
#include "camera.c"

int win_width = 800;
int win_height = 600;

// up and down look angle
float pitch = 0.0f;

// left and right look angle
float yaw = -3.14f / 2.0f;

// where the camera is in world space
float cameraPos;

// direction the camera is looking
float cameraFront;


float mouse_sensitivity = .0002f;
float x_mouse_pos = 0.0;
float y_mouse_pos = 0.0;


const char *triangle_vertex_shader_source =
"#version 460 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"uniform float uAngle;\n"
"uniform mat4 uMVP;\n"
"out vec4 vColor;\n"
"void main() {\n"
"   float c = cos(uAngle);\n"
"   float s = sin(uAngle);\n"
"\n"
"   mat4 rotY = mat4(\n"
"         c, 0.0,   s, 0.0,\n"
"       0.0, 1.0, 0.0, 0.0,\n"
"        -s, 0.0,   c, 0.0,\n"
"       0.0, 0.0, 0.0, 1.0\n"
"       ); \n"
"\n"
"   gl_Position = uMVP * rotY * vec4(aPos, 1.0);\n"
"   vColor = aColor;\n"
"}";

const char *triangle_fragment_shader_source =
"#version 460 core\n"
"out vec4 FragColor;\n"
"in vec4 vColor;\n"
"void main() {\n"
"    FragColor = vColor;\n"
"}";

float cube_vertices[] = {
    // Front face: red
    -0.5f, -0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f,

     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f,

    // Back face: green
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f,
    -0.5f,  0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f,
     0.5f,  0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f,

     0.5f,  0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f,
     0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f,

    // Left face: blue
    -0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, 0.2f, 0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 0.2f, 0.2f, 1.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, 0.2f, 0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, 0.2f, 0.2f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 1.0f, 1.0f,

    // Right face: yellow
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.2f, 1.0f,
    0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.2f, 1.0f,
    0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.2f, 1.0f,

    0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.2f, 1.0f,
    0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.2f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.2f, 1.0f,

    // Top face: magenta
    -0.5f,  0.5f, -0.5f, 1.0f, 0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 1.0f, 1.0f,

     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 0.2f, 1.0f, 1.0f,

    // Bottom face: cyan
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 0.2f, 1.0f, 1.0f, 1.0f,

     0.5f, -0.5f,  0.5f, 0.2f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, 0.2f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 1.0f, 1.0f
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
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | PointerMotionMask;

    Window window = XCreateWindow(
            display,
            root,
            100, 100,
            win_width, win_height,
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
    pglBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    pglVertexAttribPointer(
            0, // location 0 in the shader
            3, // vec3
            GL_FLOAT,
            GL_FALSE,
            7*sizeof(float), // stride
            (void*)0
            );
    pglVertexAttribPointer(
            1, // location 1 in the shader
            4, // vec4
            GL_FLOAT,
            GL_FALSE,
            7*sizeof(float), // stride
            (void*)(3*sizeof(float))
            );
    pglEnableVertexAttribArray(0);
    pglEnableVertexAttribArray(1);

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    double start = now_seconds();

    int running = 1;
    while (running) {
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                running = 0;
            }
            else if (event.type == MotionNotify)
            {
                float last_mouse_x = x_mouse_pos;
                float last_mouse_y = y_mouse_pos;
                float current_mouse_x = (float)event.xmotion.x;
                float current_mouse_y = (float)event.xmotion.y;

                float dx = (float)(current_mouse_x - last_mouse_x);
                float dy = (float)(current_mouse_y - last_mouse_y);

                update_yaw_pitch(dx, dy, mouse_sensitivity, &yaw, &pitch);

                x_mouse_pos = event.xmotion.x;
                y_mouse_pos = event.xmotion.y;

                fprintf(stderr, "x: %f, y: %f\n", x_mouse_pos, y_mouse_pos);
            }
            else if (event.type == ConfigureNotify)
            {
                win_width = event.xconfigure.width;
                win_height = event.xconfigure.height;
                glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
            }
        }

        // TODO: figure out why/when this should be called based on what is
        // being drawn?
        //
        // should this be in the loop at all?
        glEnable(GL_DEPTH_TEST);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pglUseProgram(triangle_program);

        // GLint colorLoc = pglGetUniformLocation(triangle_program, "uColor");
        // pglUniform4f(colorLoc, 1.0f, 0.2f, 0.2f, 1.0f);

        GLuint angleLoc = pglGetUniformLocation(triangle_program, "uAngle");

        double t = (now_seconds() - start);
        float angle = (float)t;

        pglUniform1f(angleLoc, angle);

        GLint mvpLoc = pglGetUniformLocation(triangle_program, "uMVP");

        Vec3 cameraPos = { 0.0f, 0.0f, 3.0f };
        Vec3 cameraUp = { 0.0f, 1.0f, 0.0f };
        Vec3 cameraFront = camera_front_from_angles(yaw, pitch);

        float aspect = (float)win_width / (float)win_height;
        Mat4 model = mat4_translate(0.0f, 0.0f, -2.5f);
        Mat4 view = mat4_look_at(cameraPos, vec3_add(cameraPos, cameraFront), cameraUp);
        Mat4 projection = mat4_perspective(45.0f * 3.1415926f / 180.0f, aspect, 0.1f, 100.0f);
        Mat4 mvp = mat4_mul(projection, mat4_mul(view, model));

        pglUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.m);

        pglBindVertexArray(triangle_vao);

        pglDrawArrays(GL_TRIANGLES, 0, 36);

        glXSwapBuffers(display, window);
    }

    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, context);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
