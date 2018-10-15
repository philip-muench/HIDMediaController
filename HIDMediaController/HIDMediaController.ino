
/*
 * HID Media Controller
 * 
 * Uses a rotary encoder with push button and 3 additional buttons
 * 
 * For hardware debouncing, 4 caps are used: 
 * on the buttons, between GND and input pin
 * on the encoder's push button, between +5V and input pin
 * I used 47 nF caps.
 * 
 * Tested on Leonardo. Input pins need interrupts -> different boards, different pins!
 * Also take a look at the HID lib docs how different boards can be used, if at all.
 * 
 * Encoder lib: https://github.com/PaulStoffregen/Encoder
 * HID lib:     https://github.com/NicoHood/HID
 * 
 */

#define DEADZONE 3
#define RE_CLK 6
#define RE_DT 5
#define RE_SW 0
#define BACK_SW 1
#define PAUSE_SW 2
#define FWD_SW 3
#define INTERVAL 1000

#include <HID-Project.h>
#include <HID-Settings.h>

#include <Encoder.h>

Encoder enc(RE_DT, RE_CLK);

byte buttonsPressed = 0;
long counter = 0;

void setup() {
  Serial.begin(9600);
  Consumer.begin();
  enc.write(0);
  /*
   * hardware debouncing: use builtin pullup resistors. 
   * Only trigger interrupts on button press, not release.
   */
  pinMode(RE_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RE_SW), mute, FALLING);
  pinMode(BACK_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BACK_SW), back, FALLING);
  pinMode(PAUSE_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PAUSE_SW), pause, FALLING);
  pinMode(FWD_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FWD_SW), fw, FALLING);
}

/*
 * software debouncing: 
 * if on one button press, multiple interrupts occur, do not send signals directly; 
 * memorize that they occured and periodically process them(see loop()).
 */

void mute() {
  buttonsPressed |= 1; 
}
void back() {
  buttonsPressed |= 2;
}
void fw() {
  buttonsPressed |= 4;
}
void pause() {
  buttonsPressed |= 8;
}

void loop() {
  /*
   * process encoder events directly, otherwise would be too slow. Use a deadzone to debounce.
   * reset encoder counter afterwards to center in deadzone again.
   */
  if(enc.read() > DEADZONE) {
    Consumer.write(MEDIA_VOLUME_DOWN);
    enc.write(0);
  }
  
  if(-enc.read() > DEADZONE) {
    Consumer.write(MEDIA_VOLUME_UP);
    enc.write(0);
  }
/*
 * software debouncing: 
 * Send signals at fixed intervals only. 
 * Do not use delay, messes with interrupts.
 */
  counter++;
  counter %= INTERVAL;
  
  if(counter == 0) {
      if(buttonsPressed & 1) {
        Consumer.write(MEDIA_VOLUME_MUTE);
      }
      if(buttonsPressed & 2) {
        Consumer.write(MEDIA_PREVIOUS);
      }
      if(buttonsPressed & 4) {
        Consumer.write(MEDIA_NEXT);
      }
      if(buttonsPressed & 8) {
        Consumer.write(MEDIA_PLAY_PAUSE);
      }
      buttonsPressed = 0;    
  }
}
