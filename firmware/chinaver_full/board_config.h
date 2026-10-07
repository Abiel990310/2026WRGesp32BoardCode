#pragma once
// Board: emakefun Maker ESP32, vendor motorTest pin map
// Each row is one labelled board port: M1, M2, M3, M4.
// Fields: GPIO A, GPIO B, PWM channel A, PWM channel B, inverted.
// taiwanboard directions match the current tested wiring.
// China polarity: M1/M2/M3 reversed from the previous fleet build per wheel test.
Motor motors[4] = {
    {27, 13, 8, 9, false},  // M1
    {4, 2, 10, 11, false},  // M2
    {17, 12, 12, 13, true},  // M3
    {14, 15, 14, 15, false},  // M4
};

// Default chassis layout: M1 top-left, M2 bottom-left,
// M3 bottom-right, M4 top-right. Edit these if port positions change.
constexpr bool kMotorOnLeft[4] = {true, true, false, false};
constexpr int kCompassSda = 21;
constexpr int kCompassScl = 22;
constexpr const char* kSketchName = "chinaver_full";
