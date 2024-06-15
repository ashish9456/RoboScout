#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 39 // OLED display height, in pixels

#define BUZZER_PIN 23

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//defining pins for ultrasonic sensors 
const int trigPin_f = 5;
const int echoPin_f = 18;

const int trigPin_r = 17;
const int echoPin_r = 16;

const int trigPin_l = 0;
const int echoPin_l = 4;

//define sound speed in cm/uS
long int duration;
int distanceCm;

//Initialising distances with 0 value from the U-S sensors
int d_f=0,d_r=0,d_l=0;  // d_f -> distance_front, d_r -> distance_right 
                        // d_l -> distance_left
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

void print_on_OLED(char arr[])
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(12, 15);
  // Display static text
  display.println(arr);
  display.display();
  Serial.println("Buzzer ON");
  buzz(800);
}

void setup() 
{
  Serial.begin(9600); // Starts the serial communication
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  pinMode(trigPin_f, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_f, INPUT); // Sets the echoPin as an Input

  pinMode(trigPin_r, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_r, INPUT); // Sets the echoPin as an Input

  pinMode(trigPin_l, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin_l, INPUT); // Sets the echoPin as an Input

  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() 
{
  display.clearDisplay();
  d_f = cal_dist(trigPin_f,echoPin_f);
  d_r = cal_dist(trigPin_r,echoPin_r);
  d_l = cal_dist(trigPin_l,echoPin_l);

  Serial.println(d_f);
  Serial.println(d_r);
  Serial.println(d_l);
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
    print_on_OLED("OBJECT_FW");
  if(d_r < 10)
    print_on_OLED("OBJECT_RT");
  if(d_l < 10)
    print_on_OLED("OBJECT_LT");
  }
  delay(20);
}