This firmware runs on an onboard Teensy 4.1 MCU in a custom designed PCB for an automated servo motor testing machine. You attach a servo, press a button, and it runs 10 tests automatically — measuring position accuracy, speed, torque, stall point — and reports everything on the OLED and over serial. 

It's split into two phases: 
Phase 1 (no load, pure motion tests) 
Phase 2 (with a physical load touching the servo arm, torque/force tests).
