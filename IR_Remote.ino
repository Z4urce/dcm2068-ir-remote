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
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

// Rotary encoder
#define RotaryPinA 2
#define RotaryPinB 4
#define RotaryBtnPin 7
#include <Encoder.h>
Encoder myEnc(RotaryPinA, RotaryPinB);
uint8_t Count = 0;
bool is_button_down;
bool button_changed;

// NAVIGATION ===================

typedef struct { 
  char* name;
  uint8_t device;
  uint8_t function;
  uint8_t function_pair;
} menu;

const menu main_menu[] {
    {"Volume", 16, 16, 17},
    {"Track", 20, 32, 33},
    {"Album", 20, 112, 113},
    {"Power", 20, 12, 0},
    {"Shuffle", 20, 29, 0}
};

bool inside_function;
int8_t function_counter;

// FUNCTIONS ====================


void setup() {
  Serial.begin(115200);
  
  u8g2.begin();
  u8g2.setDisplayRotation(U8G2_R2);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setFontMode(1);

  pinMode(RotaryBtnPin, INPUT_PULLUP);

  IrSender.begin();
  Serial.print(F("Init done."));
  
  return_to_menu();
}

void loop() {
  bool button_state = digitalRead(RotaryBtnPin) == LOW;
  button_changed = button_state != is_button_down;
  is_button_down = button_state;

  int8_t new_count = myEnc.read() / 4;

  if (inside_function) {
    if (is_button_pressed()) {
      return_to_menu();
      return;
    }

    if (new_count != function_counter){
      draw_function();

      send_ir_command(main_menu[Count].device, new_count > function_counter ?
        main_menu[Count].function :
        main_menu[Count].function_pair);
      
      function_counter = new_count;
    }
    return;
  }

  if (Count != new_count) {
    Count = new_count;

    Serial.print(F("Count: "));
    Serial.println(Count);

    redraw_menu();
  }

  handle_menu_button_press();
}

void handle_menu_button_press() {
  if (is_button_pressed() && Count < main_menu_length()){
    if (main_menu[Count].function_pair == 0) {
      send_ir_command(main_menu[Count].device, main_menu[Count].function);
      return;
    }
    enter_function();
  }
}

void enter_function() {
  myEnc.readAndReset();
  function_counter = 0;
  inside_function = true;
  draw_function();
}

void return_to_menu() {
  myEnc.readAndReset();
  inside_function = false;
  redraw_menu();
}

void send_ir_command(uint8_t device, uint8_t function) {
  IrSender.sendRC5(device, function, 0, true);
}

void draw_function() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawStr(32,32, main_menu[Count].name);
  u8g2.sendBuffer();
}

void redraw_menu() {
  uint8_t menu_offset = Count > 3 ? Count - 3 : 0;

  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawRBox(0, ((Count-menu_offset)%4)*16 + 4, 128, 14, 4);
  u8g2.setDrawColor(2);
  

  for(uint8_t i = 0; i < 4 || (i+menu_offset) < main_menu_length(); ++i) {
      u8g2.drawStr(16,16 + i*16, main_menu[i+menu_offset].name);
  }

  //char str[12];
  //sprintf(str, "Command: %d", Count);
  //u8g2.drawStr(16,64, str);
  u8g2.sendBuffer();
}

size_t main_menu_length() {
  return sizeof(main_menu)/sizeof(menu);
}

bool is_button_pressed() {
  return button_changed && is_button_down;
}