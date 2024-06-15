// defining pins for potentiometers and servo motors for the arm ///
const int pot_Pin1 = 15;
const int pot_Pin2 = 2;
int mot_val_1;
int mot_val_2;

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  // reading analog values of potentiometer
  int sensor_v1 = analogRead(pot_Pin1);
  int sensor_v2 = analogRead(pot_Pin2);

  /*mapping analog values of potentiometers to the servo motors*/
  mot_val1 = map(sensor_v1,0,4095,70,520);
  mot_val2 = map(sensor_v2,0,4095,70,520);

   Serial.print("Motor 1 value: ");
   Serial.println(mot_val1);
   Serial.println("Motor 2 value: ")
   Serial.println(mot_val2);
 }
