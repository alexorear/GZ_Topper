// NOTICE: This code is for use with GZ_LOGIC_Board Rev. 1A and GZ_Node_Board Rev. 2
#include "MonsterMonitorInsert.h"
#include "CityInsert.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_TiCoServo.h>

#include "Mux.h"
using namespace admux;

#define NEOPIXEL_PIN            10 // Pin number connected to the data input of NeoPixels
#define NUM_PIXELS     180
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Multiplexer setup
Mux mux(Pin(A4, INPUT, PinType::Analog), Pinset(A0, A1, A2, A3));

// Monster Inserts
MonsterMonitorInsert tankInsert = MonsterMonitorInsert(4, 3);
MonsterMonitorInsert kaijuInsert = MonsterMonitorInsert(5, 5);
MonsterMonitorInsert bridgeInsert = MonsterMonitorInsert(6, 7);
MonsterMonitorInsert powerlineInsert = MonsterMonitorInsert(7, 11);

//City Inserts
CityInsert paris = CityInsert(0, 600);
CityInsert tokyo = CityInsert(1, 600);
CityInsert ny = CityInsert(2, 600);
CityInsert london = CityInsert(3, 600);

CityInsert extraInsert = CityInsert(12, 600);

//heatRay, activated by middle spinner
CityInsert heatRay = CityInsert(15, 300);

//GI - used for all fire animation trigger
CityInsert redGI = CityInsert(10, 2000, 600);
CityInsert whiteGI = CityInsert(11, 0, 200);
bool destructionAnimationOn = false;

unsigned long gameInPlayTime = 0;
bool gameInPlay = false;

//  rodan insert
CityInsert rodan = CityInsert(8, 5000);
#define NUM_ANIMATION  15

// saucerAttack
// CityInsert saucer = CityInsert(9, 600, 800);
CityInsert saucerAttack = CityInsert(9, 1400, 1200);
#define SERVO_PIN    9
#define SERVO_MIN 1000 // 1 ms pulse
#define SERVO_MAX 2000 // 2 ms pulse
Adafruit_TiCoServo servo;
bool ufoUpPosition = false;

//white GI strip bottom
bool bottomWhiteStripOn = false;
bool middleWhiteStripOn = false;

// strip fire
unsigned long previousStripFire = 0;

unsigned long previousMillis = 0;
int ledState = LOW;

void setup() {
    Serial.begin(9600);
    // ufo setup
    servo.attach(SERVO_PIN, SERVO_MIN, SERVO_MAX);

    pinMode(2, OUTPUT);
    pinMode(4, OUTPUT);

    // RGB setup
    neopixel.begin();
    neopixel.clear();
    neopixel.show();
}

