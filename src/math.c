#include <math.h>

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

