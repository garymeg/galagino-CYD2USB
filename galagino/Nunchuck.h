#ifndef _NUNCHUCK_H_
#define _NUNCHUCK_H_

// ----------------------------
// Standard Libraries
// ----------------------------

#include <Wire.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <NintendoExtensionCtrl.h>
// This library is for interfacing with the Nunchuck or Nintendo Gamepad

// Can be installed from the library manager
// https://github.com/dmadison/NintendoExtensionCtrl

#ifdef USE_GAMEPAD
// Use Nintendo Gamepad (SNES, Classic Controller, etc.)
ClassicController gamepad;

// Global flag for Home button to trigger menu return
volatile boolean gamepad_home_pressed = false;

void nunchuckSetup() {

  Wire.begin(NUNCHUCK_SDA, NUNCHUCK_SCL);
  if (!gamepad.connect()) {
    Serial.println("Gamepad on bus #1 not detected!");
    delay(1000);
  }

}

unsigned char getNunchuckInput() {

  boolean success = gamepad.update();  // Get new data from the controller

  if (!success) {  // Ruh roh
    Serial.println("Gamepad disconnected!");
    return 0;
  }
  else {

    // Check Home button for menu return
    if (gamepad.buttonHome()) {
      gamepad_home_pressed = true;
    }

    // Auto-fire logic for button B
    static unsigned long autofire_counter = 0;
    static boolean autofire_state = false;
    boolean fire_button = false;
    
    if (gamepad.buttonB()) {
      // Auto-fire: toggle fire state every 4 frames (~15 Hz at 60 FPS)
      autofire_counter++;
      if (autofire_counter >= 4) {
        autofire_state = !autofire_state;
        autofire_counter = 0;
      }
      fire_button = autofire_state;
    } else {
      // Reset auto-fire when button B is released
      autofire_counter = 0;
      autofire_state = false;
      // Check for manual fire with X button
      fire_button = gamepad.buttonX();
    }

    // Read left analog stick (0-63, centered around 31-32)
    int joyY = gamepad.leftJoyY();
    int joyX = gamepad.leftJoyX();
    
    // Read D-pad buttons combined with analog stick
    // Analog stick uses same threshold logic as nunchuck (scaled to 0-63 range)
    // Threshold is approximately 15 for the 0-63 range (half of nunchuck's 30 for 0-255 range)
    return (gamepad.dpadLeft()     ? BUTTON_LEFT  : 0) |       //Move Left (D-pad or analog)
           (gamepad.dpadRight()    ? BUTTON_RIGHT : 0) |       //Move Right (D-pad or analog)
           (gamepad.dpadUp()       ? BUTTON_UP    : 0) |       //Move Up (D-pad or analog)
           (gamepad.dpadDown()     ? BUTTON_DOWN  : 0) |       //Move Down (D-pad or analog)
           (fire_button            ? BUTTON_FIRE  : 0) |       //Fire with X button or auto-fire with B
           (gamepad.buttonStart()  ? BUTTON_START : 0) |       //Start button
           (gamepad.buttonSelect() ? BUTTON_COIN  : 0);        //Select button for coin
  }
}

#else
// Use Nunchuck (default)
Nunchuk nchuk;

void nunchuckSetup() {

  Wire.begin(NUNCHUCK_SDA, NUNCHUCK_SCL);
  if (!nchuk.connect()) {
    Serial.println("Nunchuk on bus #1 not detected!");
    delay(1000);
  }

}

unsigned char getNunchuckInput() {

  boolean success = nchuk.update();  // Get new data from the controller

  if (!success) {  // Ruh roh
    Serial.println("Nunchuk disconnected!");
    return 0;
  }
  else {

    // Read a joystick axis (0-255, X and Y)
    // Roughly 127 will be the axis centered
    int joyY = nchuk.joyY();
    int joyX = nchuk.joyX();

    return ((joyX < 127 - NUNCHUCK_MOVE_THRESHOLD) ? BUTTON_LEFT : 0) | //Move Left
           ((joyX > 127 + NUNCHUCK_MOVE_THRESHOLD) ? BUTTON_RIGHT : 0) | //Move Right
           ((joyY > 127 + NUNCHUCK_MOVE_THRESHOLD) ? BUTTON_UP : 0) | //Move Up
           ((joyY < 127 - NUNCHUCK_MOVE_THRESHOLD) ? BUTTON_DOWN : 0) | //Move Down
           (nchuk.buttonZ() ? BUTTON_FIRE : 0) |
           (nchuk.buttonC() ? BUTTON_EXTRA : 0) ;
  }
}

#endif // USE_GAMEPAD

#endif //_NUNCHUCK_H_
