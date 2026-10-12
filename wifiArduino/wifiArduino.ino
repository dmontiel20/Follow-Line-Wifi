#include "FastLED.h"
#define PIN_RBGLED 4
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void setup() {

  Serial.begin(9600);
  delay(200);

  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);

  // Set Red Color to LED
  FastLED.showColor(Color(255, 0, 0));
  delay(500);

  String sendBuff;

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

}

void loop () {  
  Serial.println("{ 'test': " + String(millis()) + " }");
  delay(300);


}

