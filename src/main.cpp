// ================================================================
//  Servo Motor Test JIG — Main Application
//  Platform:  Teensy 4.1
//  Framework: Arduino / PlatformIO
//
//  Hardware connections (see config.h for pin numbers):
//    Wire  (18/19) → OLED SSD1306 + AS5600 encoder
//    Wire2 (24/25) → INA219 current sensor
//    HX711         → DAT pin 3 / CLK pin 2
//    Servo PWM     → pin 9
//    Button SW1    → pin 0 (INPUT_PULLUP, active LOW)
// ================================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <HX711.h>

#include "config.h"
#include "as5600.h"
#include "display_manager.h"
#include "servo_ctrl.h"
#include "test_results.h"
#include "report.h"

// ── Global peripherals ────────────────────────────────────────
DisplayManager  display;
AS5600Driver    encoder;
ServoController servo;
Adafruit_INA219 ina219(INA219_I2C_ADDR);
HX711           loadCell;

TestResults results;

// ── State machine ─────────────────────────────────────────────
enum class AppState {
    WELCOME,
    PHASE1_STARTING,
    STEP1_SWEEP,
    STEP2_POSITION,
    STEP3_REPEAT,
    STEP4_SPEED,
    PHASE1_RESULTS,
    WAIT_PHASE2,
    PHASE2_STARTING,
    STEP6_CONTACT,
    STEP7_TORQUE,
    STEP9_STALL,
    STEP10_HOLD,
    PHASE2_RESULTS,
    DONE
};

AppState appState = AppState::WELCOME;

// ── Button debounce ───────────────────────────────────────────
bool buttonPressed() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);
        if (digitalRead(BUTTON_PIN) == LOW) {
            while (digitalRead(BUTTON_PIN) == LOW) delay(10);
            return true;
        }
    }
    return false;
}

void waitForButton() {
    while (!buttonPressed()) delay(20);
}

// ── Sensor reads ─────────────────────────────────────────────
float readCurrentMA() {
    return ina219.getCurrent_mA();
}

float readForceGrams() {
    if (loadCell.is_ready()) {
        float raw = loadCell.get_units(3);  // average of 3 readings
        return raw;                          // calibrated in setup()
    }
    return 0.0f;
}

float readAngleDeg() {
    return encoder.readZeroedDegrees();
}

// ── STEP 1: Sweep Test ────────────────────────────────────────
void runSweepTest() {
    display.showPhaseHeader("1: Sweep Test");
    delay(800);

    results.sweepCount = 0;
    uint32_t startMs = millis();

    // Forward: 0 → 180
    for (float target = 0; target <= 180.0f; target += 2.0f) {
        servo.moveTo(target);
        delay(20);
        float actual  = readAngleDeg();
        float current = readCurrentMA();

        if (results.sweepCount < 200) {
            results.sweepData[results.sweepCount++] = {
                target, actual, current, millis() - startMs
            };
        }
        display.showSweep(actual, current, target);
    }

    // Reverse: 180 → 0
    for (float target = 180.0f; target >= 0; target -= 2.0f) {
        servo.moveTo(target);
        delay(20);
        float actual  = readAngleDeg();
        float current = readCurrentMA();

        if (results.sweepCount < 200) {
            results.sweepData[results.sweepCount++] = {
                target, actual, current, millis() - startMs
            };
        }
        display.showSweep(actual, current, target);
    }

    ReportPrinter::printSweepData(results);
    display.showStatus("Step 1 Done", "Sweep complete", "See serial log");
    delay(1500);
}

// ── STEP 2: Position Accuracy ─────────────────────────────────
void runPositionAccuracyTest() {
    const float targets[] = {0.0f, 45.0f, 90.0f, 135.0f, 180.0f};
    results.posCount = 0;

    for (uint8_t i = 0; i < 5; i++) {
        float target = targets[i];
        servo.moveTo(target);
        delay(SETTLE_TIME_MS);

        float actual = readAngleDeg();
        float error  = target - actual;

        results.posResults[results.posCount++] = {target, actual, error};
        display.showPositionTest(target, actual, error);

        char buf[24];
        snprintf(buf, sizeof(buf), "Target %.0f -> %.2f err", target, error);
#if SERIAL_DEBUG
        Serial.println(buf);
#endif
        delay(600);
    }

    ReportPrinter::printPositionAccuracy(results);
}

