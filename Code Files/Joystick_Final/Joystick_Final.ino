/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-joystick
 */
#include <ezButton.h>

#define VRX_PIN  36 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  39 // ESP32 pin GPIO39 (ADC0) connected to VRY pin
#define SW_PIN 17

#define LEFT_THRESHOLD  1000
#define RIGHT_THRESHOLD 3000
#define UP_THRESHOLD    1000
#define DOWN_THRESHOLD  3000

#define COMMAND_NO     0x00
#define COMMAND_LEFT   0x01
#define COMMAND_RIGHT  0x02
#define COMMAND_UP     0x04
#define COMMAND_DOWN   0x08

int valueX = 0 ; // to store the X-axis value
int valueY = 0 ; // to store the Y-axis value
int bValue = 0; // To store value of the button
int command = COMMAND_NO;

ezButton button(SW_PIN);

void setup() {
  Serial.begin(9600) ;
  button.setDebounceTime(50);
}

void loop() 
{
  button.loop();
  bValue = button.getState();
  if (button.isPressed()) {
    Serial.println("The button is pressed");
    // TODO do something here
  }

  if (button.isReleased()) {
    Serial.println("The button is released");
    // TODO do something here
  }

  // read X and Y analog values
  valueX = analogRead(VRX_PIN);
  valueY = analogRead(VRY_PIN);

  // converts the analog value to commands
  // reset commands
  command = COMMAND_NO;

  // check left/right commands
  if (valueX < LEFT_THRESHOLD)
    command = command | COMMAND_LEFT;
  else if (valueX > RIGHT_THRESHOLD)
    command = command | COMMAND_RIGHT;

  // check up/down commands
  if (valueY < UP_THRESHOLD)
    command = command | COMMAND_UP;
  else if (valueY > DOWN_THRESHOLD)
    command = command | COMMAND_DOWN;

  // NOTE: AT A TIME, THERE MAY BE NO COMMAND, ONE COMMAND OR TWO COMMANDS

  // print command to serial and process command

  if (command & COMMAND_LEFT) {
    Serial.println("COMMAND LEFT");
    // TODO: add your task here
  }

  if (command & COMMAND_RIGHT) {
    Serial.println("COMMAND RIGHT");
    // TODO: add your task here
  }

  if (command & COMMAND_UP) {
    Serial.println("COMMAND UP");
    // TODO: add your task here
  }

  if (command & COMMAND_DOWN) {
    Serial.println("COMMAND DOWN");
    // TODO: add your task here
  }
  else
  {
    Serial.println("Normal");
  }
}
