// wired connections
#define HG7881_B_IA 10 // D10 --> Motor B Input A --> MOTOR B +
#define HG7881_B_IB 11 // D11 --> Motor B Input B --> MOTOR B -
 
// functional connections
#define MOTOR_B_PWM HG7881_B_IA // Motor B PWM Speed
#define MOTOR_B_DIR HG7881_B_IB // Motor B Direction

void setup() {
  // put your setup code here, to run once:
  pinMode( MOTOR_B_DIR, OUTPUT );
  pinMode( MOTOR_B_PWM, OUTPUT );
  digitalWrite( MOTOR_B_DIR, LOW );
  digitalWrite( MOTOR_B_PWM, LOW );
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite( MOTOR_B_DIR, LOW );
  analogWrite( MOTOR_B_PWM, 255-200 ); // PWM speed = fast
  delay(1000);
  digitalWrite( MOTOR_B_DIR, HIGH ); // direction = reverse
  analogWrite( MOTOR_B_PWM, 50 ); // PWM speed = slow
  delay(1000);  
}
