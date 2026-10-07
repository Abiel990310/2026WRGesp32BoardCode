#pragma once
// Board: TUMi / RLISP602ER, current robot
// Each row is one labelled board port: M1, M2, M3, M4.
// Fields: GPIO A, GPIO B, PWM channel A, PWM channel B, inverted.
// taiwanboard directions match the current tested wiring.
// chinaver uses the same chassis layout as a starting assumption; verify wiring.
Motor motors[4] = {
    {23, 25, 8, 9, true},  // M1
    {26, 27, 10, 11, true},  // M2
    {32, 33, 12, 13, false},  // M3
    {14, 13, 14, 15, false},  // M4
};

// Default chassis layout: M1 top-left, M2 bottom-left,
// M3 bottom-right, M4 top-right. Edit these if port positions change.
constexpr bool kMotorOnLeft[4] = {true, true, false, false};
constexpr int kCompassSda = 21;
constexpr int kCompassScl = 22;
constexpr const char* kSketchName = "taiwanboard_full";
