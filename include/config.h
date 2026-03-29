#pragma once

// =============================================================
//  Servo Motor Test JIG — Configuration
//  Board: Teensy 4.1   PCB Rev 1
// =============================================================

// ── I2C Bus assignments ───────────────────────────────────────
// Wire  (Bus 0) : pins 18 SDA / 19 SCL  → OLED + AS5600
// Wire1 (Bus 1) : pins 17 SDA / 16 SCL  → (available)
// Wire2 (Bus 2) : pins 24 SDA / 25 SCL  → INA219

// ── I2C Addresses ────────────────────────────────────────────
#define AS5600_I2C_ADDR     0x36   // fixed
#define OLED_I2C_ADDR       0x3C   // typical SSD1306
#define INA219_I2C_ADDR     0x40   // A0=A1=GND on PCB

// ── HX711 Load Cell ──────────────────────────────────────────
#define HX711_DAT_PIN       3      // DAT → Teensy pin 3
#define HX711_CLK_PIN       2      // CLK → Teensy pin 2

// ── Motor PWM ────────────────────────────────────────────────
#define SERVO_PWM_PIN       9      // Motor PWM → Teensy pin 9 (adjustable)
#define SERVO_MIN_US        500    // µs for 0°
#define SERVO_MAX_US        2500   // µs for 180°

// ── Push Button ──────────────────────────────────────────────
#define BUTTON_PIN          0      // SW1 → Teensy pin 0 (INPUT_PULLUP)

// ── OLED Display ─────────────────────────────────────────────
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_RESET_PIN      -1     // no dedicated reset pin

// ── Test Parameters ──────────────────────────────────────────
#define ARM_LENGTH_MM       50.0f  // torque arm length in mm → adjust to your fixture
#define LOAD_CONTACT_THRESHOLD_G  20.0f   // grams — contact detection force threshold
#define SETTLE_TIME_MS      400    // ms to wait after commanding position
#define STALL_CURRENT_MA    1200.0f  // mA — stall detection threshold
#define STALL_ANGLE_DEAD_DEG 1.0f  // deg — angle must change by this to be considered moving
#define REPEATABILITY_RUNS  7      // number of runs for repeatability test
#define HOLDING_DURATION_MS 3000   // ms for loaded hold test
#define SWEEP_STEP_DEG      2      // degrees per step during slow sweep (contact detect)
#define SWEEP_STEP_DELAY_MS 30     // ms between steps during slow sweep

// ── Serial Debug ─────────────────────────────────────────────
#define SERIAL_DEBUG        1      // 1 = enable verbose serial logging
