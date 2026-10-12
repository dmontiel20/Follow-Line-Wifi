#include "PID_Controller.h"
#include "FastLED.h"
#define PIN_RBGLED 4
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];


// Ultrasonics
#define TRIG_PIN 13  
#define ECHO_PIN 12  

// Infrarrojos
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0

// Enable/Disable motor control.
//  HIGH: motor control enabled
//  LOW: motor control disabled
#define PIN_Motor_STBY 3

// Group A Motors (Right Side)
// PIN_Motor_AIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_AIN_1 7
// PIN_Motor_PWMA: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMA 5

// Group B Motors (Left Side)
// PIN_Motor_BIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_BIN_1 8
// PIN_Motor_PWMB: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMB 6


#define MOTOR_SPEED 255
// #define MOTOR_SPEED 100

#define RIGHT 0
#define LEFT 1

# define SENS_THRESHOLD 750

int last;
bool ready = true;
bool slow = false;
bool line = false;
long ping_time;
long start_time;

void setup() {
  // UltraSonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // Infrarrojos
  pinMode(PIN_ITR20001_LEFT, INPUT);
  pinMode(PIN_ITR20001_MIDDLE, INPUT);
  pinMode(PIN_ITR20001_RIGHT, INPUT);

  // Motores
  // enable
  pinMode(PIN_Motor_STBY, OUTPUT);
  // derecha
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);
  // izq
  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);

  Serial.begin(9600);

  digitalWrite(PIN_Motor_STBY, 1);
  digitalWrite(PIN_Motor_AIN_1, 1);
  digitalWrite(PIN_Motor_BIN_1, 1);

  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);

  // Set Red Color to LED
  FastLED.showColor(Color(255, 0, 0));
  delay(500);

  String sendBuff;
  Serial.println("conecting..");
  // To make this code works, remember that the switch S1 should be set to "CAM"
  while(1) {

    if (Serial.available()) {
      char c = Serial.read();
      
      if (c == '}')  {            
        Serial.print("Wifi connected!!\nIP: ");
        Serial.println(sendBuff);

        // Set Red Green to LED
        FastLED.showColor(Color(0, 255, 0));
        sendBuff = "";
        break;
      } 
      else {
        sendBuff += c;
      }

    }
  }
  Serial.println("START$");
  ping_time = millis();
  start_time = ping_time;

}

void loop() {
  float distance;
  bool sl = false, sc = false, sr = false;

  int left = analogRead(PIN_ITR20001_LEFT);
  int center = analogRead(PIN_ITR20001_MIDDLE);
  int right = analogRead(PIN_ITR20001_RIGHT);

  if (left > SENS_THRESHOLD){ sl = true; }
  if (center > SENS_THRESHOLD){ sc = true; }
  if (right > SENS_THRESHOLD){ sr = true; }

  // if (last == RIGHT){
  //   FastLED.showColor(Color(255, 0, 255));
  // } else {
  //   FastLED.showColor(Color(0, 255, 0));
  // } 
  send_ping();
  // Serial.print(sl);
  // Serial.print(" - ");
  // Serial.print(sc);
  // Serial.print(" - ");
  // Serial.print(sr);
  // Serial.println("test}");

  // if (ready){
  if (sc && !sl && !sr){
    // Serial.println("cent: ");
    FastLED.showColor(Color(0, 255, 0));
    line = true;
    if (slow){
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
    }
    else {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
    }
    
  } else if (sc && sl && !sr) {
    FastLED.showColor(Color(0, 255, 0));
    // Serial.println("center left: ");
    line = true;
    if (slow) {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 2);
    }
    else {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 6);
    }
    
    
    last = LEFT;
  } else if (sc && !sl && sr) {
    // Serial.println("center right: ");
    FastLED.showColor(Color(0, 255, 0));
    line = true;
    if (slow) {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 2);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
    }
    else {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 6);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
    }
    last = RIGHT;
  } else if (!sc && sl && !sr) {
    // Serial.println("left: ");
    FastLED.showColor(Color(0, 255, 0));
    line = true;
    // analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 2);
    if (slow) {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 4);
    }
    else {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 2);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 6);
    }
    last = LEFT;
  } else if (!sc && !sl && sr) {
    // Serial.print("right: ");
    FastLED.showColor(Color(0, 255, 0));
    line = true;
    if (slow) {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 4);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
    }
    else {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 6);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 2);
    }
    last = RIGHT;
  } else if (!sc && !sl && !sr && last == LEFT) {
    // Serial.print("none left: ");
    FastLED.showColor(Color(255, 0, 0));
    if (line){
      Serial.println("LINE$");
      line = false;
    }

    if (slow) {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
      // analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 4);
      analogWrite(PIN_Motor_PWMB, 0);
      // analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 4);
    }
    else {
      analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
      analogWrite(PIN_Motor_PWMB, 0);
    }
  } else if (!sc && !sl && !sr && last == RIGHT) {
    // Serial.print("none right: ");
    FastLED.showColor(Color(255, 0, 0));
    if (line){
      Serial.println("LINE$");
      line = false;
    }
    if (slow) {
      // analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 4);
      analogWrite(PIN_Motor_PWMA, 0);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
      // analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 4);
    }
    else {
      analogWrite(PIN_Motor_PWMA, 0);
      analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
    }
  }

  distance = dist();
  if (distance <= 10) {
    ready = false;
    analogWrite(PIN_Motor_PWMA, 0);
    analogWrite(PIN_Motor_PWMB, 0);

    send_obstacle(distance);
    send_end_lap();
  }
  // } 
  // else {
  //   // float d = dist();
  //   // Serial.println(d);
  //   if (d < 5) {
  //     ready = true;
  //     delay(2000);
  //   }
  // }
}

void send_obstacle(float d){
  Serial.print("OBSTACLE#");
  Serial.print(d);
  Serial.println("$");
}

void send_end_lap(){
  long lap_time = millis() - start_time;
  Serial.print("END#");
  Serial.print(lap_time);
  Serial.println("$");
}

void send_ping(){
  if (millis() - ping_time > 4000){
    Serial.println("????");
    Serial.print("PING#");
    Serial.print(millis());
    Serial.print("#");
    ping_time = millis();
  }
}

float dist(){
  long t; //timepo que demora en llegar el eco
  long d; //distancia en centimetros

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(TRIG_PIN, LOW);
  
  t = pulseIn(ECHO_PIN, HIGH); //obtenemos el ancho del pulso
  d = t/59;             //escalamos el tiempo a una distancia en cm

  return d;
}

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}