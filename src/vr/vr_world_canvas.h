#pragma once

#include <openxr/openxr.h>
#include <cmath>

namespace kisak::vr::world_canvas
{
constexpr float Distance = 1.48f;

inline void Rotation(const XrQuaternionf& q, float m[3][3])
{
    const float x = q.x, y = q.y, z = q.z, w = q.w;
    m[0][0] = 1.0f - 2.0f * (y * y + z * z);
    m[0][1] = 2.0f * (x * y - z * w);
    m[0][2] = 2.0f * (x * z + y * w);
    m[1][0] = 2.0f * (x * y + z * w);
    m[1][1] = 1.0f - 2.0f * (x * x + z * z);
    m[1][2] = 2.0f * (y * z - x * w);
    m[2][0] = 2.0f * (x * z - y * w);
    m[2][1] = 2.0f * (y * z + x * w);
    m[2][2] = 1.0f - 2.0f * (x * x + y * y);
}

// Rows of inverse(anchor) * eye, including eye translation in w.
// Both spaces use the runtime convention: +X right, +Y up, -Z forward.
inline void EyeToCanvas(const XrPosef& anchor, const XrPosef& eye, float rows[3][4])
{
    float a[3][3], e[3][3];
    Rotation(anchor.orientation, a);
    Rotation(eye.orientation, e);
    const float delta[3] = {eye.position.x - anchor.position.x,
                            eye.position.y - anchor.position.y,
                            eye.position.z - anchor.position.z};
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
            rows[r][c] = a[0][r] * e[0][c] + a[1][r] * e[1][c] + a[2][r] * e[2][c];
        rows[r][3] = a[0][r] * delta[0] + a[1][r] * delta[1] + a[2][r] * delta[2];
    }
}

inline void FovTangents(const XrFovf& fov, float tangents[4])
{
    tangents[0] = std::tan(fov.angleLeft);
    tangents[1] = std::tan(fov.angleRight);
    tangents[2] = std::tan(fov.angleUp);
    tangents[3] = std::tan(fov.angleDown);
}

// Match the existing screen's initial vertical coverage and 4:3 aspect.
// Evaluate only when latching; leaning must not resize the physical screen.
inline float ScreenScale(const XrFovf& fov)
{
    return Distance * (std::tan(fov.angleUp) - std::tan(fov.angleDown));
}

inline bool PointerUv(const XrPosef& anchor, const XrPosef& pointer,
                      float halfWidth, float halfHeight, float& u, float& v)
{
    float rows[3][4];
    EyeToCanvas(anchor, pointer, rows);
    const float dz = -rows[2][2];
    if (dz >= -0.0001f)
        return false;
    const float t = (-Distance - rows[2][3]) / dz;
    if (t <= 0.0f)
        return false;
    u = 0.5f + (rows[0][3] - t * rows[0][2]) / (2.0f * halfWidth);
    v = 0.5f - (rows[1][3] - t * rows[1][2]) / (2.0f * halfHeight);
    return true;
}
}
