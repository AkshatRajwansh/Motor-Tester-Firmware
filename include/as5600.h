#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// AS5600 register map
#define AS5600_REG_ANGLE_H  0x0E
#define AS5600_REG_ANGLE_L  0x0F
#define AS5600_REG_STATUS   0x0B
#define AS5600_REG_AGC      0x1A
#define AS5600_REG_MAGNITUDE_H 0x1B
#define AS5600_REG_MAGNITUDE_L 0x1C

class AS5600Driver {
public:
    AS5600Driver() : _offset(0), _lastRawAngle(0), _totalTurns(0) {}

    bool begin() {
        Wire.begin();
        Wire.setClock(400000);
        // Check presence by reading status
        Wire.beginTransmission(AS5600_I2C_ADDR);
        return (Wire.endTransmission() == 0);
    }

    // Returns raw 12-bit angle [0–4095]
    uint16_t readRaw() {
        Wire.beginTransmission(AS5600_I2C_ADDR);
        Wire.write(AS5600_REG_ANGLE_H);
        Wire.endTransmission(false);
        Wire.requestFrom(AS5600_I2C_ADDR, 2);
        if (Wire.available() < 2) return 0;
        uint16_t hi = Wire.read();
        uint16_t lo = Wire.read();
        return ((hi << 8) | lo) & 0x0FFF;
    }

    // Returns angle in degrees [0.0 – 359.9]
    float readDegrees() {
        return rawToDeg(readRaw());
    }

    // Zero the current position as reference
    void zero() {
        _offset = readRaw();
    }

    // Returns zeroed degrees (handles wraparound)
    float readZeroedDegrees() {
        uint16_t raw = readRaw();
        int16_t diff = (int16_t)raw - (int16_t)_offset;
        if (diff < 0)    diff += 4096;
        if (diff >= 4096) diff -= 4096;
        return rawToDeg((uint16_t)diff);
    }

    // Status: bit0=MH, bit1=ML, bit2=MD (MD=1 means magnet detected)
    uint8_t readStatus() {
        Wire.beginTransmission(AS5600_I2C_ADDR);
        Wire.write(AS5600_REG_STATUS);
        Wire.endTransmission(false);
        Wire.requestFrom(AS5600_I2C_ADDR, 1);
        return Wire.available() ? Wire.read() : 0;
    }

    bool magnetDetected() {
        return (readStatus() & 0x20) != 0;
    }

private:
    uint16_t _offset;
    uint16_t _lastRawAngle;
    int32_t  _totalTurns;

    float rawToDeg(uint16_t raw) {
        return (raw / 4096.0f) * 360.0f;
    }
};
