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
BLECharacteristic* pCharacteristic_rov = NULL;
BLECharacteristic* pCharacteristic_cam = NULL;
BLECharacteristic* pCharacteristic_arm = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string rov_con = "NORMAL";              //Default values to be sent to the client
std::string cam_con = "NORMAL";  
String motor_val ;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_ROV "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_CAM "9463dabd-a1d5-4358-abed-2e9b17507355"
#define CHARACTERISTIC_UUID_ARM "87159ea6-d2c0-4fa6-9df5-e4e867d2b5e3"

//Joystick Setup//
//JOYSTICK FOR ROVER CONTROL
#define ROV_JX_PIN  34
#define ROV_JY_PIN  35
#define ROV_SW_PIN 4 //BUTTON ON ROVER CONTROL JOYSTICK

//JOYSTICK FOR CAMERA MOVEMENT
#define CAM_JX_PIN  36
#define CAM_JY_PIN 39 
#define CAM_SW_PIN 17 //BUTTON ON CAMERA MOVEMENT CONTROL JOYSTICK

//joystick threshold value
#define LEFT_THRESHOLD  1000
#define RIGHT_THRESHOLD 3000
#define UP_THRESHOLD    1000
#define DOWN_THRESHOLD  3000

#define COMMAND_NO     0x00
#define COMMAND_LEFT   0x01
#define COMMAND_RIGHT  0x02
#define COMMAND_UP     0x04
#define COMMAND_DOWN   0x08


//--rover joystick values
int rov_value_X = 0 ; // to store the X-axis value
int rov_value_Y = 0 ; // to store the Y-axis value
int rov_btn_Value = 0; // To store value of the button
int rov_command = COMMAND_NO;


//--cam joystick values
int cam_value_X = 0;
int cam_value_Y = 0;
bool cam_btn_Value = 0;
int cam_command = COMMAND_NO;

//pins for potentiometers (arm control)
// 32,33,25,26,27 -->> free analog pins
const int pot_Pin1 = 15;
const int pot_Pin2 = 2;
const int pot_Pin3 = 32;
const int pot_Pin4 = 33;
const int pot_Pin5 = 25;

int mot_val_1;
int mot_val_2;
int mot_val_3;
int mot_val_4;
int mot_val_5;

ezButton button(ROV_SW_PIN);
ezButton button2(CAM_SW_PIN);
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
  pCharacteristic_rov = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_ROV,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  
  // BLE Characteristic for camera movement control
   pCharacteristic_cam = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_CAM,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

    // BLE Characteristic for robot arm angle control
    pCharacteristic_arm = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_ARM,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic_rov->addDescriptor(new BLE2902());
  pCharacteristic_cam->addDescriptor(new BLE2902());
  pCharacteristic_arm->addDescriptor(new BLE2902());

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
  rov_con="NORMAL";
  cam_con="NORMAL";

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
//  flash buttons
      button2.loop();
        cam_btn_Value = button2.getState();
        if (button2.isPressed()) 
        {
          Serial.println("Flash ON");
          cam_con="FLASHON";
          print_on_OLED("FLASH");
        }

        if (button2.isReleased()) 
        {
          Serial.println("Flash OFF");
          cam_con="FLASHOFF";
          print_on_OLED("FLASH");
      }



  // read X and Y analog values
  rov_value_X = analogRead(ROV_JX_PIN);
  rov_value_Y = analogRead(ROV_JY_PIN);

  cam_value_X =analogRead(CAM_JX_PIN);
  cam_value_Y = analogRead(CAM_JY_PIN);

  // converts the analog value to commands
  // reset commands
  rov_command= COMMAND_NO;
  cam_command = COMMAND_NO;

  // check left/right commands
  if (rov_value_X < LEFT_THRESHOLD)
    rov_command=  COMMAND_LEFT;
  else if (rov_value_X > RIGHT_THRESHOLD)
    rov_command= COMMAND_RIGHT;

  // check up/down commands
  else if (rov_value_Y < UP_THRESHOLD)
    rov_command= COMMAND_UP;
  else if (rov_value_Y > DOWN_THRESHOLD)
    rov_command= COMMAND_DOWN;

