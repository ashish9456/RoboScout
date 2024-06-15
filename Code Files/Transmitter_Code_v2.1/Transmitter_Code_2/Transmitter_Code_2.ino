/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ezButton.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string value = "NORMAL";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//JOYSTICK FOR ROVER CONTROL
#define VRX_PIN  34
 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  35 // ESP32 pin GPIO39 (ADC0) connected to VRY pin
#define SW_PIN 4

//JOYSTICK FOR CAMERA MOVEMENT
#define VRX_PIN2  36
 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN2 39 // ESP32 pin GPIO39 (ADC0) connected to VRY pin
#define SW_PIN2 2

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

int valueX2 = 0;
int valueY2 = 0;
int command2 = COMMAND_NO;


ezButton button(SW_PIN);
ezButton button2(SW_PIN2);

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(9600);
  button.setDebounceTime(50);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() 
{
  value="NORMAL";
    // notify changed value
    if (deviceConnected)
{  //JOYSTICK PROGRAM STARTS
      button.loop();
      bValue = button.getState();
      if (button.isPressed()) 
      {
        Serial.println("The button is pressed");
        value="BUTTON_PRESSED";
      }

      if (button.isReleased()) 
      {
        Serial.println("The button is released");
        value="BUTTON_PRESSED";
      }

  // read X and Y analog values
  valueX = analogRead(VRX_PIN);
  valueY = analogRead(VRY_PIN);

  valueX2 =analogRead(VRX_PIN2);
  valueY2 = analogRead(VRY_PIN2);

  // converts the analog value to commands
  // reset commands
  command = COMMAND_NO;
  command2 = COMMAND_NO;

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

//FOR CAMERA MOVEMENT

 // check left/right commands
  if (valueX2 < LEFT_THRESHOLD)
    command2 = command2 | COMMAND_LEFT;
  else if (valueX2 > RIGHT_THRESHOLD)
    command2 = command2 | COMMAND_RIGHT;

  // check up/down commands
  if (valueY2 < UP_THRESHOLD)
    command2 = command2 | COMMAND_UP;
  else if (valueY2 > DOWN_THRESHOLD)
    command2 = command2 | COMMAND_DOWN;

  // NOTE: AT A TIME, THERE MAY BE NO COMMAND, ONE COMMAND OR TWO COMMANDS

  // print command to serial and process command

//ROVER COMMAND SIGNALS
  if (command & COMMAND_LEFT) 
  {
    Serial.println("COMMAND LEFT");
    value="LEFT";
    // TODO: add your task here
  }

  if (command & COMMAND_RIGHT) 
  {
    Serial.println("COMMAND RIGHT");
    value="RIGHT";
    // TODO: add your task here
  }

  if (command & COMMAND_UP) 
  {
    Serial.println("COMMAND UP");
    value="UP";
    // TODO: add your task here
  }

  if (command & COMMAND_DOWN) 
  {
    Serial.println("COMMAND DOWN");
    value="DOWN";
    // TODO: add your task here
  }

//ROVER COMMAND SIGNALS END

//CAMERA MOVEMENT COMMAND SIGNALS
   if (command2 & COMMAND_LEFT) 
  {
    Serial.println("CAM LEFT");
    value="CLEFT";
  }

  if (command2 & COMMAND_RIGHT) 
  {
    Serial.println("CAM RIGHT");
    value="CRIGHT";
  }

  if (command2 & COMMAND_UP) 
  {
    Serial.println("CAM UP");
    value="CUP";
  }

  if (command2 & COMMAND_DOWN) 
  {
    Serial.println("CAM DOWN");
    value="CDOWN";
  }

  //END OF JOYSTICK PROGRAM

      //std::string message = "I'm Ashish";
        pCharacteristic->setValue(value);
        pCharacteristic->notify();
        delay(20); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
}
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) 
    {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) 
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}