void loop() {
  tankInsert.update(mux);
  kaijuInsert.update(mux);
  bridgeInsert.update(mux);
  powerlineInsert.update(mux);

  updateGameInPlayStatus();

  //additional inputs for mux 13 and 15
  if(extraInsert.getStatus(mux) == cityInsertStatus::ON) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }

  if(mux.read(13) > 800) {
    digitalWrite(4, HIGH);
  } else {
    digitalWrite(4, LOW);
  }

  if (bottomWhiteStripOn == false && 
    rodan.getStatus(mux) == cityInsertStatus::OFF && 
    allMonsterMonitorsLit() == false &&
    !(redGI.getStatus(mux) == cityInsertStatus::ON && whiteGI.getStatus(mux) == cityInsertStatus::OFF) &&
    !destructionAnimationOn) {
    // white on bottom strip only
    illuminateBottomStripWhite();
  }

  if (redGI.getStatus(mux) == cityInsertStatus::ON && whiteGI.getStatus(mux) == cityInsertStatus::OFF) {  
    // allFire on for Destruction Jackpot
    neoPixelFireDisplayAll();
    if (millis() - previousMillis >= 15) {
        previousMillis = millis();
        pingPongFireAnimation();
        neopixel.show(); // Update the LED strip
    }
    setAllFirePanelsOn();
    bottomWhiteStripOn = false;
    middleWhiteStripOn = false;
  } else if (whiteGI.getStatus(mux) == cityInsertStatus::ON && destructionAnimationOn == true) {
    neoPixelFireDisplayAll();
    if (millis() - previousMillis >= 15) {
        previousMillis = millis();
        pingPongFireAnimation();
        neopixel.show(); // Update the LED strip
    }
    setAllFirePanelsOn();
    bottomWhiteStripOn = false;
    middleWhiteStripOn = false;
  } else if(allMonsterMonitorsLit() == true) {
    middleWhiteStripOn = false;
    bottomWhiteStripOn = false;
    setAllFirePanelsOn();
    rainbow();
  } else if (heatRay.getStatus(mux) == cityInsertStatus::ON) {
    middleWhiteStripOn = false;
    setAllFirePanelsOn();
    // heatRay light show
    if (millis() - previousMillis >= 100) {
      // save the last time you blinked the LED
      previousMillis = millis();
      if (ledState == LOW) {
        ledState = HIGH;

        for(int i = 72; i < neopixel.numPixels(); i++) {
          neopixel.setPixelColor(i, 0, 0, 0);
        }
      } else {
        ledState = LOW;
        for(int i = 72; i < neopixel.numPixels(); i++) {
          neopixel.setPixelColor(i, 13, 35, 100);
        }
      }
      // neoPixelBlueFireDisplayAll();


      //set bottom strip to white
      neopixel.fill(neopixel.Color(255, 200, 150), 0, 72);
      bottomWhiteStripOn = true;
      neopixel.show();
    }
  } else {
    //Rodan
    if(rodan.getStatus(mux) == cityInsertStatus::ON) {
      // Rodan
      if (millis() - previousMillis >= 15) {
        previousMillis = millis();
        pingPongAnimation();
        neopixel.show(); // Update the LED strip
      }
    } else if (middleWhiteStripOn == false) {
      illuminateMiddleStripWhite();
    }
      
    if(ny.getStatus(mux) == cityInsertStatus::ON && heatRay.getStatus(mux) == cityInsertStatus::OFF) {
      ny.lastFireMillis = neoPixelFireDisplay(100, 144, 153, ny);
      ny.setTopperLightsActive(true);
    } else if (heatRay.getStatus(mux) == cityInsertStatus::OFF && ny.topperLightsActive() == true) {
      neoPixelFireOff(144, 153);
      ny.setTopperLightsActive(false);
    };
    
    if(london.getStatus(mux) == cityInsertStatus::ON && heatRay.getStatus(mux) == cityInsertStatus::OFF) {
      london.lastFireMillis = neoPixelFireDisplay(100, 153, 162, london);
      london.setTopperLightsActive(true);
    } else if (heatRay.getStatus(mux) == cityInsertStatus::OFF && london.topperLightsActive() == true) {
      neoPixelFireOff(153, 162);
      london.setTopperLightsActive(false);
    };

    if(paris.getStatus(mux) == cityInsertStatus::ON && heatRay.getStatus(mux) == cityInsertStatus::OFF) {
      paris.lastFireMillis = neoPixelFireDisplay(100, 162, 171, paris);
      paris.setTopperLightsActive(true);
    } else if (heatRay.getStatus(mux) == cityInsertStatus::OFF && paris.topperLightsActive() == true) {
      neoPixelFireOff(162, 171);
      paris.setTopperLightsActive(false);
    };

    if(tokyo.getStatus(mux) == cityInsertStatus::ON && heatRay.getStatus(mux) == cityInsertStatus::OFF) {
      tokyo.lastFireMillis = neoPixelFireDisplay(100, 171, 180, tokyo);
      tokyo.setTopperLightsActive(true);
    } else if (heatRay.getStatus(mux) == cityInsertStatus::OFF && tokyo.topperLightsActive() == true) {
      neoPixelFireOff(171, 180);
      tokyo.setTopperLightsActive(false);
    };
  }

