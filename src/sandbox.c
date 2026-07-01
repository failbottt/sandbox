#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xfixes.h>
#include <X11/keysym.h>

#include "glfuncs.c"
#include "math.c"
#include "time.c"
#include "camera.c"
#include "x11.c"

int key_w = 0;
int key_a = 0;
int key_s = 0;
int key_d = 0;
int key_space = 0;

static void update_camera_position(
        Vec3 *cameraPos,
        Vec3 cameraFront,
        Vec3 cameraUp,
        float moveSpeed,
        float dt
        ) {
    Vec3 right = vec3_normalize(vec3_cross(cameraFront, cameraUp));
    float velocity = moveSpeed * dt;

    if (key_w) {
        *cameraPos = vec3_add(*cameraPos, vec3_scale(cameraFront, velocity));
    }
    if (key_s) {
        *cameraPos = vec3_sub(*cameraPos, vec3_scale(cameraFront, velocity));
    }
    if (key_a) {
        *cameraPos = vec3_sub(*cameraPos, vec3_scale(right, velocity));
    }
    if (key_d) {
        *cameraPos = vec3_add(*cameraPos, vec3_scale(right, velocity));
    }

    // if (key_space) {
    //     *cameraPos = vec3_add(*cameraPos, vec3_scale(right, velocity));
    // }
}


int window_width = 1920;
int window_height = 1080;


// up and down look angle
float pitch = 0.0f;

// left and right look angle
float yaw = -3.14f / 2.0f;

// where the camera is in world space
float cameraPos;

// direction the camera is looking
float cameraFront;

MouseLook ml = {0};


const char *triangle_vertex_shader_source =
"#version 460 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"layout(location = 2) in vec3 aNormal;\n"
"\n"
"uniform float uAngle;\n"
"uniform mat4 uMVP;\n"
"\n"
"out vec4 vColor;\n"
"out vec3 vNormal;\n"
"out vec3 vFragPos;\n"
"\n"
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
"   vec4 worldPos = rotY * vec4(aPos, 1.0);\n"
"   gl_Position = uMVP * worldPos;\n"
"   vColor = aColor;\n"
"   vFragPos = worldPos.xyz;\n"
"   vNormal = mat3(rotY) * aNormal;\n"
"}";

const char *triangle_fragment_shader_source =
"#version 460 core\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"in vec4 vColor;\n"
"in vec3 vNormal;\n"
"in vec3 vFragPos;\n"
"\n"
"uniform vec3 uLightPos;\n"
"uniform vec3 uViewPos;\n"
"\n"
"void main() {\n"
"   vec3 baseColor = vColor.rgb;\n"
"\n"
"   vec3 norm = normalize(vNormal);\n"
"   vec3 lightDir = normalize(uLightPos - vFragPos);\n"
"\n"
"   float diff = max(dot(norm, lightDir), 0.0);\n"
"\n"
"   float ambientStrength = 0.2;\n"
"   vec3 ambient = ambientStrength * baseColor;\n"
"\n"
"   vec3 diffuse = diff * baseColor;\n"
"\n"
"   vec3 result = ambient + diffuse;\n"
"   FragColor = vec4(result, vColor.a);\n"
"}";

//- Front face: (0, 0, 1)
//- Back face: (0, 0, -1)
//- Left face: (-1, 0, 0)
//- Right face: (1, 0, 0)
//- Top face: (0, 1, 0)
//- Bottom face: (0, -1, 0)



