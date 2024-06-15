#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();              //defining the adafruit motor driver
//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "ESP32"

//Pins for all 3 rover left motors
const int enA = 12; // Enable pin for Motor 1 (PWM)
const int in1 = 14; // Input 1 of Motor 1
const int in2 = 27; // Input 2 of Motor 1

//Pins for all 3 rover right motors
const int enB = 13; // Enable pin for Motor 2 (PWM)
const int in3 = 26; // Input 1 of Motor 2
const int in4 = 25; // Input 2 of Motor 2

#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

int xct=300,yct=300;
long int ct = 0;     //For camera flash
const int flash_pin = 15; //pin for camera flash

// our servo # counter
uint8_t servonum = 0;

//MOTOR FUNCTIONS
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

void setServoPulse(uint8_t n, double pulse) 
  {
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= SERVO_FREQ;   // Analog servos run at ~60 Hz updates
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000000;  // convert input seconds to us
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}

// The remote service we wish to connect to.
static BLEUUID bleServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID CHARACTERISTIC_UUID_SW("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID CHARACTERISTIC_UUID_SW_2("9463dabd-a1d5-4358-abed-2e9b17507355");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLEAddress *pServerAddress;
static BLEAdvertisedDevice* myDevice;
 BLERemoteCharacteristic* sv_wri;
 BLERemoteCharacteristic* sv_wri2;


//uint16_t counter = 0;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) 
{
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
 sv_wri2 = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_SW_2);


 if (sv_wri == nullptr) 
 {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");

if (sv_wri2 == nullptr) 
 {
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
  pinMode(flash_pin, OUTPUT);

  stopMotors();
 
  //Start serial communication
  Serial.begin(9600);
  Serial.println("Starting Arduino BLE Client application...");

   pwm.begin();
   pwm.setOscillatorFrequency(27000000);
   pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

   delay(10);

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
  std::string rxval2=sv_wri2->readValue();
  
  if(!rxval.empty())
  {
      Serial.print("rover control: \t");
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
      
      sv_wri->writeValue(rxval);
      delay(100);
  }
  if(!rxval2.empty())
    {
      Serial.print("camera mov control \t");
      Serial.println(rxval2.c_str());
    if(rxval2 == "FLASHON" || rxval2 == "FLASHOFF")
      {
        ct=ct+1;
        Serial.println("FLASH");
        if(ct % 2 == 1)
        {
          digitalWrite(flash_pin, HIGH);
        }
        else if(ct % 2 == 0)
        {
          digitalWrite(flash_pin,LOW);
        }

      }

    else if(rxval2 == "CLEFT")
      {
         if(xct<=440)
        {
          xct+=10;
          Serial.println(xct);
          pwm.setPWM(2,0,xct);
        }
        else
        {
          xct=300;
        }
      }

    else if(rxval2 == "CRIGHT")
      {
        if(xct >= 160)
        {
          xct-=10;
          Serial.println(xct);
          pwm.setPWM(2,0,xct);
        }
        else
        {
          xct=300;
        }

      }

    else if(rxval2 == "CUP")
      {
        if(yct >= 160)
        {
          yct-=10;
          Serial.println(yct);
          pwm.setPWM(3,0,yct);
        }
        else
        {
          yct=300;
        }
        
      }

    else if(rxval2 == "CDOWN")
      {
        if(yct<=440)
        {
          yct+=10;
          Serial.println(yct);
          pwm.setPWM(3,0,yct);
        }
        else
        {
          yct=300;
        }
      }
    }
      
} // End of loop
