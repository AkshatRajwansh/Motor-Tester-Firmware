#pragma once

// =============================================================
//  Servo Motor Test JIG — Configuration

// ── I2C Bus assignments ───────────────────────────────────────
// Wire  (Bus 0) : pins 18 SDA / 19 SCL  → AS5600
// Wire1 (Bus 1) : pins 17 SDA / 16 SCL  → OLED
// Wire2 (Bus 2) : pins 24 SDA / 25 SCL  → INA219

#define AS5600_I2C_ADDR     0x36   // fixed
#define OLED_I2C_ADDR       0x3C   // typical SSD1306
#define INA219_I2C_ADDR     0x40   // A0=A1=GND on PCB

#define HX711_DAT_PIN       3      
#define HX711_CLK_PIN       2      

#define SERVO_PWM_PIN       9      
#define SERVO_MIN_US        500    
#define SERVO_MAX_US        2500   

#define BUTTON_PIN          0      

#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_RESET_PIN      -1     

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