// ── STEP 3: Repeatability ─────────────────────────────────────
void runRepeatabilityTest() {
    display.showPhaseHeader("3: Repeatability");
    delay(500);

    results.repeat.count = REPEATABILITY_RUNS;

    for (uint8_t run = 0; run < REPEATABILITY_RUNS; run++) {
        // Start from 0
        servo.moveTo(0);
        delay(300);

        // Move to 90
        servo.moveTo(90);
        delay(SETTLE_TIME_MS);

        float actual = readAngleDeg();
        results.repeat.positions[run] = actual;

        char buf[28];
        snprintf(buf, sizeof(buf), "Run %d: %.3f deg", run + 1, actual);
        display.showStatus("Repeatability", buf,
                           run == 0 ? "Target: 90 deg" : nullptr);
#if SERIAL_DEBUG
        Serial.println(buf);
#endif
        delay(400);
    }

    results.repeat.mean   = computeMean(results.repeat.positions, REPEATABILITY_RUNS);
    results.repeat.stdDev = computeStdDev(results.repeat.positions,
                                           REPEATABILITY_RUNS, results.repeat.mean);
    ReportPrinter::printRepeatability(results);
}

// ── STEP 4: Speed & Response ──────────────────────────────────
void runSpeedTest() {
    display.showPhaseHeader("4: Speed Test");
    delay(500);

    servo.moveTo(0);
    delay(400);

    float    peakI     = 0;
    float    peakAngle = 0;
    uint32_t t0        = millis();
    uint32_t tSettle   = 0;
    bool     reached90 = false;
    uint32_t t90       = 0;

    servo.moveTo(90);

    // Sample for 2 seconds after command
    uint32_t sampleEnd = millis() + 2000;
    while (millis() < sampleEnd) {
        float angle   = readAngleDeg();
        float current = readCurrentMA();

        if (current > peakI)    peakI    = current;
        if (angle   > peakAngle) peakAngle = angle;
        if (!reached90 && angle >= 90.0f) {
            reached90 = true;
            t90 = millis() - t0;
        }

        char buf[28];
        snprintf(buf, sizeof(buf), "Ang:%.1f I:%.0fmA", angle, current);
        display.showStatus("Speed Test", buf, "0 -> 90 deg");
    }

    // Measure settle: within ±1° of 90 after peak
    float finalAngle = readAngleDeg();
    float overshoot  = max(0.0f, peakAngle - 90.0f);

    // Crude settle time: wait until stable within 0.5°
    uint32_t settleStart = millis();
    uint32_t settleTime  = 0;
    bool settled = false;
    while (millis() - settleStart < 1500 && !settled) {
        float a = readAngleDeg();
        if (abs(a - 90.0f) < 0.5f) {
            settleTime = millis() - t0;
            settled = true;
        }
        delay(10);
    }

    results.speed = {t90, overshoot, settleTime, peakI};
    ReportPrinter::printSpeedResponse(results);
}

// ── STEP 6: Contact Detection ─────────────────────────────────
void runContactDetection() {
    display.showPhaseHeader("6: Contact Detect");
    display.showStatus("Contact Detect", "Attach load now", "Move slowly fwd");
    delay(2000);

    servo.moveTo(0);
    delay(400);

    float contactAngle = 0;
    float contactForce = 0;
    bool  found = false;

    for (float angle = 0; angle <= 180.0f && !found; angle += SWEEP_STEP_DEG) {
        servo.moveTo(angle);
        delay(SWEEP_STEP_DELAY_MS);

        float force   = readForceGrams();
        float current = readCurrentMA();

        char buf[28];
        snprintf(buf, sizeof(buf), "Ang:%.1f F:%.1fg", angle, force);
        display.showStatus("Contact Detect", buf, "Moving forward...");

        if (force > LOAD_CONTACT_THRESHOLD_G) {
            contactAngle = angle;
            contactForce = force;
            found = true;
        }
    }

    results.contact = {contactAngle, contactForce};

    if (found) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Contact @ %.1f deg", contactAngle);
        display.showStatus("Contact Found!", buf);
        Serial.printf("[CONTACT] Angle: %.2f°  Force: %.1f g\n",
                      contactAngle, contactForce);
    } else {
        display.showStatus("Contact Detect", "No contact found!", "Check load cell");
        Serial.println("[CONTACT] No contact detected — check load cell calibration");
        delay(2000);
    }
    delay(1000);
}

// ── STEP 7 & 8: Torque vs Angle vs Current ────────────────────
void runTorqueTest() {
    display.showPhaseHeader("7&8: Torque Map");
    delay(500);

    results.torqueCount = 0;
    float startAngle = results.contact.contactAngleDeg;

    for (float angle = startAngle; angle <= 180.0f && results.torqueCount < 50;
         angle += SWEEP_STEP_DEG * 2) {

        servo.moveTo(angle);
        delay(SWEEP_STEP_DELAY_MS * 2);

        float force   = readForceGrams();
        float current = readCurrentMA();
        // Torque = Force[N] × Arm[m] ; force in grams → N: /1000 * 9.81
        // ARM_LENGTH_MM → m: /1000
        float torqueNmm = (force / 1000.0f * 9.81f) * ARM_LENGTH_MM;

        results.torqueData[results.torqueCount++] = {angle, force, torqueNmm, current};
        display.showTorque(angle, force, torqueNmm, current);

        Serial.printf("[TORQUE] Ang:%.1f° F:%.1fg Tq:%.3fN·mm I:%.1fmA\n",
                      angle, force, torqueNmm, current);
    }

    ReportPrinter::printTorqueData(results);
}

