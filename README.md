This firmware runs on an onboard Teensy 4.1 MCU in a custom designed PCB for an automated servo motor testing machine. You attach a servo, press a button, and it runs 10 tests automatically — measuring position accuracy, speed, torque, stall point — and reports everything on the OLED and over serial. 

### It's split into two phases: 
- Phase 1 (no load, pure motion tests) 
- Phase 2 (with a physical load touching the servo arm, torque/force tests).

Hardware Being Used -
| Device            | What it does in this project                                      |
|------------------|-------------------------------------------------------------------|
| AS5600           | Magnetic encoder — reads the actual angle of the servo shaft      |
| INA219           | Current + voltage sensor — measures what the servo is drawing     |
| HX711 + Load Cell| Measures force being applied to the servo arm                     |
| SSD1306 OLED     | 128x64 display — shows live test status                           |
| SW1 Button       | User input — advances between test phases                         |
| Servo PWM        | The servo motor being tested                                      |
