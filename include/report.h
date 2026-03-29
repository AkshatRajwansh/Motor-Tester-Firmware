#pragma once
#include <Arduino.h>
#include <algorithm>
#include "test_results.h"

class ReportPrinter {
public:

    static void printHeader(const char* title) {
        Serial.println();
        Serial.println("══════════════════════════════════════════════");
        Serial.println(title);
        Serial.println("══════════════════════════════════════════════");
    }

    static void printSweepData(TestResults& r) {
        printHeader("STEP 1: SWEEP TEST — Angle vs Time vs Current");
        Serial.println("  t(ms)  | Target(°) | Actual(°) | Current(mA)");
        Serial.println("  -------|-----------|-----------|------------");
        for (uint16_t i = 0; i < r.sweepCount; i++) {
            SweepPoint& p = r.sweepData[i];
            Serial.printf("  %6lu | %9.2f | %9.2f | %10.2f\n",
                          (unsigned long)p.timestampMs, p.targetDeg,
                          p.actualDeg, p.currentMA);
        }
    }

    static void printPositionAccuracy(TestResults& r) {
        printHeader("STEP 2: POSITION ACCURACY");
        Serial.println("  Target(°) | Actual(°) | Error(°)");
        Serial.println("  ----------|-----------|----------");
        for (uint8_t i = 0; i < r.posCount; i++) {
            PositionResult& p = r.posResults[i];
            Serial.printf("  %9.2f | %9.2f | %+8.3f\n",
                          p.targetDeg, p.actualDeg, p.errorDeg);
        }
        Serial.printf("  Max absolute error: %.3f°\n",
                      computeMaxAbsError(r.posResults, r.posCount));
    }

    static void printRepeatability(TestResults& r) {
        printHeader("STEP 3: REPEATABILITY (target 90°)");
        for (uint8_t i = 0; i < r.repeat.count; i++) {
            Serial.printf("  Run %d: %.3f°\n", i + 1, r.repeat.positions[i]);
        }
        Serial.printf("  Mean:   %.3f°\n", r.repeat.mean);
        Serial.printf("  StdDev: %.4f°\n", r.repeat.stdDev);
        Serial.printf("  Range:  %.4f°\n",
                      *std::max_element(r.repeat.positions, r.repeat.positions + r.repeat.count) -
                      *std::min_element(r.repeat.positions, r.repeat.positions + r.repeat.count));
    }

    static void printSpeedResponse(TestResults& r) {
        printHeader("STEP 4: SPEED & RESPONSE (0° → 90°)");
        Serial.printf("  Rise time (to 90°): %lu ms\n",
                      (unsigned long)r.speed.time0to90Ms);
        Serial.printf("  Overshoot:          %.2f°\n", r.speed.overshootDeg);
        Serial.printf("  Settling time:      %lu ms\n",
                      (unsigned long)r.speed.settleTimeMs);
        Serial.printf("  Peak current:       %.1f mA\n", r.speed.peakCurrentMA);
    }

    static void printPhase1Summary(TestResults& r) {
        printHeader("═══ PHASE 1 COMPLETE — SUMMARY ═══");
        Serial.printf("  Position max error: %.3f°\n",
                      computeMaxAbsError(r.posResults, r.posCount));
        Serial.printf("  Repeatability σ:    %.4f°\n", r.repeat.stdDev);
        Serial.printf("  0→90 rise time:     %lu ms\n",
                      (unsigned long)r.speed.time0to90Ms);
        Serial.printf("  Overshoot:          %.2f°\n", r.speed.overshootDeg);
        Serial.printf("  Peak sweep current: %.1f mA\n", r.speed.peakCurrentMA);
    }

    static void printContactDetect(TestResults& r) {
        printHeader("STEP 6: CONTACT DETECTION");
        Serial.printf("  Contact angle:  %.2f°\n", r.contact.contactAngleDeg);
        Serial.printf("  Contact force:  %.1f g\n", r.contact.contactForceg);
    }

    static void printTorqueData(TestResults& r) {
        printHeader("STEP 7 & 8: TORQUE vs ANGLE vs CURRENT");
        Serial.println("  Angle(°) | Force(g)  | Torque(N·mm) | Current(mA)");
        Serial.println("  ---------|-----------|--------------|------------");
        for (uint8_t i = 0; i < r.torqueCount; i++) {
            TorquePoint& p = r.torqueData[i];
            Serial.printf("  %8.2f | %9.2f | %12.3f | %10.2f\n",
                          p.angleDeg, p.forceg, p.torqueNmm, p.currentMA);
        }
    }

    static void printStall(TestResults& r) {
        printHeader("STEP 9: STALL TEST");
        Serial.printf("  Stall angle:   %.2f°\n",   r.stall.stallAngleDeg);
        Serial.printf("  Stall torque:  %.3f N·mm\n", r.stall.stallTorqueNmm);
        Serial.printf("  Stall current: %.1f mA\n",   r.stall.stallCurrentMA);
    }

    static void printHold(TestResults& r) {
        printHeader("STEP 10: LOADED HOLDING TEST");
        Serial.printf("  Hold angle:    %.2f°\n", r.hold.startAngleDeg);
        Serial.printf("  End angle:     %.2f°\n", r.hold.endAngleDeg);
        Serial.printf("  Drift:         %.4f°\n", r.hold.driftDeg);
        Serial.printf("  Avg current:   %.1f mA\n", r.hold.avgCurrentMA);
        Serial.printf("  Min current:   %.1f mA\n", r.hold.minCurrentMA);
        Serial.printf("  Max current:   %.1f mA\n", r.hold.maxCurrentMA);
    }

    static void printPhase2Summary(TestResults& r) {
        printHeader("═══ PHASE 2 COMPLETE — SUMMARY ═══");
        Serial.printf("  Contact angle:  %.2f°\n",     r.contact.contactAngleDeg);
        Serial.printf("  Stall torque:   %.3f N·mm\n",  r.stall.stallTorqueNmm);
        Serial.printf("  Stall current:  %.1f mA\n",    r.stall.stallCurrentMA);
        Serial.printf("  Hold drift:     %.4f°\n",      r.hold.driftDeg);
        Serial.println();
        Serial.println("  === END OF TEST REPORT ===");
    }
};