// tokyo check is to keep saucer from only firing outside of gameplay
//   if (gameInPlay == true) {
//     if ((saucer.getStatus(mux) == cityInsertStatus::ON) && !ufoUpPosition) {
//       servo.write(2400);
//       ufoUpPosition = true;
//     } else if (saucer.getStatus(mux) == cityInsertStatus::OFF && saucerAttack.getStatus(mux) == cityInsertStatus::OFF) {
//       servo.write(600);
//       ufoUpPosition = false;
//     }
//   } else if (saucer.getStatus(mux) == cityInsertStatus::OFF && saucerAttack.getStatus(mux) == cityInsertStatus::OFF) {
//     servo.write(600);
//     ufoUpPosition = false;
//   }
// }

if (gameInPlay == true) {
    if ((saucerAttack.getStatus(mux) == cityInsertStatus::ON) && !ufoUpPosition) {
      servo.write(2400);
      ufoUpPosition = true;
    } else if (saucerAttack.getStatus(mux) == cityInsertStatus::OFF && ufoUpPosition == true) {
      servo.write(600);
      ufoUpPosition = false;
    }
  } else if (saucerAttack.getStatus(mux) == cityInsertStatus::OFF && ufoUpPosition == true) {
    servo.write(600);
    ufoUpPosition = false;
  }
}

void illuminateMiddleStripWhite() {
  neopixel.fill(neopixel.Color(75, 44, 36), 72, 72); // starts at light 72 and has a count of 72, which would end on light 144
  neopixel.show();
  middleWhiteStripOn = true;
}

void illuminateBottomStripWhite() {
  neopixel.fill(neopixel.Color(225, 132, 108), 0, 72); // starts at light 0 and has a count of 72, which would end on light 72
  neopixel.show();
  bottomWhiteStripOn = true;
}

unsigned long neoPixelFireDisplay(int delay, int firstPixel, int lastPixel, CityInsert insert) {
  if (millis() - insert.lastFireMillis > delay) {
    int r = 255, g = 121, b = 35;

    for(int i = firstPixel; i < lastPixel; i++) {
      int flicker = random(15,130);
      int r1 = r-flicker;
      int g1 = g-flicker;
      int b1 = b-flicker;
      if(g1<0) g1=0;
      if(r1<0) r1=0;
      if(b1<0) b1=0;
      neopixel.setPixelColor(i,r1,g1, b1);
    }
    neopixel.show();
    return millis();
  }
  return insert.lastFireMillis;
}

unsigned long neoPixelFireDisplayAll() {
  if (millis() - previousStripFire > 100) {
    int r = 255, g = 121, b = 35;

    for(int i = 144; i < neopixel.numPixels(); i++) {
      int flicker = random(15,130);
      int r1 = r-flicker;
      int g1 = g-flicker;
      int b1 = b-flicker;
      if(g1<0) g1=0;
      if(r1<0) r1=0;
      if(b1<0) b1=0;
      neopixel.setPixelColor(i,r1,g1, b1);
    }
    neopixel.show();
    previousStripFire = millis();
  }
}

unsigned long neoPixelBlueFireDisplayAll() {
  if (millis() - previousStripFire > 100) {
    int r = 100, g = 150, b = 255;

    for(int i = 72; i < neopixel.numPixels(); i++) {
      int flicker = random(15,130);
      int r1 = r-flicker;
      int g1 = g-flicker;
      int b1 = b-flicker;
      if(g1<0) g1=0;
      if(r1<0) r1=0;
      if(b1<0) b1=0;
      neopixel.setPixelColor(i,r1,g1, b1);
    }
    neopixel.fill(neopixel.Color(255, 200, 150), 0, 72);
    bottomWhiteStripOn = true;
    neopixel.show();
    // neopixel.show();
    previousStripFire = millis();
  }
}

