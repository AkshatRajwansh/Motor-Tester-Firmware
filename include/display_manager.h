#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

class DisplayManager {
public:
    DisplayManager() : _oled(OLED_WIDTH, OLED_HEIGHT, &Wire1, OLED_RESET_PIN) {}

    bool begin() {
        if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
            return false;
        }
        _oled.setTextColor(SSD1306_WHITE);
        _oled.clearDisplay();
        _oled.display();
        return true;
    }

    // ── Splash / Welcome ─────────────────────────────────────
    void showWelcome() {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("SERVO TEST JIG", 0);
        _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);
        centered("System Ready", 18);
        centered("Attach motor", 32);
        centered("Press button", 44);
        _oled.display();
    }

    // ── Phase header ─────────────────────────────────────────
    void showPhaseHeader(const char* title) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        _oled.drawRect(0, 0, 128, 12, SSD1306_WHITE);
        centered(title, 2);
        _oled.display();
    }

    // ── Generic status (title + up to 3 value lines) ─────────
    void showStatus(const char* title, const char* line1,
                    const char* line2 = nullptr, const char* line3 = nullptr) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        _oled.drawRect(0, 0, 128, 12, SSD1306_WHITE);
        centered(title, 2);
        if (line1) { _oled.setCursor(0, 16); _oled.print(line1); }
        if (line2) { _oled.setCursor(0, 28); _oled.print(line2); }
        if (line3) { _oled.setCursor(0, 40); _oled.print(line3); }
        _oled.display();
    }

    // ── Live sweep data ───────────────────────────────────────
    void showSweep(float angle, float current, float target) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("SWEEP TEST", 0);
        _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        char buf[32];
        snprintf(buf, sizeof(buf), "Target: %5.1f deg", target);
        _oled.setCursor(0, 14); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Actual: %5.1f deg", angle);
        _oled.setCursor(0, 26); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Current:%6.1f mA", current);
        _oled.setCursor(0, 38); _oled.print(buf);

        // Mini progress bar (0–180)
        int barW = (int)((angle / 180.0f) * 126);
        barW = constrain(barW, 0, 126);
        _oled.drawRect(1, 54, 126, 8, SSD1306_WHITE);
        _oled.fillRect(1, 54, barW, 8, SSD1306_WHITE);
        _oled.display();
    }

    // ── Position accuracy row ─────────────────────────────────
    void showPositionTest(float target, float actual, float error) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("POSITION TEST", 0);
        _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        char buf[32];
        snprintf(buf, sizeof(buf), "Target:  %6.1f deg", target);
        _oled.setCursor(0, 14); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Actual:  %6.1f deg", actual);
        _oled.setCursor(0, 26); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Error:   %+6.2f deg", error);
        _oled.setCursor(0, 38); _oled.print(buf);
        _oled.display();
    }

    // ── Torque live data ──────────────────────────────────────
    void showTorque(float angle, float force_g, float torque_Nmm, float current) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("TORQUE TEST", 0);
        _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        char buf[32];
        snprintf(buf, sizeof(buf), "Ang: %5.1f  I:%5.0fmA", angle, current);
        _oled.setCursor(0, 14); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Force: %7.1f g", force_g);
        _oled.setCursor(0, 26); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Torque:%6.2f N.mm", torque_Nmm);
        _oled.setCursor(0, 38); _oled.print(buf);
        _oled.display();
    }

    // ── Stall detection ───────────────────────────────────────
    void showStall(float angle, float current, float torque) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("STALL TEST", 0);
        _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        char buf[32];
        snprintf(buf, sizeof(buf), "Angle:  %5.1f deg", angle);
        _oled.setCursor(0, 14); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Current:%6.1f mA", current);
        _oled.setCursor(0, 26); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Torque: %5.2f N.mm", torque);
        _oled.setCursor(0, 38); _oled.print(buf);
        _oled.display();
    }

    // ── Wait for button prompt ────────────────────────────────
    void showPressButton(const char* nextTest) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("Results Logged", 4);
        _oled.drawLine(0, 16, 127, 16, SSD1306_WHITE);
        centered("Next:", 20);
        centered(nextTest, 32);
        centered(">> Press Button <<", 48);
        _oled.display();
    }

    // ── Full result summary (scrolled via call) ───────────────
    void showResultsPhase1(float maxErr, float repeatStd, float t90ms,
                           float overshoot, float peakI) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("=PHASE 1 RESULTS=", 0);
        _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        char buf[32];
        snprintf(buf, sizeof(buf), "MaxErr: %+.2f deg", maxErr);
        _oled.setCursor(0, 14); _oled.print(buf);
        snprintf(buf, sizeof(buf), "RepStd: %.3f deg", repeatStd);
        _oled.setCursor(0, 24); _oled.print(buf);
        snprintf(buf, sizeof(buf), "T_90:   %lu ms", (unsigned long)t90ms);
        _oled.setCursor(0, 34); _oled.print(buf);
        snprintf(buf, sizeof(buf), "Ovsht:  %.1f deg", overshoot);
        _oled.setCursor(0, 44); _oled.print(buf);
        snprintf(buf, sizeof(buf), "PeakI:  %.0f mA", peakI);
        _oled.setCursor(0, 54); _oled.print(buf);
        _oled.display();
    }

    void showResultsPhase2(float stallTorque, float stallCurrent,
                           float stallAngle, float holdDrift) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        centered("=PHASE 2 RESULTS=", 0);
        _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

        char buf[32];
        snprintf(buf, sizeof(buf), "StallTq:%.2f N.mm", stallTorque);
        _oled.setCursor(0, 14); _oled.print(buf);
        snprintf(buf, sizeof(buf), "StallI: %.0f mA", stallCurrent);
        _oled.setCursor(0, 24); _oled.print(buf);
        snprintf(buf, sizeof(buf), "StallAng:%.1f deg", stallAngle);
        _oled.setCursor(0, 34); _oled.print(buf);
        snprintf(buf, sizeof(buf), "HoldDft:%.3f deg", holdDrift);
        _oled.setCursor(0, 44); _oled.print(buf);
        _oled.display();
    }

    // ── "Test Complete" final screen ──────────────────────────
    void showDone() {
        _oled.clearDisplay();
        _oled.setTextSize(2);
        centered2x("ALL DONE", 10);
        _oled.setTextSize(1);
        centered("Check serial log", 40);
        centered("for full report", 52);
        _oled.display();
    }

    void showError(const char* msg) {
        _oled.clearDisplay();
        _oled.setTextSize(1);
        _oled.drawRect(0, 0, 128, 12, SSD1306_WHITE);
        centered("!! ERROR !!", 2);
        _oled.setCursor(0, 16);
        _oled.print(msg);
        _oled.display();
    }

private:
    Adafruit_SSD1306 _oled;

    void centered(const char* txt, int y) {
        int16_t x1, y1;
        uint16_t w, h;
        _oled.setTextSize(1);
        _oled.getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
        _oled.setCursor((OLED_WIDTH - w) / 2, y);
        _oled.print(txt);
    }

    void centered2x(const char* txt, int y) {
        int16_t x1, y1;
        uint16_t w, h;
        _oled.setTextSize(2);
        _oled.getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
        _oled.setCursor((OLED_WIDTH - w) / 2, y);
        _oled.print(txt);
        _oled.setTextSize(1);
    }
};
