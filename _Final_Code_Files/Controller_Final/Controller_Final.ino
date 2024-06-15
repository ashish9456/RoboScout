#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// #include <ezButton.h>
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
const char* arm_con;
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

bool bt_state = false ;
bool send_st = true;

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
#define COMMAND_FLN   0xFF
#define COMMAND_FLO    0x55

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

const int pot_Pins[6] = {15,2,32,33,25,26};
int sensor_values[6]  = {350},i;
int motor_values[6]   = {0};

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

void print_on_OLED(char arr[], bool a)
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  if(a)
  {display.setCursor(10, 30);}
  else{
    display.setCursor(10,50);
  }
  // Display static text
  display.println(arr);
  display.display(); 
}

void compare(int *sensor_value,int i)
{
  int curr_val = analogRead(pot_Pins[i]);
  if(abs(curr_val - *sensor_value) >= 12)
    *sensor_value = curr_val;
}

void setup() 
{
  pinMode(CAM_SW_PIN,INPUT_PULLUP);
  Serial.begin(9600);
  // button.setDebounceTime(50);        //Debounce time for joystick button

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



  // read X and Y analog values
  rov_value_X = analogRead(ROV_JX_PIN);
  rov_value_Y = analogRead(ROV_JY_PIN);

  cam_value_X =analogRead(CAM_JX_PIN);
  cam_value_Y = analogRead(CAM_JY_PIN);

  if(digitalRead(CAM_SW_PIN)==LOW){
    bt_state=!bt_state;
    send_st=false;
    Serial.print("swpressed:");
    Serial.print(bt_state);
    Serial.println(send_st);
    delay(100);
  }

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
  else if(!send_st){

    bt_state?cam_command=COMMAND_FLN:cam_command=COMMAND_FLO;
    send_st=true;
    Serial.println(send_st);
   

  }

  // NOTE: AT A TIME, THERE MAY BE NO COMMAND, ONE COMMAND OR TWO COMMANDS

  // print command to serial and process command

//ROVER COMMAND SIGNALS
  if (rov_command == COMMAND_LEFT) 
  {
    Serial.println("COMMAND LEFT");
    rov_con="LEFT";
    print_on_OLED("R_LEFT",true);
    // TODO: add your task here
  }

  else if (rov_command ==COMMAND_RIGHT) 
  {
    Serial.println("COMMAND RIGHT");
    rov_con="RIGHT";
    print_on_OLED("R_RIGHT",true);
    // TODO: add your task here
  }

  else if (rov_command ==COMMAND_UP) 
  {
    Serial.println("COMMAND UP");
    rov_con="UP";
    //print_on_OLED("R_UP");
    print_on_OLED("R_FRONT",true);
    // TODO: add your task here
  }

   else if (rov_command == COMMAND_DOWN) 
  {
    Serial.println("COMMAND DOWN");
    rov_con="DOWN";
    print_on_OLED("R_BACK",true);
    // TODO: add your task here
  }
  else if(rov_command == COMMAND_NO)
  {
    Serial.println("STILL");
    rov_con="NORMAL";
    print_on_OLED("STILL",true);
    
  }

//ROVER COMMAND SIGNALS END

//CAMERA MOVEMENT COMMAND SIGNALS
   if (cam_command == COMMAND_LEFT) 
  {
    Serial.println("CAM LEFT");
    cam_con="CLEFT";
    print_on_OLED("C_LEFT",false);
  }

  else if (cam_command == COMMAND_RIGHT) 
  {
    Serial.println("CAM RIGHT");
    cam_con="CRIGHT";
    print_on_OLED("C_RIGHT",false);
  }

  else if (cam_command == COMMAND_UP) 
  {
    Serial.println("CAM UP");
    cam_con="CUP";
    print_on_OLED("C_UP",false);
  }

 else if (cam_command == COMMAND_DOWN) 
  {
    Serial.println("CAM DOWN");
    cam_con="CDOWN";
    print_on_OLED("C_DOWN",false);
  }

  else if(cam_command==COMMAND_NO)
  {
    Serial.println("CAM STILL");
    cam_con="NORMAL";
    print_on_OLED("CAM STILL",false);
  }else if(cam_command==COMMAND_FLN){
    cam_con="FLASHON";
    print_on_OLED("FLASH ON",false);
  }else if(cam_command==COMMAND_FLO){
    cam_con="FLASHOFF";
    print_on_OLED("FLASH_OF",false);
  }
//END OF JOYSTICK PROGRAM

  // reading analog values of potentiometer
  for(i=0;i<6;i++)
  {
    sensor_values[i] = analogRead(pot_Pins[i]);
  }
   delay(50);

   for(i=0;i<6;i++)
   {
      compare(&sensor_values[i],i);
   }
  
  /*mapping analog values of potentiometers to the servo motors*/
  motor_values[0] = map(sensor_values[0],0,4095,40,520);
  motor_values[1] = map(sensor_values[1],0,4095,70,520);
  motor_values[2] = map(sensor_values[2],0,4095,40,520);
  motor_values[3] = map(sensor_values[3],0,4095,30,520);
  motor_values[4] = map(sensor_values[4],0,4095,70,520);
  motor_values[5] = map(sensor_values[5],0,4095,70,520);


  motor_val= motor_val + String(motor_values[0]) + "-" + String(motor_values[1])+ "-" + String(motor_values[2])+ "-" + String(motor_values[3])+ "-" + String(motor_values[4])+"-" + String(motor_values[5]);

  Serial.println(motor_val);
  arm_con = motor_val.c_str();
  std::string stdString(arm_con);


        pCharacteristic_rov->setValue(rov_con);
        pCharacteristic_rov->notify();
        delay(20); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms

        pCharacteristic_cam->setValue(cam_con);
        pCharacteristic_cam->notify();
        delay(20);

        pCharacteristic_arm->setValue(arm_con);
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