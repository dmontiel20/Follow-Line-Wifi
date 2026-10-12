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


#define MOTOR_SPEED 125
// #define MOTOR_SPEED 100

#define RIGHT 0
#define LEFT 1

# define SENS_THRESHOLD 750

int last;
bool ready = true;
bool slow = true;
bool line = true;
bool last_line = true;
bool init_control = true;
unsigned long init_time;
unsigned long ping_time;
bool connected = false;

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
  // To make this code works, remember that the switch S1 should be set to "CAM"
  while(1) {

    // To know if Esp32 ists connected to wifi
    if (Serial.available()) {
      char c = Serial.read();
      
      if (c == '}')  {            
        FastLED.showColor(Color(0, 0, 255));
        connected = true;
        delay(1000);
        sendBuff = "";
        break;
      } 
      else {
        sendBuff += c;
      }

    }
  }
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

  if (connected && init_control){
    init_lap();
    init_control = false;
  }

  // For change color led when it is detected line
  if (line){
    FastLED.showColor(Color(0, 255, 0));
  }
  else {
    FastLED.showColor(Color(255, 0, 0));
  }

  if (ready){
    intervals_ping();

    if (sc && !sl && !sr){
      // line = true;
      last_line = true;
      if (!line){
        found_line();
      }
      if (slow){
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
      }
      else {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
      }
      line = true;
      
    } else if (sc && sl && !sr) {
      
      last_line = true;
      if (!line){
        found_line();
      }
      if (slow) {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 2);
      }
      else {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 6);
      }
      line = true;
      last = LEFT;
    } else if (sc && !sl && sr) {
      last_line = true;
      if (!line){
        found_line();
      }
      if (slow) {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 2);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
      }
      else {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 6);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
      }
      line = true;
      last = RIGHT;
    } else if (!sc && sl && !sr) {
      if (!line){
        found_line();
      }
      last_line = true;
      if (slow) {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 4);
      }
      else {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 2);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 6);
      }
      line = true;
      last = LEFT;
    } else if (!sc && !sl && sr) {
      if (!line){
        found_line();
      }
      last_line = true;
      if (slow) {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 4);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED);
      }
      else {
        analogWrite(PIN_Motor_PWMA, MOTOR_SPEED / 6);
        analogWrite(PIN_Motor_PWMB, MOTOR_SPEED / 2);
      }
      line = true;
      last = RIGHT;
    } else if (!sc && !sl && !sr && last == LEFT) {
      if (line){
        lost_line();
      }
      last_line = false;
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
      line = false;
    } else if (!sc && !sl && !sr && last == RIGHT) {
      if (line){
        lost_line();
      }
      last_line = false;
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
      line = false;
    }

    distance = dist();
    if (distance <= 7) {
      ready = false;
      analogWrite(PIN_Motor_PWMA, 0);
      analogWrite(PIN_Motor_PWMB, 0);

      obstacle();  //send obstacle message
      end_lap();   //send end lap messsage
      ready = false;
    }
  }
}

void found_line(){
  Serial.println("F");
}

void lost_line(){
  Serial.println("L");
}

void init_lap(){
  init_time = millis();
  ping_time = init_time;
  ready = true;
  Serial.println("I");
}

void end_lap(){
  unsigned long lap_time = (millis() - init_time);
  Serial.print("E:");
  Serial.print(lap_time);
}

void obstacle(){
  float distancia = dist();
  Serial.print("O:");
  Serial.println(distancia);
}

void intervals_ping(){
  if (millis() - ping_time > 4000){
    Serial.print("P:");
    Serial.print(millis());

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