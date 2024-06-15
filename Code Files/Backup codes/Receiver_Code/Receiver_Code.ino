#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 39 // OLED display height, in pixels

#define BUZZER_PIN 23

//for U-S sensor distance calculation
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//defining pins for ultrasonic sensors 
const int trigPin_f = 5;   //ultrasonic sensor - forward
const int echoPin_f = 18;

const int trigPin_r = 17;  //ultrasonic sensor - right
const int echoPin_r = 16;

const int trigPin_l = 0;   //ultrasonic sensor - left
const int echoPin_l = 4;

//define sound speed in cm/uS
long int duration;
int distanceCm;

//Initialising distances with 0 value from the U-S sensors
int d_f=0,d_r=0,d_l=0;  // d_f -> distance_front, d_r -> distance_right 
                        // d_l -> distance_left

//defining the adafruit motor driver
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();             
//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "ESP32"

//Pins for all 3 rover left rover motors
const int enA = 12; // Enable pin for Motor 1 (PWM)
const int in1 = 14; // Input 1 of Motor 1
const int in2 = 27; // Input 2 of Motor 1

//Pins for all 3 rover right rover motors
const int enB = 13; // Enable pin for Motor 2 (PWM)
const int in3 = 26; // Input 1 of Motor 2
const int in4 = 25; // Input 2 of Motor 2

#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

int xct=300,yct=300; // for camera servo motors
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

//Function for distance calc for U-S sensors
int cal_dist(int trigPin, int echoPin)
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  return distanceCm;
}

//Buzzer function
void buzz(int interval) 
{
  //Inverse logic. Pull down resistance
  digitalWrite(BUZZER_PIN, LOW);  // Turn on the buzzer
  delay(interval);                  // Wait for the specified duration
  digitalWrite(BUZZER_PIN, HIGH);   // Turn off the buzzer
}

//Printing on 128 x 32 OLED dispaly 
void print_on_OLED(char arr[])
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(12, 15);
  // Display static text
  display.println(arr);
  display.display();
  //Serial.println("Buzzer ON");
  //buzz(800);
}

//Servo motor function
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
static BLEUUID CHARACTERISTIC_UUID_SW_3("87159ea6-d2c0-4fa6-9df5-e4e867d2b5e3");


static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLEAddress *pServerAddress;
static BLEAdvertisedDevice* myDevice;
 BLERemoteCharacteristic* sv_wri;
 BLERemoteCharacteristic* sv_wri2;
 BLERemoteCharacteristic* sv_wri3;


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
 sv_wri3 = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_SW_3);


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
  pinMode(trigPin_f, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_f, INPUT); // Sets the echoPin as an Input
  pinMode(trigPin_r, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_r, INPUT); // Sets the echoPin as an Input
  pinMode(trigPin_l, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_l, INPUT); // Sets the echoPin as an Input
  pinMode(BUZZER_PIN, OUTPUT);

  stopMotors();
  //print_on_OLED("POWER ON");
  //digitalWrite(BUZZER_PIN, HIGH);
  
 
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  //Start serial communication
  Serial.begin(9600);
  Serial.println("Starting Arduino BLE Cliewnt application...");

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
  digitalWrite(BUZZER_PIN, HIGH);
  display.clearDisplay();  // clearing OLED display in every loop

  std::string rxval=sv_wri->readValue();
  std::string rxval2=sv_wri2->readValue();
  std::string rxval3=sv_wri3->readValue();
  
  if(!rxval.empty())
  {
      Serial.print("rover control: \t");
      Serial.println(rxval.c_str());

    //Calculating distances for the U-S sensor
   d_f = cal_dist(trigPin_f,echoPin_f);
   d_r = cal_dist(trigPin_r,echoPin_r);
   d_l = cal_dist(trigPin_l,echoPin_l);

   if(d_f > 10 && d_r > 10 && d_l > 10)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(32, 15);
    // Display static text
    display.println("CLEAR");
    display.display(); 
  }
  else
  {
    if(d_f < 10)
    {
      print_on_OLED("OBJECT_FW");
      buzz(800);
      stopMotors();
    }
    if(d_r < 10)
    {
      print_on_OLED("OBJECT_RT");
      buzz(800);
      stopMotors();
    }
    if(d_l < 10)
    {
      print_on_OLED("OBJECT_LT");
      buzz(800);
      stopMotors();
    }
  }
  
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

    if(!rxval3.empty())
    {
      Serial.print("arm mov control \t");
      Serial.println(rxval3.c_str());
    }
} // End of loop
