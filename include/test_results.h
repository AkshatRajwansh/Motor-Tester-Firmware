#pragma once
#include <Arduino.h>

// ── Phase 1 results ──────────────────────────────────────────
struct SweepPoint {
    float targetDeg;
    float actualDeg;
    float currentMA;
    uint32_t timestampMs;
};

struct PositionResult {
    float targetDeg;
    float actualDeg;
    float errorDeg;
};

struct RepeatResult {
    float positions[12];  // up to 12 runs
    uint8_t count;
    float mean;
    float stdDev;
};

struct SpeedResult {
    uint32_t time0to90Ms;
    float    overshootDeg;
    uint32_t settleTimeMs;
    float    peakCurrentMA;
};

// ── Phase 2 results ──────────────────────────────────────────
struct ContactResult {
    float contactAngleDeg;
    float contactForceg;
};

struct TorquePoint {
    float angleDeg;
    float forceg;
    float torqueNmm;
    float currentMA;
};

struct StallResult {
    float stallAngleDeg;
    float stallTorqueNmm;
    float stallCurrentMA;
};

struct HoldResult {
    float startAngleDeg;
    float endAngleDeg;
    float driftDeg;
    float avgCurrentMA;
    float minCurrentMA;
    float maxCurrentMA;
};

// ── Master result bundle ──────────────────────────────────────
struct TestResults {
    // Phase 1
    SweepPoint   sweepData[200];
    uint16_t     sweepCount;

    PositionResult posResults[5];
    uint8_t        posCount;

    RepeatResult  repeat;
    SpeedResult   speed;

    // Phase 2
    ContactResult contact;
    TorquePoint   torqueData[50];
    uint8_t       torqueCount;
    StallResult   stall;
    HoldResult    hold;
};

// Compute stats helpers
inline float computeMean(float* arr, uint8_t n) {
    float sum = 0;
    for (uint8_t i = 0; i < n; i++) sum += arr[i];
    return sum / n;
}

inline float computeStdDev(float* arr, uint8_t n, float mean) {
    float var = 0;
    for (uint8_t i = 0; i < n; i++) {
        float d = arr[i] - mean;
        var += d * d;
    }
    return sqrt(var / n);
}

inline float computeMaxAbsError(PositionResult* arr, uint8_t n) {
    float mx = 0;
    for (uint8_t i = 0; i < n; i++) {
        float e = abs(arr[i].errorDeg);
        if (e > mx) mx = e;
    }
    return mx;
}