// verts 3
// color 4
// normal 3
float cube_vertices[] = {
    // Front face: red
    -0.5f, -0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f,

     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 0.2f, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f,

    // Back face: green
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
     0.5f,  0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f, 0.0f, 0.0f, -1.0f,

     0.5f,  0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
     0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f, 0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 0.2f, 1.0f, 0.0f, 0.0f, -1.0f,

    // Left face: blue
    -0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, 0.2f, 0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 0.2f, 0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, 0.2f, 0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 0.2f, 0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

    // Right face: yellow
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 0.0f,

    0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 0.0f,

    // Top face: magenta
    -0.5f,  0.5f, -0.5f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

     0.5f,  0.5f,  0.5f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 0.2f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

    // Bottom face: cyan
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f,  0.5f, 0.2f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,

     0.5f, -0.5f,  0.5f, 0.2f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, 0.2f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.2f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f
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
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | PointerMotionMask;

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
    pglBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    pglVertexAttribPointer(
            0, // location 0 in the shader
            3, // vec3
            GL_FLOAT,
            GL_FALSE,
            10*sizeof(float), // stride
            (void*)0
            );
    pglVertexAttribPointer(
            1, // location 1 in the shader
            4, // vec4
            GL_FLOAT,
            GL_FALSE,
            10*sizeof(float), // stride
            (void*)(3*sizeof(float))
            );
    pglVertexAttribPointer(
            2, // location 2 in the shader
            3, // vec3
            GL_FLOAT,
            GL_FALSE,
            10*sizeof(float), // stride
            (void*)(7*sizeof(float))
            );

    pglEnableVertexAttribArray(0);
    pglEnableVertexAttribArray(1);
    pglEnableVertexAttribArray(2);

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    double start = now_seconds();

    float movement_speed = 10.0f;

    ml.sensitivity = 0.002f;
    ml.yaw = -90.0f;
    ml.pitch = 0.0f;

    init_raw_mouse(display, window, &ml);
    enter_mouse_look(&ml);

    float last_time = now_seconds();

    int running = 1;

    // camera
    Vec3 cameraPos = { 0.0f, 0.0f, 3.0f };
    Vec3 cameraUp = { 0.0f, 1.0f, 0.0f };

    while (running)
    {
        while (XPending(display))
        {
            XEvent event;
            XNextEvent(display, &event);

            if  (
                    event.type == GenericEvent &&
                    event.xcookie.extension == ml.xi_opcode &&
                    XGetEventData(display, &event.xcookie)
                )
            {
                if (event.xcookie.evtype == XI_RawMotion)
                {
                    XIRawEvent *raw = (XIRawEvent *)event.xcookie.data;
                    if (raw->valuators.mask_len == 0) {
                        XFreeEventData(display, &event.xcookie);
                        break;
                    }

                    double deltaX = 0.0f;
                    double deltaY = 0.0f;

                    /* check if relative motion data exists where we think it does */
                    if (XIMaskIsSet(raw->valuators.mask, 0) != 0)
                        deltaX += raw->raw_values[0];
                    if (XIMaskIsSet(raw->valuators.mask, 1) != 0)
                        deltaY += raw->raw_values[1];

                    update_yaw_pitch(deltaX, deltaY, ml.sensitivity, &ml.yaw, &ml.pitch);

                    //The mouse must be moved back to the center when it moves
                    XWarpPointer(display, None, window, 0, 0, 0, 0, window_width / 2, window_height / 2);
                }

                XFreeEventData(display, &event.xcookie);
            }

            if (event.type == KeyPress)
            {
                KeySym sym = XLookupKeysym(&event.xkey, 0);
                if (sym == XK_w) key_w = 1;
                if (sym == XK_a) key_a = 1;
                if (sym == XK_s) key_s = 1;
                if (sym == XK_d) key_d = 1;
                if (sym == XK_KP_Space) key_space = 1;

                if (sym == XK_Escape) running = 0;
            }
            if (event.type == KeyRelease)
            {
                KeySym sym = XLookupKeysym(&event.xkey, 0);
                if (sym == XK_w) key_w = 0;
                if (sym == XK_a) key_a = 0;
                if (sym == XK_s) key_s = 0;
                if (sym == XK_d) key_d = 0;
                if (sym == XK_KP_Space) key_space = 0;

            }
            else if (event.type == MotionNotify)
            {
                // fprintf(stderr, "MotionNotify\n");
            }
            else if (event.type == ConfigureNotify)
            {
                window_width = event.xconfigure.width;
                window_height = event.xconfigure.height;
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


        GLuint lightingLoc = pglGetUniformLocation(triangle_program, "uLightPos");

        pglUniform3f(lightingLoc, -1.0f, 0.5f, 1.0f);

        GLuint angleLoc = pglGetUniformLocation(triangle_program, "uAngle");

        double t = (now_seconds() - start);
        float angle = (float)t;

        pglUniform1f(angleLoc, angle);

        GLint mvpLoc = pglGetUniformLocation(triangle_program, "uMVP");

        Vec3 cameraFront = camera_front_from_angles(ml.yaw, ml.pitch);

        double now = now_seconds();
        float dt = (float)(now - last_time);
        last_time = now;

        update_camera_position(
                &cameraPos,
                cameraFront,
                cameraUp,
                3.0f,
                dt
                );
        // if (key_a)
        // if (key_s)
        // if (key_d)

        float aspect = (float)window_width / (float)window_height;
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