void neoPixelFireOff(int firstPixel, int lastPixel) {
  for(int i = firstPixel; i < lastPixel; i++) {
    neopixel.setPixelColor(i, 0, 0, 0);
  }
  neopixel.show();
}

void rainbow() {
  static unsigned long previousRainbow = 0;
  static uint16_t rainbowJ = 0;
  static uint16_t rainbowI = 0;

  if (millis() - previousRainbow >= 150) {
    // save the last time you blinked the LED
    previousRainbow = millis();
    if (rainbowI == 0) {
      //blue
      for(int i = 0; i < neopixel.numPixels(); i++) {
        neopixel.setPixelColor(i, 25, 25, 255);
      }
      rainbowI = 1;
    } else if (rainbowI == 1 ) {
      // pink
      for(int i = 0; i < neopixel.numPixels(); i++) {
        neopixel.setPixelColor(i, 255, 20, 147);
      }
      rainbowI = 2;
    } else if (rainbowI == 2) {
      // yellow
      for(int i = 0; i < neopixel.numPixels(); i++) {
        neopixel.setPixelColor(i, 255, 239, 0);
      }
      rainbowI = 3;
    } else if (rainbowI == 3) {
      for(int i = 0; i < neopixel.numPixels(); i++) {
        neopixel.setPixelColor(i, 0, 255, 0);
      }
      rainbowI = 0;
    }
    previousRainbow = millis();
    neopixel.show();
  }
}

void setAllFirePanelsOn() {
  paris.setTopperLightsActive(true);
  ny.setTopperLightsActive(true);
  london.setTopperLightsActive(true);
  tokyo.setTopperLightsActive(true);
}

void pingPongAnimation() {
  static int animationPos = 0;
  static int animationDirection = 1; // 1 for moving right, -1 for moving left

  if (animationPos != rodan.getAnimationPos() || rodan.getAnimationPos() == -1) {
    animationPos = 0;
    animationDirection = 1;
    rodan.updateAnimationPos(0);
  }

  // Check if either the bottom or middle strip is illuminated white
  if (bottomWhiteStripOn == true || middleWhiteStripOn == true) {
    for (int i = 0; i < 144; i++) {
      neopixel.setPixelColor(i, 0, 0, 0);
    }
    neopixel.show();
    bottomWhiteStripOn = false;
    middleWhiteStripOn = false;
  }
  // Clear the previous frame
  for (int i = 0; i < NUM_ANIMATION; i++) {
    neopixel.setPixelColor(animationPos + i, 0); // Set pixel to black (off)
    neopixel.setPixelColor(144 - 1 - (animationPos + i), 0); // Set pixel in the mirrored position to black
  }
  
  // Update animation position
  animationPos += animationDirection;
  
  // Check if animation has reached the ends
  if (animationPos <= 0 || animationPos >= 144 - NUM_ANIMATION) {
    animationDirection *= -1; // Reverse the animation direction
  } 

  // Set the new frame
  for (int i = 0; i < NUM_ANIMATION; i++) {
    neopixel.setPixelColor(animationPos + i, neopixel.Color(128, 0, 128)); // Set pixel color to purple
    neopixel.setPixelColor(144 - 1 - (animationPos + i), neopixel.Color(128, 0, 128)); // Set pixel in the mirrored position to purple
  }
  rodan.updateAnimationPos(animationPos);
}

