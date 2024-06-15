#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();      

#define SERVOMIN  70 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  520 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  50 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  600 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

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

// our servo # counter
uint8_t servonum = 0;

void setup() 
{
  // put your setup code here, to run once:
   pwm.begin();
   pwm.setOscillatorFrequency(27000000);
   pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

   delay(10);

}

void loop() 
{
  /*pwm.setPWM(4,0,100);
  delay(2000);
  pwm.setPWM(4,0,200);
  delay(2000);*/
  
  pwm.setPWM(4,0,300);
  delay(1000);




}
