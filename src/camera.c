
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