// ── STEP 9: Stall Test ────────────────────────────────────────
void runStallTest() {
    display.showPhaseHeader("9: Stall Test");
    delay(500);

    float lastAngle     = readAngleDeg();
    float stallAngle    = 0;
    float stallTorque   = 0;
    float stallCurrent  = 0;
    bool  stalled       = false;

    for (float angle = results.contact.contactAngleDeg;
         angle <= 180.0f && !stalled;
         angle += SWEEP_STEP_DEG) {

        servo.moveTo(angle);
        delay(SWEEP_STEP_DELAY_MS + 20);

        float actualAngle = readAngleDeg();
        float force       = readForceGrams();
        float current     = readCurrentMA();
        float torque      = (force / 1000.0f * 9.81f) * ARM_LENGTH_MM;

        display.showStall(actualAngle, current, torque);

        bool angleStuck   = abs(actualAngle - lastAngle) < STALL_ANGLE_DEAD_DEG;
        bool currentSpike = current > STALL_CURRENT_MA;

        if (angleStuck && currentSpike) {
            stallAngle   = actualAngle;
            stallTorque  = torque;
            stallCurrent = current;
            stalled      = true;
            Serial.printf("[STALL] Detected at %.2f°, Tq=%.3f N·mm, I=%.1f mA\n",
                          stallAngle, stallTorque, stallCurrent);
        }

        lastAngle = actualAngle;
    }

    if (!stalled) {
        // Record whatever we got at the end
        stallAngle   = readAngleDeg();
        stallCurrent = readCurrentMA();
        float f      = readForceGrams();
        stallTorque  = (f / 1000.0f * 9.81f) * ARM_LENGTH_MM;
        Serial.println("[STALL] No stall detected before 180°");
    }

    results.stall = {stallAngle, stallTorque, stallCurrent};
    ReportPrinter::printStall(results);
}

// ── STEP 10: Loaded Holding Test ──────────────────────────────
void runHoldTest() {
    display.showPhaseHeader("10: Hold Test");
    delay(500);

    float holdAngle = 90.0f;
    servo.moveTo(holdAngle);
    delay(SETTLE_TIME_MS);

    float startAngle = readAngleDeg();
    float sumI = 0, minI = 1e6f, maxI = 0;
    int   samples = 0;

    uint32_t holdStart = millis();
    while (millis() - holdStart < HOLDING_DURATION_MS) {
        float angle   = readAngleDeg();
        float force   = readForceGrams();
        float current = readCurrentMA();
        float torque  = (force / 1000.0f * 9.81f) * ARM_LENGTH_MM;

        sumI += current;
        if (current < minI) minI = current;
        if (current > maxI) maxI = current;
        samples++;

        display.showTorque(angle, force, torque, current);
        delay(50);
    }

    float endAngle = readAngleDeg();
    float avgI     = (samples > 0) ? sumI / samples : 0;

    results.hold = {startAngle, endAngle, endAngle - startAngle, avgI, minI, maxI};
    ReportPrinter::printHold(results);
}

