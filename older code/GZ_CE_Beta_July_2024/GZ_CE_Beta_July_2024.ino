/*
NOTES: Code includes ability to delete SPIFFS and update gif files in SPIFFS if SD card is present. Gif file names
need to match the file names in the *filenames array.

TODOs:
1. Stop current gif for new gif if new topper status changes. Current behavior waits for current gif to end before starting the next gif. 
  this can cause sync issues if it's a global status change like distruction jackpot or Rodan
2. Check SD card for present files and only delete/update gifs that are on the SD card vs all gifs
3. Display some kind of update status to the users if updating SPIFFs
*/

#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>      // Install this library with the Arduino Library Manager
                           // Don't forget to configure the driver for the display!

#include <AnimatedGIF.h>   // Install this library with the Arduino Library Manager

#define SD_CS_PIN 5 // Chip Select Pin (CS) for SD card Reader

AnimatedGIF gif;
File gifFile;              // Global File object for the GIF file
TFT_eSPI tft = TFT_eSPI(); 

// const char *filename1 = "/dest_jp_1.gif";   // Change to load other gif files in images/GIF
// const char *filename2 = "/static_loop.gip";   // Change to load other gif files in images/GIF

const char *filenames[] = {
  "/00_static.gif",
  "/01_djackpot.gif",
  "/02_saucerup.gif",
  "/03_rodan2x.gif",
  "/04_heatray.gif",
  "/05_monitoron.gif",
};

void setup() {
  Serial.begin(115200);
  pinMode(13, INPUT_PULLUP); // monster monitor
  pinMode(14, INPUT_PULLUP); // heat ray
  pinMode(25, INPUT_PULLUP); // saucer
  pinMode(26, INPUT_PULLUP); // rodan
  pinMode(27, INPUT_PULLUP); // destrustion JP

  // Initialize SD card
  Serial.println("SD card initialization...");
  if (SD.begin(SD_CS_PIN)) {
    // Initialize SPIFFS
    Serial.println("Initialize SPIFFS...");
    if (!SPIFFS.begin(true))
    {
      Serial.println("SPIFFS initialization failed!");
    }

    // Reformmating the SPIFFS to have space if a large GIF is loaded
    // You could run out of SPIFFS storage space if you load more than one image or a large GIF
    // You can also increase the SPIFFS storage space by changing the partition of the ESP32
    //
    Serial.println("Formatting SPIFFS...");
    SPIFFS.format(); // This will erase all the files, change as needed, SPIFFs is non-volatile memory
    Serial.println("SPIFFS formatted successfully.");

    // Open GIF file from SD card
    for (int i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
      Serial.print("Filename ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(filenames[i]);

      Serial.println("Openning GIF file from SD card...");
      File sdFile = SD.open(filenames[i]);
      if (!sdFile)
      {
        Serial.println("Failed to open GIF file from SD card!");
        return;
      }

      // Create a file in SPIFFS to store the GIF
      File spiffsFile = SPIFFS.open(filenames[i], FILE_WRITE, true);
      if (!spiffsFile)
      {
        Serial.println("Failed to copy GIF in SPIFFS!");
        return;
      }

      // Read the GIF from SD card and write to SPIFFS
      Serial.println("Copy GIF in SPIFFS...");
      byte buffer[512];
      while (sdFile.available())
      {
        int bytesRead = sdFile.read(buffer, sizeof(buffer));
        spiffsFile.write(buffer, bytesRead);
      }

      spiffsFile.close();
      sdFile.close();
    }
  } else {
    Serial.println("SD card initialization failed or no SD card inserted!");
    
    // Initialize SPIFFS
    Serial.println("Initialize SPIFFS...");
    if (!SPIFFS.begin(true)) {
      Serial.println("SPIFFS initialization failed!");
    }

    // Print SPIFFS information
    Serial.printf("Total space: %u bytes\n", SPIFFS.totalBytes());
    Serial.printf("Used space: %u bytes\n", SPIFFS.usedBytes());

    uint32_t SPIFFS_freeBytes = (SPIFFS.totalBytes() - SPIFFS.usedBytes());
    Serial.println(SPIFFS_freeBytes);
  }

  // Initialize the GIF
  Serial.println("Starting animation...");

  tft.begin();
  tft.setRotation(3); // Adjust Rotation of your screen (0-3)
  tft.fillScreen(TFT_BLACK);
  gif.begin(BIG_ENDIAN_PIXELS);
}

void loop() {
  if (digitalRead(27) == LOW) {
    playGif("/01_djackpot.gif");
  } else if (digitalRead(25) == LOW) { 
    playGif("/02_saucerup.gif");
  } else if (digitalRead(26) == LOW) {
    playGif("/03_rodan2x.gif");
  }else if (digitalRead(14) == LOW) {
    playGif("/04_heatray.gif");
  } else if (digitalRead(13) == LOW) {
    playGif("/05_monitoron.gif");
  } else {
    playGif("/00_static.gif");
  }
}

void playGif(char * filename) {
  if (gif.open(filename, fileOpen, fileClose, fileRead, fileSeek, GIFDraw)) {
    tft.startWrite(); // The TFT chip slect is locked low
    while (gif.playFrame(true, NULL)) { }
    gif.close();
    tft.endWrite(); // Release TFT chip select for the SD Card Reader
  }
}

// Callbacks for file operations for the Animated GIF Lobrary
void *fileOpen(const char *filename, int32_t *pFileSize)
{
  gifFile = SPIFFS.open(filename, FILE_READ);
  *pFileSize = gifFile.size();
  if (!gifFile)
  {
    Serial.println("Failed to open GIF file from SPIFFS!");
  }
  return &gifFile;
}

void fileClose(void *pHandle)
{
  gifFile.close();
}

int32_t fileRead(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos;
  if (iBytesRead <= 0)
    return 0;

  gifFile.seek(pFile->iPos);
  int32_t bytesRead = gifFile.read(pBuf, iLen);
  pFile->iPos += iBytesRead;

  return bytesRead;
}

int32_t fileSeek(GIFFILE *pFile, int32_t iPosition)
{
  if (iPosition < 0)
    iPosition = 0;
  else if (iPosition >= pFile->iSize)
    iPosition = pFile->iSize - 1;
  pFile->iPos = iPosition;
  gifFile.seek(pFile->iPos);
  return iPosition;
}