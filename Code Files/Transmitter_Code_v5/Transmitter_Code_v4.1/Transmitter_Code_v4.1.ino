#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ezButton.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Setting up OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLECharacteristic* pCharacteristic3 = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string value = "NORMAL";              //Default values to be sent to the client
std::string value2 = "NORMAL";  
String motor_val ;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID2 "9463dabd-a1d5-4358-abed-2e9b17507355"
#define CHARACTERISTIC_UUID3 "87159ea6-d2c0-4fa6-9df5-e4e867d2b5e3"

//Joystick Setup//
//JOYSTICK FOR ROVER CONTROL
#define VRX_PIN  34
#define VRY_PIN  35
#define SW_PIN 4 //BUTTON ON ROVER CONTROL JOYSTICK

//JOYSTICK FOR CAMERA MOVEMENT
#define VRX_PIN2  36
#define VRY_PIN2 39 
#define SW_PIN2 17 //BUTTON ON CAMERA MOVEMENT CONTROL JOYSTICK

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
int bValue2 = 0;
int command2 = COMMAND_NO;

//pins for potentiometers (arm control)
const int pot_Pin1 = 15;
const int pot_Pin2 = 2;
int mot_val_1;
int mot_val_2;

ezButton button(SW_PIN);
ezButton button2(SW_PIN2);
//End of joystick setup


class MyServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) 
    {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) 
    {
      deviceConnected = false;
    }
};

void print_on_OLED(char arr[])
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(35, 30);
  // Display static text
  display.println(arr);
  display.display(); 
}

void setup() 
{
  Serial.begin(9600);
  button.setDebounceTime(50);        //Debounce time for joystick button

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // BLE Characteristic for rover control
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  
  // BLE Characteristic for camera movement control
   pCharacteristic2 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID2,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

    // BLE Characteristic for robot arm angle control
    pCharacteristic3 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID3,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic2->addDescriptor(new BLE2902());
  pCharacteristic3->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { // Address 0x3D for 128x64
   Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

}

void loop() 
{
  value="NORMAL";
  value2="NORMAL";

     // notify changed value
    if (deviceConnected)
{ //Display connected on OLED //
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  // Display static text
  display.println("CONNECTED");
  display.display(); 


  //JOYSTICK PROGRAM STARTS
      button2.loop();
      bValue2 = button2.getState();
      if (button2.isPressed()) 
      {
        Serial.println("Flash ON");
        value2="FLASHON";
        print_on_OLED("FLASH");
      }

      if (button2.isReleased()) 
      {
        Serial.println("Flash OFF");
        value2="FLASHOFF";
        print_on_OLED("FLASH");
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
    print_on_OLED("R_LEFT");
    // TODO: add your task here
  }

  if (command & COMMAND_RIGHT) 
  {
    Serial.println("COMMAND RIGHT");
    value="RIGHT";
    print_on_OLED("R_RIGHT");
    // TODO: add your task here
  }

  if (command & COMMAND_UP) 
  {
    Serial.println("COMMAND UP");
    value="UP";
    //print_on_OLED("R_UP");
    print_on_OLED("R_FRONT");
    // TODO: add your task here
  }

  if (command & COMMAND_DOWN) 
  {
    Serial.println("COMMAND DOWN");
    value="DOWN";
    print_on_OLED("R_BACK");
    // TODO: add your task here
  }

//ROVER COMMAND SIGNALS END

//CAMERA MOVEMENT COMMAND SIGNALS
   if (command2 & COMMAND_LEFT) 
  {
    Serial.println("CAM LEFT");
    value2="CLEFT";
    print_on_OLED("C_LEFT");
  }

  if (command2 & COMMAND_RIGHT) 
  {
    Serial.println("CAM RIGHT");
    value2="CRIGHT";
    print_on_OLED("C_RIGHT");
  }

  if (command2 & COMMAND_UP) 
  {
    Serial.println("CAM UP");
    value2="CUP";
    print_on_OLED("C_UP");
  }

  if (command2 & COMMAND_DOWN) 
  {
    Serial.println("CAM DOWN");
    value2="CDOWN";
    print_on_OLED("C_DOWN");
  }
//END OF JOYSTICK PROGRAM

  // reading analog values of potentiometer
  int sensor_v1 = analogRead(pot_Pin1);
  int sensor_v2 = analogRead(pot_Pin2);

  /*mapping analog values of potentiometers to the servo motors*/
  mot_val_1 = map(sensor_v1,0,4095,70,520);
  mot_val_2 = map(sensor_v2,0,4095,70,520);

  motor_val= motor_val + String(mot_val_1) + "-" + String(mot_val_2);

  Serial.println(motor_val);
  const char* value3 = motor_val.c_str();
  std::string stdString(value3);


        pCharacteristic->setValue(value);
        pCharacteristic->notify();
        delay(20); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms

        pCharacteristic2->setValue(value2);
        pCharacteristic2->notify();
        delay(20);

        pCharacteristic3->setValue(value3);
        pCharacteristic3->notify();
        delay(20);
}
    // disconnecting
        
  else
        {
        //Displaying not connected status on OLED
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(45, 10);
        // Display static text
        display.println("NOT");
        display.setCursor(10,30);
        display.println("CONNECTED");
        display.display();
        }

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
    motor_val="";
}