// ═════════════════════════════════════════════════════════════
//  setup()
// ═════════════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== Servo Test JIG Booting ===");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // ── I2C Bus 0: OLED + AS5600 ─────────────────────────────
    Wire.begin();
    Wire.setClock(400000);

    if (!display.begin()) {
        Serial.println("ERROR: OLED not found");
        while (1) {}
    }
    display.showStatus("Booting...", "OLED OK");
    Serial.println("[OK] OLED");

    if (!encoder.begin()) {
        display.showError("AS5600 not found!");
        Serial.println("ERROR: AS5600 not found");
        while (1) {}
    }
    if (!encoder.magnetDetected()) {
        display.showError("No magnet!");
        Serial.println("WARNING: AS5600 magnet not detected");
    }
    display.showStatus("Booting...", "Encoder OK");
    Serial.println("[OK] AS5600");

    // ── I2C Bus 2: INA219 ─────────────────────────────────────
    Wire2.begin();
    Wire2.setClock(400000);
    // Point INA219 library to Wire2
    // Note: Adafruit_INA219 uses Wire by default; we pass &Wire2 here
    // via a modified constructor — if using standard lib, change address pins
    // to match or re-route Wire2 SDA/SCL at board level
    ina219 = Adafruit_INA219(INA219_I2C_ADDR);
    if (!ina219.begin(&Wire2)) {
        display.showError("INA219 not found!");
        Serial.println("ERROR: INA219 not found on Wire2");
        while (1) {}
    }
    ina219.setCalibration_32V_2A();
    display.showStatus("Booting...", "INA219 OK");
    Serial.println("[OK] INA219");

    // ── HX711 Load Cell ───────────────────────────────────────
    loadCell.begin(HX711_DAT_PIN, HX711_CLK_PIN);
    loadCell.set_scale(2280.f);  // ← calibrate this value for your load cell
    loadCell.tare();
    display.showStatus("Booting...", "HX711 OK", "Tared");
    Serial.println("[OK] HX711 tared");

    // ── Servo ─────────────────────────────────────────────────
    servo.begin();
    Serial.println("[OK] Servo attached");

    // ── Zero encoder at rest ──────────────────────────────────
    delay(300);
    encoder.zero();
    Serial.println("[OK] Encoder zeroed");

    memset(&results, 0, sizeof(results));
    delay(800);
    appState = AppState::WELCOME;
}

// ═════════════════════════════════════════════════════════════
//  loop()  — simple blocking state machine
// ═════════════════════════════════════════════════════════════
void loop() {
    switch (appState) {

    // ── WELCOME ──────────────────────────────────────────────
    case AppState::WELCOME:
        display.showWelcome();
        waitForButton();
        appState = AppState::PHASE1_STARTING;
        break;

    // ── PHASE 1 ──────────────────────────────────────────────
    case AppState::PHASE1_STARTING:
        display.showStatus("Phase 1", "Starting...", "Position Tests");
        Serial.println("\n>>> PHASE 1: POSITION TESTS <<<");
        delay(1000);
        appState = AppState::STEP1_SWEEP;
        break;

    case AppState::STEP1_SWEEP:
        display.showPhaseHeader("1: Sweep Test");
        delay(600);
        runSweepTest();
        appState = AppState::STEP2_POSITION;
        break;

    case AppState::STEP2_POSITION:
        display.showPhaseHeader("2: Position Test");
        delay(600);
        runPositionAccuracyTest();
        appState = AppState::STEP3_REPEAT;
        break;

    case AppState::STEP3_REPEAT:
        runRepeatabilityTest();
        appState = AppState::STEP4_SPEED;
        break;

    case AppState::STEP4_SPEED:
        runSpeedTest();
        appState = AppState::PHASE1_RESULTS;
        break;

    case AppState::PHASE1_RESULTS: {
        float maxErr = computeMaxAbsError(results.posResults, results.posCount);
        display.showResultsPhase1(
            maxErr,
            results.repeat.stdDev,
            (float)results.speed.time0to90Ms,
            results.speed.overshootDeg,
            results.speed.peakCurrentMA);
        ReportPrinter::printPhase1Summary(results);

        delay(3000);
        display.showPressButton("Phase 2: Load Tests");
        Serial.println("\nAttach load to servo arm, then press button...");
        waitForButton();
        appState = AppState::PHASE2_STARTING;
        break;
    }

    // ── PHASE 2 ──────────────────────────────────────────────
    case AppState::PHASE2_STARTING:
        display.showStatus("Phase 2", "Load Tests", "Starting...");
        Serial.println("\n>>> PHASE 2: LOAD / TORQUE TESTS <<<");
        loadCell.tare();  // re-tare before load tests
        delay(1000);
        appState = AppState::STEP6_CONTACT;
        break;

    case AppState::STEP6_CONTACT:
        runContactDetection();
        appState = AppState::STEP7_TORQUE;
        break;

    case AppState::STEP7_TORQUE:
        runTorqueTest();
        appState = AppState::STEP9_STALL;
        break;

    case AppState::STEP9_STALL:
        runStallTest();
        appState = AppState::STEP10_HOLD;
        break;

    case AppState::STEP10_HOLD:
        runHoldTest();
        appState = AppState::PHASE2_RESULTS;
        break;

    case AppState::PHASE2_RESULTS:
        display.showResultsPhase2(
            results.stall.stallTorqueNmm,
            results.stall.stallCurrentMA,
            results.stall.stallAngleDeg,
            results.hold.driftDeg);
        ReportPrinter::printPhase2Summary(results);
        delay(5000);
        appState = AppState::DONE;
        break;

    // ── DONE ─────────────────────────────────────────────────
    case AppState::DONE:
        display.showDone();
        servo.moveTo(0);
        // Halt — reset board to restart
        while (true) delay(1000);
        break;
    }
}
