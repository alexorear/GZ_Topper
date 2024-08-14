// TFT_eSPI_memory
//
// Current sketch which shows how to display an
// animated GIF image stored in FLASH memory
//
// Adapted from sketch originally written by Larry Bank
// bitbank@pobox.com
//
// Utilized Adapted TFT_eSPI Arduino library by Bodmer:
// https://github.com/Bodmer/TFT_eSPI
//
// To display a GIF from memory, a single callback function
// must be provided - GIFDRAW
// This function is called after each scan line is decoded
// and is passed the 8-bit pixels, RGB565 palette and info
// about how and where to display the line. The palette entries
// can be in little-endian or big-endian order; this is specified
// in the begin() method.
//
// The AnimatedGIF class doesn't allocate or free any memory, but the
// instance data occupies about 22.5K of RAM.

#define USE_DMA       // ESP32 ~1.25x single frame rendering performance boost for badgers.h
                        // Note: Do not use SPI DMA if reading GIF images from SPI SD card on same bus as TFT
#define NORMAL_SPEED  // Comment out for rame rate for render speed test

bool displayMM_ON_Animation = true;

// Load GIF library
#include <AnimatedGIF.h>
AnimatedGIF gif;

// used on all screens
#include "static_loop.h"
#include "heat_ray_loop.h"
#include "dest_jp.h"
#define HEAT_RAY_LOOP heat_ray_loop
#define STATIC_LOOP static_loop
#define DEST_JP dest_jp_1

// screen 1
// #include "mm_1_tank_loop.h"
// #include "mm_1_tank_on.h"
// #define MM_ON mm_1_tank_on
// #define MM_LOOP mm_1_tank_loop


// screen 2
// #include "mm_2_raid_on.h"
// #include "mm_2_raid_loop.h"
// #define MM_ON mm_2_raid_on
// #define MM_LOOP mm_2_raid_loop


// screen 3
// #include "mm_3_bridge_on.h"
// #include "mm_3_bridge_loop.h"
// #define MM_ON mm_3_bridge_on
// #define MM_LOOP mm_3_bridge_loop


// screen 4
#include "mm_4_power_on.h"
#include "mm_4_power_loop.h"  
#define MM_ON mm_4_power_on
#define MM_LOOP mm_4_power_loop

// ESP32 40MHz SPI single frame rendering performance
// Note: no DMA performance gain on smaller images or transparent pixel GIFs
//  No DMA  63 fps, DMA:  71fps
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  pinMode(13, INPUT_PULLUP);
  pinMode(27, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  tft.begin();
#ifdef USE_DMA
  tft.initDMA();
#endif
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  gif.begin(BIG_ENDIAN_PIXELS);
}

#ifdef NORMAL_SPEED // Render at rate that is GIF controlled
void loop() {
  // Put your main code here, to run repeatedly:
  if(digitalRead(27) == LOW) {
    if (gif.open((uint8_t *)DEST_JP, sizeof(DEST_JP), GIFDraw)) {
        Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
        tft.startWrite(); // The TFT chip slect is locked low
        while (gif.playFrame(true, NULL))
        {
          yield();
        }
        gif.close();
        tft.endWrite(); // Release TFT chip select for other SPI devices
      }
  } else if(digitalRead(14) == LOW) {
    displayMM_ON_Animation == true;

    if (gif.open((uint8_t *)HEAT_RAY_LOOP, sizeof(HEAT_RAY_LOOP), GIFDraw)) {
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
      tft.startWrite(); // The TFT chip slect is locked low
      while (gif.playFrame(true, NULL))
      {
        yield();
      }
      gif.close();
      tft.endWrite(); // Release TFT chip select for other SPI devices
    }
  } else if (digitalRead(13) == LOW) {
    if (displayMM_ON_Animation == true) {
      if (gif.open((uint8_t *)MM_ON, sizeof(MM_ON), GIFDraw)) {
        Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
        tft.startWrite(); // The TFT chip slect is locked low
        while (gif.playFrame(true, NULL))
        {
          yield();
        }
        gif.close();
        tft.endWrite(); // Release TFT chip select for other SPI devices
      }
    }

    displayMM_ON_Animation = false;

    if (gif.open((uint8_t *)MM_LOOP, sizeof(MM_LOOP), GIFDraw)) {
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
      tft.startWrite(); // The TFT chip slect is locked low
      while (gif.playFrame(true, NULL))
      {
        yield();
      }
      gif.close();
      tft.endWrite(); // Release TFT chip select for other SPI devices
    }
  } else {
    displayMM_ON_Animation == true;

    if (gif.open((uint8_t *)STATIC_LOOP, sizeof(STATIC_LOOP), GIFDraw)) {
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
      tft.startWrite(); // The TFT chip slect is locked low
      while (gif.playFrame(true, NULL))
      {
        yield();
      }
      gif.close();
      tft.endWrite(); // Release TFT chip select for other SPI devices
    }
  }
  
}
#else // Test maximum rendering speed
// void loop()
// {
//   long lTime = micros();
//   int iFrames = 0;

// if (gif.open((uint8_t *)STATIC_LOOP, sizeof(STATIC_LOOP), GIFDraw))
//   {
//     tft.startWrite(); // For DMA the TFT chip slect is locked low
//     while (gif.playFrame(false, NULL))
//     {
//       // Each loop renders one frame
//       iFrames++;
//       yield();
//     }
//     gif.close();    
//     tft.endWrite(); // Release TFT chip select for other SPI devices
//     lTime = micros() - lTime;
//   }

//   if (gif.open((uint8_t *)MM_ON, sizeof(MM_ON), GIFDraw))
//   {
//     tft.startWrite(); // For DMA the TFT chip slect is locked low
//     while (gif.playFrame(false, NULL))
//     {
//       // Each loop renders one frame
//       iFrames++;
//       yield();
//     }
//     gif.close();    
//     tft.endWrite(); // Release TFT chip select for other SPI devices
//     lTime = micros() - lTime;
//   }

//   if (gif.open((uint8_t *)MM_LOOP, sizeof(MM_LOOP), GIFDraw))
//   {
//     tft.startWrite(); // For DMA the TFT chip slect is locked low
//     while (gif.playFrame(false, NULL))
//     {
//       // Each loop renders one frame
//       iFrames++;
//       yield();
//     }
//     gif.close();    
//     tft.endWrite(); // Release TFT chip select for other SPI devices
//     lTime = micros() - lTime;
//   }

//   if (gif.open((uint8_t *)HEAT_RAY_LOOP, sizeof(HEAT_RAY_LOOP), GIFDraw))
//   {
//     tft.startWrite(); // For DMA the TFT chip slect is locked low
//     while (gif.playFrame(false, NULL))
//     {
//       // Each loop renders one frame
//       iFrames++;
//       yield();
//     }
//     gif.close();    
//     tft.endWrite(); // Release TFT chip select for other SPI devices
//     lTime = micros() - lTime;
//   }
// }
#endif

