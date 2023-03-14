/*
Battery:
NEG - GND
POS - 3.3V

Screen:
GND - GND
VDD - 3.3V
SCK - UNO A5
SDA - UNO A4

IR Transmitter:
Shorter - Resistor - GND
Longer - UNO 3

Rotary: (upside is the two button pins)
Down Left - UNO 2
Down Center - GND
Down Right - UNO 4

Up Left - GND
Up Right - UNO 7
*/

/*
=== Commands for Philips CD player:
== Address 20:
00 - Number 0
01 - Number 1
..
09 - Number 9

12 - Power (On/Off toggle)
15 - Display information (Artist / Time)
29 - Repeat / Shuffle
32 - Next (Fast Forward)
33 - Previous
36 - "STOP ONLY" ???
45 - Eject
53 - Ok / Pause
54 - Stop
63 - Disk / Mp3 Link

112 - Next album
113 - Previous album

== Address 16:
13 - Mute
16 - Volume UP
17 - Volume DOWN
38 - Timer
70 - EQ
99 - "X-Bass 0"
*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// HARDWARE INIT ==================

// IR
#define DISABLE_CODE_FOR_RECEIVER
#define SEND_PWM_BY_TIMER
#define NO_LED_FEEDBACK_CODE
#define IR_SEND_PIN 3
#include <IRremote.hpp>

// Display
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

// Rotary encoder
#define RotaryPinA 2
#define RotaryPinB 4
#define RotaryBtnPin 7

volatile bool fired;
volatile bool rotation_cw;
uint8_t Count = 0;

// NAVIGATION ===================




// FUNCTIONS ====================


void setup() {
  Serial.begin(115200);
  
  u8g2.begin();
  u8g2.setDisplayRotation(U8G2_R2);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setFontMode(1);

  pinMode(RotaryPinA, INPUT_PULLUP);
  pinMode(RotaryPinB, INPUT_PULLUP);
  pinMode(RotaryBtnPin, INPUT_PULLUP);
  attachInterrupt (digitalPinToInterrupt(RotaryPinA), isr, CHANGE);

  IrSender.begin();
  Serial.println("Init done");
  
  redraw_menu();
}

void loop() {
  if (fired) {
    if (rotation_cw) Count++;
    else Count--;
    fired = false;

    Serial.print("Rotation: ");
    Serial.println(Count);

    redraw_menu();
  }

  if (button_pressed()){
    send_ir_command(Count);
  }
}

void send_ir_command(uint8_t function) {
  //noInterrupts();
  IrSender.sendRC5(20, function, 0, true);
  //interrupts();
}

void redraw_menu() {
  u8g2.firstPage();
  do {
    u8g2.setDrawColor(1);
    u8g2.drawRBox(0, (Count%4)*16 + 4, 128, 14, 4);
    u8g2.setDrawColor(2);
    u8g2.drawStr(16,16, "Volume");
    u8g2.drawStr(16,32, "Album");
    u8g2.drawStr(16,48, "Track");
    char str[12];
    sprintf(str, "Command: %d", Count);
    u8g2.drawStr(16,64, str);

  }  while (u8g2.nextPage());
}

void isr ()
{
  if (digitalRead (RotaryPinA) == LOW) {
    return;
  }

  rotation_cw = digitalRead(RotaryPinB);
  fired = true;
}

bool button_pressed() {
  return digitalRead(RotaryBtnPin) == LOW;
}