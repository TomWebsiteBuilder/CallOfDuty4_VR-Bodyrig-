#include "vr/vr_ladder_climb.h"
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace Ladder = kisak::vr::ladder;
static int checks = 0;
static void Check(bool ok, const char* message)
{
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

struct Fixture
{
    Ladder::State state;
    Ladder::HandInput hands[2] = {{true, false, true, 1.8f}, {true, false, true, 1.8f}};
    int time = 0;
    Fixture() { Ladder::Update(state, true, 0, hands); }
    Ladder::Result Step(int milliseconds = 20, bool active = true)
    {
        time += milliseconds;
        return Ladder::Update(state, active, time, hands);
    }
    void Grab(int hand)
    {
        hands[hand].grip = true;
        const auto result = Step();
        Check(result.grabbed[hand] && result.speed == 0, "new grip anchors without teleporting");
    }
};

int main()
{
    Fixture f;
    f.Grab(0);
    Check(f.Step(0).anyHandHeld, "holding grip reports anyHandHeld true");
    f.hands[0].height -= 0.02f;
    Check(f.Step().speed > 0, "downward pull climbs");
    for (int i = 0; i < 50; ++i) Check(f.Step().speed == 0, "holding still does not climb");
    f.hands[0].height += 0.02f;
    Check(f.Step().speed == 0, "lifting held hand does not climb");
    f.hands[0].height -= 0.02f;
    Check(f.Step().speed == 0, "oscillating above low watermark does not farm height");
    f.hands[0].grip = false;
    f.hands[0].height -= 0.02f;
    Check(!f.Step().anyHandHeld, "releasing grip reports anyHandHeld false");
    Check(f.Step().speed == 0, "release stops movement immediately");
    f.hands[0].height = 1.8f;
    f.Step(); f.Grab(0);
    f.hands[0].height -= 0.02f;
    Check(f.Step().speed > 0, "re-grab allows next pull");
    f.Grab(1);
    f.hands[0].grip = false;
    f.hands[1].height -= 0.02f;
    Check(f.Step().speed > 0, "alternating hands continues climbing");

    Fixture single, both;
    single.Grab(0); both.Grab(0); both.Grab(1);
    single.hands[0].height -= 0.02f;
    both.hands[0].height -= 0.02f; both.hands[1].height -= 0.02f;
    Check(single.Step().speed == both.Step().speed, "two hands do not double the speed");

    Fixture miss;
    miss.hands[0].touchingLadder = false;
    miss.hands[0].grip = true;
    Check(!miss.Step().grabbed[0], "grabbing empty air is ignored");
    miss.hands[0].touchingLadder = true;
    miss.hands[0].height -= 0.04f;
    Check(miss.Step().speed == 0, "holding grip before contact does not auto-grab");

    Fixture loss;
    loss.Grab(0);
    loss.hands[0].valid = false;
    Check(loss.Step().speed == 0, "tracking loss stops pull");
    loss.hands[0].valid = true; loss.hands[0].height -= 0.1f;
    Check(loss.Step().speed == 0, "tracking recovery requires fresh grip");
    loss.hands[0].grip = false; loss.Step(); loss.Grab(0);
    loss.hands[0].height -= 0.6f;
    Check(loss.Step().speed == 0, "large tracking jump rejected");
    loss.hands[0].height = std::numeric_limits<float>::quiet_NaN();
    Check(loss.Step().speed == 0, "NaN tracking rejected");

    Fixture paused;
    paused.Grab(0);
    paused.Step(20, false);
    paused.hands[0].height -= 0.1f;
    Check(paused.Step().speed == 0, "resume/map entry does not reuse held grip");
    paused.hands[0].grip = false; paused.Step(); paused.Grab(0);
    paused.hands[0].height -= 0.1f;
    Check(paused.Step(500).speed == 0, "long frame gap resets pull");
    Check(Ladder::Update(paused.state, true, 1, paused.hands).speed == 0, "map clock rollback resets pull");

    Fixture capped;
    capped.Grab(0);
    for (int i = 0; i < 60; ++i)
    {
        capped.hands[0].height -= 0.01f;
        const auto result = capped.Step();
        Check(result.speed >= 0 && result.speed <= 127, "speed stays encodable");
        if (i > 46) Check(result.speed == 0, "each grab has finite reach");
    }
    Check(capped.state.hands[0].pulled <= Ladder::MaxPullMeters + 0.00001f, "pull distance bounded");

    for (int milliseconds : {10, 20, 50})
    {
        Fixture timed;
        timed.Grab(0);
        float distance = 0;
        for (int elapsed = 0; elapsed < 500; elapsed += milliseconds)
        {
            timed.hands[0].height -= 0.4f * milliseconds / 1000.0f;
            const auto result = timed.Step(milliseconds);
            distance += Ladder::CommandSpeed(result.speed) * milliseconds / 1000.0f;
            Check(timed.Step(0).speed == 0, "duplicate command time does not replay pull");
        }
        Check(std::fabs(distance - (0.2f - Ladder::DeadzoneMeters) * Ladder::UnitsPerMeter) < 0.2f,
              "same physical pull travels same distance across frame rates");
    }
    Check(Ladder::CommandSpeed(0) == 0 && Ladder::CommandSpeed(127) == Ladder::MaxSpeed,
          "prediction decodes idle and full speed");
    Check(Ladder::CommandSpeed(-127) == -Ladder::MaxSpeed, "backward command descends");
    Check(static_cast<signed char>(Ladder::CommandMarker) == -128, "marker survives signed byte transport");
    std::printf("%d ladder climbing checks passed\n", checks);
}
