#include <math.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float m[16];
} Mat4;

static Mat4 mat4_identity(void)
{
    Mat4 r = {0};
    r.m[0]  = 1.0f;
    r.m[5]  = 1.0f;
    r.m[10] = 1.0f;
    r.m[15] = 1.0f;
    return r;
}

static Mat4 mat4_translate(float x, float y, float z)
{
    Mat4 r = mat4_identity();
    r.m[12] = x;
    r.m[13] = y;
    r.m[14] = z;
    return r;
}

static Mat4 mat4_perspective(float fovy_radians, float aspect, float near_z, float
        far_z)
{
    float f = 1.0f / tanf(fovy_radians * 0.5f);

    Mat4 r = {0};
    r.m[0]  = f / aspect;
    r.m[5]  = f;
    r.m[10] = (far_z + near_z) / (near_z - far_z);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * far_z * near_z) / (near_z - far_z);
    return r;
}

static Mat4 mat4_mul(Mat4 a, Mat4 b)
{
    Mat4 r = {0};

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            r.m[col * 4 + row] =
                a.m[0 * 4 + row] * b.m[col * 4 + 0] +
                a.m[1 * 4 + row] * b.m[col * 4 + 1] +
                a.m[2 * 4 + row] * b.m[col * 4 + 2] +
                a.m[3 * 4 + row] * b.m[col * 4 + 3];
        }
    }

    return r;
}

static Vec3 normalize(Vec3 v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    Vec3 out = { v.x / len, v.y / len, v.z / len };
    return out;
}

static Vec3 camera_front_from_angles(float yaw, float pitch)
{
    Vec3 front;
    front.x = cosf(yaw) * cosf(pitch);
    front.y = sinf(pitch);
    front.z = sinf(yaw) * cosf(pitch);
    return normalize(front);
}

static Vec3 vec3_add(Vec3 a, Vec3 b)
{
    Vec3 r = { a.x + b.x, a.y + b.y, a.z + b.z };
    return r;
}

static Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    Vec3 r = { a.x - b.x, a.y - b.y, a.z - b.z };
    return r;
}

static Vec3 vec3_scale(Vec3 v, float s)
{
    Vec3 r = { v.x * s, v.y * s, v.z * s };
    return r;
}

static float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    Vec3 r = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return r;
}

static Vec3 vec3_normalize(Vec3 v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    Vec3 r = { v.x / len, v.y / len, v.z / len };
    return r;
}

static Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = vec3_normalize(vec3_sub(center, eye));
    Vec3 s = vec3_normalize(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Mat4 r = mat4_identity();

    r.m[0] = s.x;
    r.m[1] = u.x;
    r.m[2] = -f.x;
    r.m[3] = 0.0f;

    r.m[4] = s.y;
    r.m[5] = u.y;
    r.m[6] = -f.y;
    r.m[7] = 0.0f;

    r.m[8]  = s.z;
    r.m[9]  = u.z;
    r.m[10] = -f.z;
    r.m[11] = 0.0f;

    r.m[12] = -vec3_dot(s, eye);
    r.m[13] = -vec3_dot(u, eye);
    r.m[14] = vec3_dot(f, eye);
    r.m[15] = 1.0f;

    return r;
}
