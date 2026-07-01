#include "math.c"

// yaw: left/right
// pitch: up/down
static void update_yaw_pitch(
        float dx,
        float dy,
        float sensitivity,
        float *yaw,
        float *pitch
        ) {

    *yaw   += dx * sensitivity;
    *pitch -= dy * sensitivity;

    // clamp fps s
    float limit = 89.0f * (3.14159265f / 180.0f);
    if (*pitch > limit)  *pitch = limit;
    if (*pitch < -limit) *pitch = -limit;
}

static Mat4 setup_orthographic_camera(float window_width, float window_height)
{
    float left = 0.0;
    float right = (float)window_width;
    float bottom = (float)window_height;
    float top = 0.0f;

    Mat4 proj = mat4_ortho(left, right, bottom, top, -1.0f, 1.0f);

    return proj;
}