void pingPongFireAnimation() {
  static int animationPos = 0;
  static int animationDirection = 1; // 1 for moving right, -1 for moving left

  destructionAnimationOn = true;
  if (animationPos != redGI.getAnimationPos() || redGI.getAnimationPos() == -1) {
    animationPos = 0;
    animationDirection = 1;
    redGI.updateAnimationPos(0);
  }

  // Check if either the bottom or middle strip is illuminated white
  if (bottomWhiteStripOn == true || middleWhiteStripOn == true) {
    for (int i = 0; i < 144; i++) {
      neopixel.setPixelColor(i, 0, 0, 0);
    }
    neopixel.show();
    bottomWhiteStripOn = false;
    middleWhiteStripOn = false;
  }
  // Clear the previous frame
  for (int i = 0; i < NUM_ANIMATION; i++) {
    neopixel.setPixelColor(animationPos + i, 0); // Set pixel to black (off)
    neopixel.setPixelColor(144 - 1 - (animationPos + i), 0); // Set pixel in the mirrored position to black
  }
  
  // Update animation position
  animationPos += animationDirection;
  
  // Check if animation has reached the ends
  if (animationPos <= 0 || animationPos >= 144 - NUM_ANIMATION) {
    animationDirection *= -1; // Reverse the animation direction
    // destructionAnimationOn = false;
  } 

  if (animationPos <= 0 || animationPos == 144 - NUM_ANIMATION || animationPos == 72 - (NUM_ANIMATION / 2)) {
    destructionAnimationOn = false;
  }

  // Set the new frame
  for (int i = 0; i < NUM_ANIMATION; i++) {
    neopixel.setPixelColor(animationPos + i, neopixel.Color(255, 0, 0)); // Set pixel color to purple
    neopixel.setPixelColor(144 - 1 - (animationPos + i), neopixel.Color(255, 0, 0)); // Set pixel in the mirrored position to purple
  }
  redGI.updateAnimationPos(animationPos);
}

bool allMonsterMonitorsLit() {
 return (tankInsert.getState() == insertState::ON &&
    kaijuInsert.getState() == insertState::ON && 
    bridgeInsert.getState() == insertState::ON && 
    powerlineInsert.getState() == insertState::ON);
}

void fadeRangeOfPixels(uint16_t startPixel, uint16_t endPixel, uint32_t startColor, uint32_t endColor, uint32_t duration) {
  static uint32_t startMillis; // Static variable to store the start time

  uint32_t currentMillis = millis();

  if (currentMillis - startMillis >= duration) {
    // Swap start and end colors and reset the timer
    startColor = endColor;
    endColor = neopixel.Color(0, 0, 0);
    startMillis = currentMillis;
  }

  for (int i = startPixel; i <= endPixel; i++) {
    uint32_t currentColor = getCurrentColor(startColor, endColor, startMillis, duration);
    neopixel.setPixelColor(i, currentColor);
  }

  neopixel.show();
}

uint32_t getCurrentColor(uint32_t startColor, uint32_t endColor, uint32_t startTime, int fadeDuration) {
  uint32_t currentTime = millis() - startTime;

  if (currentTime >= fadeDuration) {
    return endColor;
  } else {
    int progress = map(currentTime, 0, fadeDuration, 0, 100);
    int r = ((startColor >> 16) & 0xFF) + progress * ((endColor >> 16) & 0xFF - (startColor >> 16) & 0xFF) / 100;
    int g = ((startColor >> 8) & 0xFF) + progress * ((endColor >> 8) & 0xFF - (startColor >> 8) & 0xFF) / 100;
    int b = (startColor & 0xFF) + progress * ((endColor) & 0xFF - (startColor & 0xFF)) / 100;
    return neopixel.Color(r, g, b);
  }
}

void updateGameInPlayStatus() {
  if (tokyo.getStatus(mux) == cityInsertStatus::OFF) {
    // Serial.println("tokyo off");
    gameInPlayTime = 0;
    gameInPlay = false;
  }

  if (gameInPlayTime == 0 && tokyo.getStatus(mux) == cityInsertStatus::ON) {
    // Serial.println("tokyo on");
    gameInPlayTime = millis();
    // Serial.println(gameInPlayTime);
  }

  if (gameInPlayTime != 0 && (millis() - gameInPlayTime) >= 6000) {
    if (tokyo.getStatus(mux) == cityInsertStatus::ON) {
      gameInPlay = true;
      // Serial.println(true);
    }
  }
}