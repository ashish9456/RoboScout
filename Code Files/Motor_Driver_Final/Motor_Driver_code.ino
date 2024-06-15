

// Motor 1
const int enA = 12; // Enable pin for Motor 1 (PWM)
const int in1 = 14; // Input 1 of Motor 1
const int in2 = 27; // Input 2 of Motor 1

// Motor 2
const int enB = 13; // Enable pin for Motor 2 (PWM)
const int in3 = 26; // Input 1 of Motor 2
const int in4 = 25; // Input 2 of Motor 2

void setup() {
  // Set motor control pins as outputs
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Initialize serial communication
  Serial.begin(115200);

  // Stop both motors initially
  stopMotors();
}

void loop() {
  // Move forward for 2 seconds
  Serial.println("Forward");
  moveForward();
  delay(2000);

  // Move backward for 2 seconds
  Serial.println("Backward");
  moveBackward();
  delay(2000);

  // Turn left for 2 seconds
  Serial.println("Left");
  turnLeft();
  delay(2000);

  // Turn right for 2 seconds
  Serial.println("Right");
  turnRight();
  delay(2000);
}

void moveForward() {
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
