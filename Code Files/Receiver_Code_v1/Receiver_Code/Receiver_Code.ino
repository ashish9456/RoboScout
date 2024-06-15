#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <Arduino.h>


//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "ESP32"

const int enA = 12; // Enable pin for Motor 1 (PWM)
const int in1 = 14; // Input 1 of Motor 1
const int in2 = 27; // Input 2 of Motor 1

// Motor 2
const int enB = 13; // Enable pin for Motor 2 (PWM)
const int in3 = 26; // Input 1 of Motor 2
const int in4 = 25; // Input 2 of Motor 2

//START OF MOTOR FUNCTIONS
void moveForward() 
{
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        analogWrite(enA, 255); // PWM Speed Control for Motor 1
        analogWrite(enB, 255); // PWM Speed Control for Motor 2
      }

      void moveBackward() {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        analogWrite(enA, 255); // PWM Speed Control for Motor 1
        analogWrite(enB, 255); // PWM Speed Control for Motor 2
      }

      void turnLeft() {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
        analogWrite(enA, 255); // PWM Speed Control for Motor 1
        analogWrite(enB, 255); // PWM Speed Control for Motor 2
      }

      void turnRight() {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
        analogWrite(enA, 255); // PWM Speed Control for Motor 1
        analogWrite(enB, 255); // PWM Speed Control for Motor 2
      }

      void stopMotors() {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
        analogWrite(enA, 0); // Stop Motor 1
        analogWrite(enB, 0); // Stop Motor 2
      }

// END OF MOTOR FUNCTIONS

// The remote service we wish to connect to.
static BLEUUID bleServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID CHARACTERISTIC_UUID_SW("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLEAddress *pServerAddress;
static BLEAdvertisedDevice* myDevice;
 BLERemoteCharacteristic* sv_wri;

uint16_t counter = 0;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bleServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bleServiceUUID.toString().c_str());
    return (false);
  }
 sv_wri = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_SW);

 if (sv_wri == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
 

  return true;
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.print("Device found. Connecting!----::RSSI-->");
      Serial.println((int)advertisedDevice.getRSSI());
    }
  }
};
 

void setup() 
{

  //PIN CONFIG L298N
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  stopMotors();
 
  //Start serial communication
  Serial.begin(9600);
  Serial.println("Starting Arduino BLE Client application...");
  //stopMotors();

  //Init BLE device
  BLEDevice::init("ESP32");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);

  if (doConnect == true) {
    while (connectToServer(*pServerAddress)==false) {
      Serial.println("connecting...");

    }
    Serial.println("We are now connected to the BLE Server.");
}else{
    ESP.restart();
}
 
}

// This is the Arduino main loop function.
void loop() 
{

   std::string rxval=sv_wri->readValue();
  if(!rxval.empty())
  {
      Serial.print("form server: \t");
      Serial.println(rxval.c_str());
      //int data = Serial.println(rxval.c_str());
      //Serial.println(data);

      if(rxval == "UP")
      {
        Serial.println("Forward");
        turnRight();
        //moveForward();
      }

      else if(rxval == "DOWN")
      {
        Serial.println("Backward");
        //moveBackward();
        turnLeft(); 
      }

      else if(rxval == "LEFT")
      {
        Serial.println("Left");
        moveBackward();
        //turnLeft();  
      }

      else if(rxval == "RIGHT")
      {
        Serial.println("Right");
        moveForward();
        //turnRight();
      }

      else if(rxval == "NORMAL")
      {
        Serial.println("STOP");
        stopMotors();
      }

      else if(rxval == "CLEFT")
      {

      }

       else if(rxval == "CRIGHT")
      {
        
      }

       else if(rxval == "CUP")
      {
        
      }

       else if(rxval == "CDOWN")
      {
        
      }
      
      else if(rxval == "FLASHON")
      {

      }

      else if(rxval == "FLASHOFF");
      {

      }

      
      sv_wri->writeValue(rxval);





      delay(100);
  }
} // End of loop
