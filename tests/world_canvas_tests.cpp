#include "vr/vr_world_canvas.h"
#include <array>
#include <cstdio>
#include <cstdlib>

namespace Canvas = kisak::vr::world_canvas;
using Vec = std::array<float, 3>;
static int checks = 0;

static void Check(bool ok, const char* label)
{
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", label); std::exit(1); }
}

static void Near(float actual, float expected, const char* label)
{
    Check(std::fabs(actual - expected) < 0.0001f, label);
}

static XrQuaternionf AxisAngle(int axis, float angle)
{
    XrQuaternionf q = {0, 0, 0, std::cos(angle / 2)};
    if (axis == 0) q.x = std::sin(angle / 2);
    if (axis == 1) q.y = std::sin(angle / 2);
    if (axis == 2) q.z = std::sin(angle / 2);
    return q;
}

// Independent quaternion-vector reference, without the production matrices.
static Vec Rotate(XrQuaternionf q, Vec v)
{
    const Vec t = {2 * (q.y * v[2] - q.z * v[1]),
                   2 * (q.z * v[0] - q.x * v[2]),
                   2 * (q.x * v[1] - q.y * v[0])};
    return {v[0] + q.w * t[0] + q.y * t[2] - q.z * t[1],
            v[1] + q.w * t[1] + q.z * t[0] - q.x * t[2],
            v[2] + q.w * t[2] + q.x * t[1] - q.y * t[0]};
}

static void CheckWorldPoint(const XrPosef& anchor, const XrPosef& eye,
                            const XrFovf& fov, Vec point)
{
    Vec world = Rotate(anchor.orientation, point);
    world[0] += anchor.position.x - eye.position.x;
    world[1] += anchor.position.y - eye.position.y;
    world[2] += anchor.position.z - eye.position.z;
    const auto q = eye.orientation;
    const Vec local = Rotate({-q.x, -q.y, -q.z, q.w}, world);
    Check(local[2] < 0, "fixture point is in front of eye");

    // Project a fixed physical point into the runtime's asymmetric viewport.
    const float u = (local[0] / -local[2] - std::tan(fov.angleLeft)) /
                    (std::tan(fov.angleRight) - std::tan(fov.angleLeft));
    const float v = (std::tan(fov.angleUp) - local[1] / -local[2]) /
                    (std::tan(fov.angleUp) - std::tan(fov.angleDown));
    float tangents[4], rows[3][4];
    Canvas::FovTangents(fov, tangents);
    Canvas::EyeToCanvas(anchor, eye, rows);
    const Vec ray = {tangents[0] + u * (tangents[1] - tangents[0]),
                     tangents[2] + v * (tangents[3] - tangents[2]), -1};
    Vec direction;
    for (int r = 0; r < 3; ++r)
        direction[r] = rows[r][0] * ray[0] + rows[r][1] * ray[1] + rows[r][2] * ray[2];
    const float t = (-Canvas::Distance - rows[2][3]) / direction[2];
    // Every view of that point must sample the same spot on the screen.
    Near(rows[0][3] + t * direction[0], point[0], "world screen x remains fixed");
    Near(rows[1][3] + t * direction[1], point[1], "world screen y remains fixed");

    float live[3][3];
    Canvas::Rotation(eye.orientation, live);
    for (int r = 0; r < 3; ++r)
        Near(live[r][0] * ray[0] + live[r][1] * ray[1] + live[r][2] * ray[2],
             world[r] / -local[2], "smoke ray points into the same world direction");
}

int main()
{
    const XrFovf fovs[] = {{-0.9f, 0.65f, 0.8f, -0.7f}, {-0.65f, 0.9f, 0.8f, -0.7f}};
    for (int anchorAxis = 0; anchorAxis < 3; ++anchorAxis)
    {
        const XrPosef anchor = {AxisAngle(anchorAxis, 0.4f), {2, 1.6f, -3}};
        for (int moveAxis = 0; moveAxis < 3; ++moveAxis)
        for (float angle : {-0.35f, 0.0f, 0.35f})
        for (int eyeIndex = 0; eyeIndex < 2; ++eyeIndex)
        {
            XrPosef eye = {AxisAngle(moveAxis, angle),
                          {2.15f + (eyeIndex == 0 ? -0.032f : 0.032f), 1.72f, -2.8f}};
            for (float x : {-0.3f, 0.0f, 0.3f})
            for (float y : {-0.2f, 0.0f, 0.2f})
                CheckWorldPoint(anchor, eye, fovs[eyeIndex], {x, y, -Canvas::Distance});
        }
    }

    const XrPosef identity = {{0, 0, 0, 1}, {0, 0, 0}};
    float u, v;
    Check(Canvas::PointerUv(identity, identity, 0.6f, 0.45f, u, v), "center pointer hits");
    Near(u, 0.5f, "center pointer u"); Near(v, 0.5f, "center pointer v");
    XrPosef moved = identity;
    moved.position = {0.3f, 0.225f, 0.4f};
    Check(Canvas::PointerUv(identity, moved, 0.6f, 0.45f, u, v), "translated pointer hits");
    Near(u, 0.75f, "translated pointer u"); Near(v, 0.25f, "translated pointer v");
    for (float turn : {1.5707963f, 3.1415927f, -1.5707963f})
    {
        moved = {AxisAngle(1, turn), {0, 0, 0}};
        Check(!Canvas::PointerUv(identity, moved, 0.6f, 0.45f, u, v), "screen absent when looking sideways/behind");
        Check(Canvas::PointerUv(moved, moved, 0.6f, 0.45f, u, v), "recenter puts screen in front");
        Near(u, 0.5f, "recenter u"); Near(v, 0.5f, "recenter v");
    }
    moved = identity; moved.position.z = -2;
    Check(!Canvas::PointerUv(identity, moved, 0.6f, 0.45f, u, v), "screen behind eye rejected");
    const float scale = Canvas::ScreenScale(fovs[0]);
    Near(2 * 0.27f * scale / (Canvas::Distance *
        (std::tan(fovs[0].angleUp) - std::tan(fovs[0].angleDown))), 0.54f,
        "preserve initial menu vertical coverage");
    std::printf("%d world canvas checks passed\n", checks);
}
