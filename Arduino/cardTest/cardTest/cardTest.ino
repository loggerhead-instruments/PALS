// SD testing

//#include <SerialFlash.h>
//#include <Audio.h>  //this also includes SD.h from lines 89 & 90
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
//#include <SdFat.h>
#include <Snooze.h>  //using https://github.com/duff2013/Snooze; uncomment line 62 #define USE_HIBERNATE
#include <TimeLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
//#include <TimerOne.h>

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define BOTTOM 55

// set this to the hardware serial port you wish to use
#define HWSERIAL Serial1

// LS1 Pins
const int hydroPowPin = 2;
const int UP = 4;
const int DOWN = 3; 
const int SELECT = 8;
const int displayPow = 6;
const int CAM_SW = 5;
const int vSense = 16; 

// microSD chip select pins
#define CS1 10
#define CS2 15
#define CS3 20
#define CS4 21
int chipSelect = CS1;
uint32_t cardList[4];

Sd2Card card;
SdVolume volume;
SdFile root;



void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(hydroPowPin, OUTPUT);
  digitalWrite(displayPow, HIGH);
  Wire.begin();

  pinMode(hydroPowPin, OUTPUT);
  pinMode(displayPow, OUTPUT);
  pinMode(CAM_SW, OUTPUT);

  pinMode(vSense, INPUT);
  analogReference(DEFAULT);

  digitalWrite(hydroPowPin, LOW);
  digitalWrite(displayPow, HIGH);
  digitalWrite(CAM_SW, LOW);

  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);
  pinMode(CS4, OUTPUT);
  digitalWrite(CS1, HIGH);
  digitalWrite(CS2, HIGH);
  digitalWrite(CS3, HIGH);
  digitalWrite(CS4, HIGH);
  
  
  //setup display and controls
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(SELECT, INPUT);
  digitalWrite(UP, HIGH);
  digitalWrite(DOWN, HIGH);
  digitalWrite(SELECT, HIGH);

  delay(500);    

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //initialize display
  cDisplay();
  display.print("LS1 Init");
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.println("Card Free/Total MB");

  for (int n=0; n<4; n++){
    cardList[n] = 0; //reset
    if (n==0) chipSelect = CS1;
    if (n==1) chipSelect = CS2; digitalWrite(CS1, HIGH); 
    if (n==2) chipSelect = CS3; digitalWrite(CS2, HIGH); 
    if (n==3) chipSelect = CS4; digitalWrite(CS3, HIGH); 
    Serial.println(); Serial.println();
    Serial.print("Card:"); Serial.println(n + 1);

    display.print(n + 1); display.print("    ");
    display.display();
    
    // Initialize the SD card
    SPI.setMOSI(7);
    SPI.setSCK(14);
    SPI.setMISO(12);
  
    if(card.init(SPI_FULL_SPEED, chipSelect)){
       if(!volume.init(card)){
      Serial.println("could not find fat partition");
      }
      uint32_t freeSpace;
      cardList[n] = volumeInfo(&freeSpace);//store volume size for good cards
      Serial.print("Free space (MB): ");
      Serial.println(freeSpace);

      display.print(freeSpace);
      display.print("/");
      display.println(cardList[n]);
      display.display();
    }
    else{
      Serial.println("Unable to access the SD card");
      //Serial.println(card.errorCode());
     // Serial.println(card.errorData());
      display.println("  None");
      display.display();
    }
  
    delay(200);
}
  display.print("Press EN");
  display.display();
  // wait here for Enter to be pressed
}

void loop() {
  // put your main code here, to run repeatedly:

}

uint32_t volumeInfo(uint32_t *freeSpace){
  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();
  
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  if (volumesize < 8388608ul) {
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize * 512);        // SD card blocks are always 512 bytes
  }
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 2;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  
  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);
  uint32_t usedSpace = root.ls(LS_R | LS_DATE | LS_SIZE);
  *freeSpace = volumesize - (usedSpace / 1024 / 1024);
  return volumesize;
}

void cDisplay(){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
}