//FOR CAMERA MOVEMENT

 // check left/right commands
  if (cam_value_X < LEFT_THRESHOLD)
    cam_command = COMMAND_LEFT;
  else if (cam_value_X > RIGHT_THRESHOLD)
    cam_command = COMMAND_RIGHT;

  // check up/down commands
  else if (cam_value_Y < UP_THRESHOLD)
    cam_command =COMMAND_UP;
  else if (cam_value_Y > DOWN_THRESHOLD)
    cam_command =COMMAND_DOWN;

  // NOTE: AT A TIME, THERE MAY BE NO COMMAND, ONE COMMAND OR TWO COMMANDS

  // print command to serial and process command

//ROVER COMMAND SIGNALS
  if (rov_command == COMMAND_LEFT) 
  {
    Serial.println("COMMAND LEFT");
    rov_con="LEFT";
    print_on_OLED("R_LEFT");
    // TODO: add your task here
  }

  else if (rov_command ==COMMAND_RIGHT) 
  {
    Serial.println("COMMAND RIGHT");
    rov_con="RIGHT";
    print_on_OLED("R_RIGHT");
    // TODO: add your task here
  }

  else if (rov_command ==COMMAND_UP) 
  {
    Serial.println("COMMAND UP");
    rov_con="UP";
    //print_on_OLED("R_UP");
    print_on_OLED("R_FRONT");
    // TODO: add your task here
  }

   else if (rov_command == COMMAND_DOWN) 
  {
    Serial.println("COMMAND DOWN");
    rov_con="DOWN";
    print_on_OLED("R_BACK");
    // TODO: add your task here
  }
  /*else
  {
    Serial.println("STILL");
    rov_con="NORMAL";
    print_on_OLED("STILL");
    
  }*/

//ROVER COMMAND SIGNALS END

//CAMERA MOVEMENT COMMAND SIGNALS
   if (cam_command == COMMAND_LEFT) 
  {
    Serial.println("CAM LEFT");
    cam_con="CLEFT";
    print_on_OLED("C_LEFT");
  }

  else if (cam_command == COMMAND_RIGHT) 
  {
    Serial.println("CAM RIGHT");
    cam_con="CRIGHT";
    print_on_OLED("C_RIGHT");
  }

  else if (cam_command == COMMAND_UP) 
  {
    Serial.println("CAM UP");
    cam_con="CUP";
    print_on_OLED("C_UP");
  }

 else if (cam_command == COMMAND_DOWN) 
  {
    Serial.println("CAM DOWN");
    cam_con="CDOWN";
    print_on_OLED("C_DOWN");
  }

  /*else
  {
    Serial.println("CAM STILL");
    cam_con="NORMAL";
    print_on_OLED("CAM STILL");
  }*/
//END OF JOYSTICK PROGRAM

  // reading analog values of potentiometer
  int sensor_v1 = analogRead(pot_Pin1);
  int sensor_v2 = analogRead(pot_Pin2);
  int sensor_v3 = analogRead(pot_Pin3);
  int sensor_v4 = analogRead(pot_Pin4);
  int sensor_v5 = analogRead(pot_Pin5);
  // One potentiometer for claw motor to be added later
  

  /*mapping analog values of potentiometers to the servo motors*/
  mot_val_1 = map(sensor_v1,0,4095,70,520);
  mot_val_2 = map(sensor_v2,0,4095,70,520);
  mot_val_3 = map(sensor_v3,0,4095,70,520);
  mot_val_4 = map(sensor_v4,0,4095,70,520);
  mot_val_5 = map(sensor_v5,0,4095,70,520);


  motor_val= motor_val + String(mot_val_1) + "-" + String(mot_val_2)+ "-" + String(mot_val_3)+ "-" + String(mot_val_4)+ "-" + String(mot_val_5);

  Serial.println(motor_val);
  const char* value3 = motor_val.c_str();
  std::string stdString(value3);


        pCharacteristic_rov->setValue(rov_con);
        pCharacteristic_rov->notify();
        delay(20); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms

        pCharacteristic_cam->setValue(cam_con);
        pCharacteristic_cam->notify();
        delay(20);

        pCharacteristic_arm->setValue(value3);
        pCharacteristic_arm->notify();
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