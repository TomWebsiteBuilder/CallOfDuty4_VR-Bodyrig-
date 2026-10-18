#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace kisak::vr::ladder
{
// Reserved SP upmove value (CL_CreateCmd keeps native input above -128). Keeps
// ladder intent and speed in the existing serialized/predicted usercmd.
constexpr int CommandMarker = -128;
constexpr float MaxSpeed = 80.0f; // game inches per second
constexpr float UnitsPerMeter = 39.3700787f;
constexpr float MaxPullMeters = 0.45f;
constexpr float DeadzoneMeters = 0.001f;

struct HandInput
{
    bool valid = false;
    bool grip = false;
    bool touchingLadder = false;
    float height = 0.0f; // tracking-space height; never includes player movement
};

struct HandState
{
    bool wasHeld = true;
    bool grabbed = false;
    float low = 0.0f;
    float previous = 0.0f;
    float pulled = 0.0f;
};

struct State
{
    bool active = false;
    int time = 0;
    HandState hands[2];
};

struct Result
{
    int speed = 0;
    bool grabbed[2] = {};
    bool anyHandHeld = false;
};

inline Result Update(State& state, bool active, int time, const HandInput (&input)[2])
{
    Result result;
    const std::int64_t elapsed = static_cast<std::int64_t>(time) - state.time;
    if (!active || !state.active || elapsed < 0 || elapsed > 100)
    {
        state = {};
        state.active = active;
        state.time = time;
        for (int h = 0; h < 2; ++h)
            state.hands[h].wasHeld = input[h].grip || !input[h].valid;
        return result;
    }
    if (elapsed == 0)
    {
        result.anyHandHeld = state.hands[0].grabbed || state.hands[1].grabbed;
        return result;
    }
    state.time = time;

    float pull = 0.0f;
    for (int h = 0; h < 2; ++h)
    {
        auto& hand = state.hands[h];
        const auto& sample = input[h];
        if (!sample.valid || !std::isfinite(sample.height))
        {
            hand = {};
            hand.wasHeld = true;
            continue;
        }
        if (!sample.grip)
            hand.grabbed = false;
        if (sample.grip && !hand.wasHeld && sample.touchingLadder)
        {
            hand.grabbed = true;
            hand.low = hand.previous = sample.height;
            hand.pulled = 0.0f;
            result.grabbed[h] = true;
        }
        hand.wasHeld = sample.grip;
        if (!hand.grabbed)
            continue;
        if (std::fabs(sample.height - hand.previous) > 0.25f)
        {
            hand.grabbed = false; // reject recenter/teleport/tracking spikes
            continue;
        }
        hand.previous = sample.height;
        const float delta = std::clamp(hand.low - sample.height - DeadzoneMeters,
                                      0.0f, (std::max)(0.0f, MaxPullMeters - hand.pulled));
        hand.low -= delta;
        hand.pulled += delta;
        pull = (std::max)(pull, delta); // two hands never double movement
    }
    result.anyHandHeld = state.hands[0].grabbed || state.hands[1].grabbed;
    const float speed = pull * UnitsPerMeter * 1000.0f / static_cast<float>(elapsed);
    result.speed = static_cast<int>(std::lround(std::clamp(speed / MaxSpeed, 0.0f, 1.0f) * 127.0f));
    return result;
}

inline float CommandSpeed(int forward)
{
    return std::clamp(forward, -127, 127) * (MaxSpeed / 127.0f);
}
}
