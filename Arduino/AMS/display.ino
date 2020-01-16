float mAmpRec = 49;
float mAmpSleep = 3.4;
float mAmpCam = 700;
byte nBatPacks = 4;
float mAhPerBat = 12000.0; // assume 12Ah per battery pack; good batteries should be 14000

csd_t m_csd;

int curMenuItem = 0;
volatile int maxMenuItem = 8;
char *menuItem[] = {"Start",
                     "Record",
                     "Sleep",
                     "Rate",
                     "Gain",
                     "Time",
                     "Mode",
                     "Diel Time"
                     };

char *helpText[] = {"ENTER:Start RecordingUP/DN:scroll menu",
                    "ENTER:Set Record Dur\nUP/DN:scroll menu",
                    "ENTER:Set Sleep Dur\nUP/DN:scroll menu",
                    "ENTER:Set Sample RateUP/DN:scroll menu",
                    "ENTER:Set Gain (dB)\nUP/DN:scroll menu",
                    "ENTER:Set Date/Time\nUP/DN:scroll menu",
                    "ENTER:Set Mode\nUP/DN:scroll menu",
                    "ENTER:Set Diel Time\nUP/DN:scroll menu"
                    };

/* DISPLAY FUNCTIONS
 *  
 */

time_t autoStartTime;
 
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  display.print(":");
  printZero(digits);
  display.print(digits);
}

void printZero(int val){
  if(val<10) display.print('0');
}

#define setStart 0
#define setRecDur 1
#define setRecSleep 2
#define setFsamp 3
#define setGain 4
#define setDateTime 5
#define setMode 6
#define setDielTime 7

void manualSettings(){
  boolean startRec = 0, startUp, startDown;
  readEEPROM();
  calcGain();

  autoStartTime = getTeensy3Time();

// get free space on cards
    cDisplay();
    display.print("LS1 Init");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.println("Card Free/Total MB");
    // Initialize the SD card
    SPI.setMOSI(7);
    SPI.setSCK(14);
    SPI.setMISO(12);
      
    for (int n=0; n<4; n++){
      freeMB[n] = 0; //reset
      Serial.println(); Serial.println();
      Serial.print("Card:"); Serial.println(n + 1);
      display.print(n + 1); display.print("    ");
      display.display();

      if(sd.begin(chipSelect[n])){

        int32_t volFree = sd.vol()->freeClusterCount();
        Serial.print("volFree:");
        Serial.println(volFree);

        float fs = 0.000512 * volFree * sd.vol()->blocksPerCluster();

        uint32_t freeSpace = (uint32_t) fs;
        uint32_t volumeMB = uint32_t ( 0.000512 * (float) sd.card()->cardSize());

        Serial.print("Volume MB ");
        Serial.println(volumeMB);
        
        if (freeSpace < 200) freeMB[n] = 0;
        else
          freeMB[n] = freeSpace - 200; // take off 200 MB to be safe

        Serial.print("Free space (MB): ");
        Serial.println((uint32_t) freeMB[n]);
        
        display.print(freeMB[n]);
        display.print("/");
        display.println(volumeMB);
        display.display();
        delay(1000);
      }
      else{
        Serial.println("Unable to access the SD card");
        //Serial.println(card.errorCode());
       // Serial.println(card.errorData());
        display.println("  None");
        display.display();
    }
  }

  // set back to card 1
  if(!sd.begin(chipSelect[0], SD_SCK_MHZ(50))){
    display.print("Card 1 Fail");
    display.display();
    while(1);
  }

 
  LoadScript(); // secret settings accessible from card 1
  calcGain();
  writeEEPROM(); // update EEPROM in case any settings changed from card

    // make sure settings valid (if EEPROM corrupted or not set yet)
  
  if (rec_dur < 0 | rec_dur>100000) {
    rec_dur = 60;
    writeEEPROMlong(0, rec_dur);  //long
  }
  if (rec_int<0 | rec_int>100000) {
    rec_int = 60;
    writeEEPROMlong(4, rec_int);  //long
  }
  if (startHour<0 | startHour>23) {
    startHour = 0;
    EEPROM.write(8, startHour); //byte
  }
  if (startMinute<0 | startMinute>59) {
    startMinute = 0;
    EEPROM.write(9, startMinute); //byte
  }
  if (endHour<0 | endHour>23) {
    endHour = 0;
    EEPROM.write(10, endHour); //byte
  }
  if (endMinute<0 | endMinute>59) {
    endMinute = 0;
    EEPROM.write(11, endMinute); //byte
  }
  if (recMode<0 | recMode>1) {
    recMode = 0;
    EEPROM.write(12, recMode); //byte
  }
  if (isf<0 | isf>=I_SAMP) {
    isf = I_SAMP; // change 3 to 4 to allow 192 kHz
    EEPROM.write(13, isf); //byte
  }
  if (nBatPacks<0 | nBatPacks>8){
    nBatPacks = 8;
    EEPROM.write(14, nBatPacks); //byte
  }
  if (gainSetting<0 | gainSetting>15) {
    gainSetting = 4;
    EEPROM.write(15, gainSetting); //byte
  }

  // Main Menu Loop
   while(startRec==0){
    static int newYear, newMonth, newDay, newHour, newMinute, newSecond, oldYear, oldMonth, oldDay, oldHour, oldMinute, oldSecond;
    t = getTeensy3Time();
    if (t - autoStartTime > 600) startRec = 1; //autostart if no activity for 10 minutes
    
    
    // Check for button press
    boolean selectVal = digitalRead(UP);
    if(recMode==MODE_DIEL){
      maxMenuItem = 8;
    }
    else{
      maxMenuItem = 7;
    }
    if(selectVal==0){
      while(digitalRead(UP)==0){
        delay(10); // wait until let go
      }
      curMenuItem++;
      if(curMenuItem>=maxMenuItem) curMenuItem = 0;
    }
    
    selectVal = digitalRead(DOWN);
    if(selectVal==0){
      while(digitalRead(DOWN)==0){
        delay(10); // wait until let go
      }
      curMenuItem--;
      if(curMenuItem<0) curMenuItem = maxMenuItem - 1;
    }


    // Enter pressed from main menu
    selectVal = digitalRead(SELECT);
    if(selectVal==0){
      while(digitalRead(SELECT)==0){ // wait until let go of button
        delay(10);
      }

      // Process enter
      switch (curMenuItem){
        case setStart:
            cDisplay();
            writeEEPROM(); //save settings
            display.println("Starting..");
            display.setTextSize(1);
            display.print("Press UP+DN to Stop");
            display.display();
            delay(2000);
            startRec = 1;  //start recording 
            break;
        case setRecDur:
            while(digitalRead(SELECT)==1){
              rec_dur = updateVal(rec_dur, 1, 3600);
              cDisplay();
              display.println("Record:");
              display.print(rec_dur);
              display.println("s");
              displaySettings();
              displayVoltage();
              display.display();
              delay(2);
            }
            while(digitalRead(SELECT)==0); // wait to let go
            curMenuItem = setStart;
            break;
          
        case setRecSleep:
          while(digitalRead(SELECT)==1){
            rec_int = updateVal(rec_int, 0, 3600 * 24);
            cDisplay();
            display.println("Sleep:");
            display.print(rec_int);
            display.println("s");
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go
          curMenuItem = setStart;
          break;

        case setFsamp:
          while(digitalRead(SELECT)==1){
            isf = updateVal(isf, 0, 8);
            cDisplay();
            display.println("Rate");
            display.printf("%.1f kHz",lhi_fsamps[isf]/1000.0f);
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go
          curMenuItem = setStart;
          break;

        case setGain:
          while(digitalRead(SELECT)==1){
            gainSetting = updateVal(gainSetting, 0, 15);
            calcGain();
            cDisplay();
            display.print("Gain:");
            display.println(gainSetting);
            display.print(gainDb);
            display.print("dB");
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go
          curMenuItem = setStart;
          break;
          
        case setDateTime:
          while(digitalRead(SELECT)==1){
            oldYear = year(t);
            newYear = updateVal(oldYear,2000, 2100);
            if(oldYear!=newYear) setTeensyTime(hour(t), minute(t), second(t), day(t), month(t), newYear);
            cDisplay();
            display.println("Year:");
            display.print(year(getTeensy3Time()));
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go
          
          while(digitalRead(SELECT)==1){
            oldMonth = month(t);
            newMonth = updateVal(oldMonth, 1, 12);
            if(oldMonth != newMonth) setTeensyTime(hour(t), minute(t), second(t), day(t), newMonth, year(t));
            cDisplay();
            display.println("Month:");
            display.print(month(getTeensy3Time()));
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go

          while(digitalRead(SELECT)==1){
            oldDay = day(t);
            newDay = updateVal(oldDay, 1, 31);
            if(oldDay!=newDay) setTeensyTime(hour(t), minute(t), second(t), newDay, month(t), year(t));
            cDisplay();
            display.println("Day:");
            display.print(day(getTeensy3Time()));
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go

          while(digitalRead(SELECT)==1){
            oldHour = hour(t);
            newHour = updateVal(oldHour, 0, 23);
            if(oldHour!=newHour) setTeensyTime(newHour, minute(t), second(t), day(t), month(t), year(t));
            cDisplay();
            display.println("Hour:");
            display.print(hour(getTeensy3Time()));
            displaySettings();  
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go

          while(digitalRead(SELECT)==1){
            oldMinute = minute(t);
            newMinute = updateVal(oldMinute, 0, 59);
            if(oldMinute!=newMinute) setTeensyTime(hour(t), newMinute, second(t), day(t), month(t), year(t));
            cDisplay();
            display.println("Minute:");
            display.print(minute(getTeensy3Time()));
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go

          while(digitalRead(SELECT)==1){
            oldSecond = second(t);
            newSecond = updateVal(oldSecond, 0, 59);
            if(oldSecond!=newSecond) setTeensyTime(hour(t), minute(t), newSecond, day(t), month(t), year(t));
            cDisplay();
            display.println("Second:");
            display.print(second(getTeensy3Time()));
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go
          curMenuItem = setStart;
          break;


        case setMode:
          while(digitalRead(SELECT)==1){
            recMode = updateVal(recMode, 0, 1);
            cDisplay();
            display.print("Mode:");
            if (recMode==MODE_NORMAL)  display.print("Norm");
            if (recMode==MODE_DIEL) {
              if(camFlag==0) display.print("Diel");
              else
                display.print("Diel*");
            }
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go
          curMenuItem = setStart;
          break;

        case setDielTime:         
          while(digitalRead(SELECT)==1){
            startHour = updateVal(startHour, 0, 23);
            cDisplay();
            display.println("Strt Hr");
            display.print(startHour);
            displaySettings();  
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go

          while(digitalRead(SELECT)==1){
            startMinute = updateVal(startMinute, 0, 59);
            cDisplay();
            display.println("Strt Min");
            display.print(startMinute);
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go
          
          while(digitalRead(SELECT)==1){
            endHour = updateVal(endHour, 0, 23);
            cDisplay();
            display.println("End Hr");
            display.print(endHour);
            displaySettings();  
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go

          while(digitalRead(SELECT)==1){
            endMinute = updateVal(endMinute, 0, 59);
            cDisplay();
            display.println("End Min");
            display.print(endMinute);
            displaySettings();
            displayVoltage();
            display.display();
            delay(2);
          }
          while(digitalRead(SELECT)==0); // wait to let go

          curMenuItem = setStart;
          break;
       
      }
      
      if (settingsChanged) {
        writeEEPROM();
        settingsChanged = 0;
        autoStartTime = getTeensy3Time();  //reset autoStartTime
      }
    }

  /*
  while(startRec==0){
    static int curSetting = noSet;
    static int newYear, newMonth, newDay, newHour, newMinute, newSecond, oldYear, oldMonth, oldDay, oldHour, oldMinute, oldSecond;
    
    // Check for mode change
    boolean selectVal = digitalRead(SELECT);
    if(selectVal==0){
      curSetting += 1;
      while(digitalRead(SELECT)==0){ // wait until let go of button
        delay(10);
      }
      if((recMode==MODE_NORMAL & curSetting>12) | (recMode==MODE_DIEL & curSetting>16)) curSetting = 0;
   }

    cDisplay();

    t = getTeensy3Time();

    if (t - autoStartTime > 600) startRec = 1; //autostart if no activity for 10 minutes
    switch (curSetting){
      case noSet:
        if (settingsChanged) {
          writeEEPROM();
          settingsChanged = 0;
          autoStartTime = getTeensy3Time();  //reset autoStartTime
        }
        display.print("UP+DN->Rec"); 
        // Check for start recording
        startUp = digitalRead(UP);
        startDown = digitalRead(DOWN);
        if(startUp==0 & startDown==0) {
          cDisplay();
          writeEEPROM(); //save settings
          display.print("Starting..");
          display.display();
          delay(1500);
          startRec = 1;  //start recording
        }
        break;
      case setRecDur:
        rec_dur = updateVal(rec_dur, 1, 21600);
        display.print("Rec:");
        display.print(rec_dur);
        display.println("s");
        break;
      case setRecSleep:
        rec_int = updateVal(rec_int, 0, 3600 * 24);
        display.print("Slp:");
        display.print(rec_int);
        display.println("s");
        break;
      case setGain:
        gainSetting = updateVal(gainSetting, 0, 15);
        calcGain();
        display.print("Gain:");
        display.print(gainSetting);
        break;
      case setYear:
        oldYear = year(t);
        newYear = updateVal(oldYear,2000, 2100);
        if(oldYear!=newYear) setTeensyTime(hour(t), minute(t), second(t), day(t), month(t), newYear);
        display.print("Year:");
        display.print(year(getTeensy3Time()));
        break;
      case setMonth:
        oldMonth = month(t);
        newMonth = updateVal(oldMonth, 1, 12);
        if(oldMonth != newMonth) setTeensyTime(hour(t), minute(t), second(t), day(t), newMonth, year(t));
        display.print("Month:");
        display.print(month(getTeensy3Time()));
        break;
      case setDay:
        oldDay = day(t);
        newDay = updateVal(oldDay, 1, 31);
        if(oldDay!=newDay) setTeensyTime(hour(t), minute(t), second(t), newDay, month(t), year(t));
        display.print("Day:");
        display.print(day(getTeensy3Time()));
        break;
      case setHour:
        oldHour = hour(t);
        newHour = updateVal(oldHour, 0, 23);
        if(oldHour!=newHour) setTeensyTime(newHour, minute(t), second(t), day(t), month(t), year(t));
        display.print("Hour:");
        display.print(hour(getTeensy3Time()));
        break;
      case setMinute:
        oldMinute = minute(t);
        newMinute = updateVal(oldMinute, 0, 59);
        if(oldMinute!=newMinute) setTeensyTime(hour(t), newMinute, second(t), day(t), month(t), year(t));
        display.print("Minute:");
        display.print(minute(getTeensy3Time()));
        break;
      case setSecond:
        oldSecond = second(t);
        newSecond = updateVal(oldSecond, 0, 59);
        if(oldSecond!=newSecond) setTeensyTime(hour(t), minute(t), newSecond, day(t), month(t), year(t));
        display.print("Second:");
        display.print(second(getTeensy3Time()));
        break;
      case setFsamp:
        isf = updateVal(isf, 0, I_SAMP);
        display.printf("SF: %.1f",lhi_fsamps[isf]/1000.0f);
        break;
      case setMode:
        display.print("Mode:");
        recMode = updateVal(recMode, 0, 1);
        if (recMode==MODE_NORMAL)  display.print("Norm");
        if (recMode==MODE_DIEL) {
          if(camFlag==0) display.print("Diel");
          else
            display.print("Diel*");
        }
        break;
      case setBatPack:
        nBatPacks = updateVal(nBatPacks, 1, 8);
        display.print("Batt:");
        display.println(nBatPacks);
        break;
      case setStartHour:
        startHour = updateVal(startHour, 0, 23);
        display.print("Strt HH:");
        printZero(startHour);
        display.print(startHour);
        break;
      case setStartMinute:
        startMinute = updateVal(startMinute, 0, 59);
        display.print("Strt MM:");
        printZero(startMinute);
        display.print(startMinute);
        break;
      case setEndHour:
        endHour = updateVal(endHour, 0, 23);
        display.print("End HH:");
        printZero(endHour);
        display.print(endHour);
        break;
      case setEndMinute:
        endMinute = updateVal(endMinute, 0, 59);
        display.print("End MM:");
        printZero(endMinute);
        display.print(endMinute);
        break;
    }
    */
    
    cDisplay();
    displayMenu();
    displaySettings();
    displayVoltage();
    displayClock(getTeensy3Time(), BOTTOM);
    display.display();
    delay(10);
  }
}

void setTeensyTime(int hr, int mn, int sc, int dy, int mh, int yr){
  tmElements_t tm;
  tm.Year = yr - 1970;
  tm.Month = mh;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = mn;
  tm.Second = sc;
  time_t newtime;
  newtime = makeTime(tm); 
  Teensy3Clock.set(newtime); 
  autoStartTime = getTeensy3Time();
}
  
int updateVal(long curVal, long minVal, long maxVal){
  boolean upVal = digitalRead(UP);
  boolean downVal = digitalRead(DOWN);
  static int heldDown = 0;
  static int heldUp = 0;

  if(upVal==0){
    settingsChanged = 1;
    if (heldUp < 20) delay(200);
      curVal += 1;
      heldUp += 1;
    }
    else heldUp = 0;

    if (heldUp > 100) curVal += 4; //if held up for a while skip an additional 4
    if (heldUp > 200) curVal += 55; //if held up for a while skip an additional 4
    
    if(downVal==0){
      settingsChanged = 1;
      if(heldDown < 20) delay(200);
      if(curVal < 61) { // going down to 0, go back to slow mode
        heldDown = 0;
      }
        curVal -= 1;
        heldDown += 1;
    }
    else heldDown = 0;

    if(heldDown > 100) curVal -= 4;
    if(heldDown > 200) curVal -= 55;

    if (curVal < minVal) curVal = maxVal;
    if (curVal > maxVal) curVal = minVal;
    return curVal;
}

void cDisplay(){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
}

void displaySettings(){
  t = getTeensy3Time();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 38);
  display.print("Rec");
  display.setCursor(25, 38);
  display.print("Slp");
  display.setCursor(50, 38);
  display.print("kHz");
  display.setCursor(75, 38);
  display.print("dB");
  
  display.setCursor(0, 46);
  display.print(rec_dur);
  
  display.setCursor(25, 46);
  display.print(rec_int);

  display.setCursor(50, 46);
  display.printf("%.0f",lhi_fsamps[isf]/1000.0f);

  display.setCursor(75, 46);
  display.printf("%.0f",gainDb);

  uint32_t totalRecSeconds = 0;

  uint32_t fileBytes = (2 * rec_dur * lhi_fsamps[isf]) + 44;
  float fileMB = (fileBytes + 32768) / 1000.0 / 1000.0; // add cluster size so don't underestimate fileMB
  float dielFraction = 1.0; //diel mode decreases time spent recording, increases time in sleep
  if(recMode==MODE_DIEL){
    float dielHours, dielMinutes;
    if(startHour>endHour){
      dielHours = (24 - startHour) + endHour; //  22 to 05 = 7
    }
    else{
      dielHours = endHour - startHour; //e.g. 10 to 12 = 2; 
    }
    if(startMinute > endMinute){
      dielMinutes = (60-startMinute) + endMinute;
    }
    else{
      dielMinutes = endMinute - startMinute;
    }
    dielMinutes += (dielHours * 60);
    dielFraction = dielMinutes / (24.0 * 60.0); // fraction of day recording in diel mode
  }

  float recDraw = mAmpRec + ((float) camFlag * mAmpCam);
  float recFraction = ((float) rec_dur * dielFraction) / (float) (rec_dur + rec_int);
  float sleepFraction = 1 - recFraction;
  float avgCurrentDraw = (recDraw * recFraction) + (mAmpSleep * sleepFraction);
  if(camFlag & recMode==MODE_DIEL){
    float audioOnlyFraction = (1.0-dielFraction) * ((float) rec_dur / (float) (rec_dur + rec_int));
    avgCurrentDraw = (recDraw * recFraction) + (mAmpSleep * sleepFraction) + (mAmpRec * audioOnlyFraction); // this will overestimate because counting sleep twice
//    Serial.print("Audio only fraction: ");
//    Serial.println(audioOnlyFraction);
  }

//  Serial.print("Rec Fraction Sleep Fraction Avg Power:");
//  Serial.print(rec_dur);
//  Serial.print("  ");
//  Serial.print(recFraction);
//  Serial.print("  ");
//  Serial.print(rec_int);
//  Serial.print("  ");
//  Serial.print(sleepFraction);
//  Serial.print("  ");
//  Serial.println(avgCurrentDraw);


  uint32_t powerSeconds = uint32_t (3600.0 * (nBatPacks * mAhPerBat / avgCurrentDraw));

//  Serial.print("fileMB FreeMBCards totalRecSeconds: ");
//  Serial.print(fileMB); Serial.print(" ");

  for(int n=0; n<4; n++){
    filesPerCard[n] = 0;
    if(freeMB[n]==0) filesPerCard[n] = 0;
    else{
      filesPerCard[n] = (uint32_t) floor(freeMB[n] / fileMB);
    }
    
    totalRecSeconds += (filesPerCard[n] * rec_dur);
    float totalCamSeconds = 43200;
//    Serial.print("file bytes:");
//    Serial.print(fileBytes);
//    Serial.print("file MB:");
//    Serial.print(fileMB);
//    Serial.print(" ");
//    Serial.println(filesPerCard[n]);
    //display.setCursor(60, 18 + (n*8));  // display file count for debugging
    //display.print(n+1); display.print(":");display.print(filesPerCard[n]); 
  }
//    Serial.print(" ");
//    Serial.println(totalRecSeconds); Serial.print(" ");
//  Serial.println();

  float totalSecondsMemory = totalRecSeconds / recFraction;
  float totalSecondsCamMemory = 43200 / recFraction;
 
  if(powerSeconds < totalSecondsMemory){
   // displayClock(getTeensy3Time() + powerSeconds, 45, 0);
    display.setCursor(92, 38);
    display.print("Lim B");
    display.setCursor(92, 46);
    display.print((int) powerSeconds / 86400);
    display.print("d");
  }
  else{
  //  displayClock(getTeensy3Time() + totalRecSeconds + totalSleepSeconds, 45, 0);
    display.setCursor(92, 38);
    display.print("Lim B");
    display.setCursor(92, 46);
    display.print((int)totalSecondsMemory / 86400);
    display.print("d");
  }

  display.setCursor(115, BOTTOM);
  if(recMode==MODE_DIEL){
    display.print("D");
  }
  else{
    display.print("N");
  }
}


void displayClock(time_t t, int loc){
  display.setTextSize(1);
  display.setCursor(0,loc);
  display.print(year(t));
  display.print('-');
  display.print(month(t));
  display.print('-');
  display.print(day(t));
  display.print("  ");
  printZero(hour(t));
  display.print(hour(t));
  printDigits(minute(t));
  printDigits(second(t));
}

void printTime(time_t t){
  Serial.print(year(t));
  Serial.print('-');
  Serial.print(month(t));
  Serial.print('-');
  Serial.print(day(t));
  Serial.print(" ");
  Serial.print(hour(t));
  Serial.print(':');
  Serial.print(minute(t));
  Serial.print(':');
  Serial.println(second(t));
}

void readEEPROM(){
  rec_dur = readEEPROMlong(0);
  rec_int = readEEPROMlong(4);
  startHour = EEPROM.read(8);
  startMinute = EEPROM.read(9);
  endHour = EEPROM.read(10);
  endMinute = EEPROM.read(11);
  recMode = EEPROM.read(12);
  isf = EEPROM.read(13);
  byte newBatPacks = EEPROM.read(14);
  if(newBatPacks>0)
  {
    nBatPacks = newBatPacks;
  }
  gainSetting = EEPROM.read(15);
}

union {
  byte b[4];
  long lval;
}u;

long readEEPROMlong(int address){
  u.b[0] = EEPROM.read(address);
  u.b[1] = EEPROM.read(address + 1);
  u.b[2] = EEPROM.read(address + 2);
  u.b[3] = EEPROM.read(address + 3);
  return u.lval;
}

void writeEEPROMlong(int address, long val){
  u.lval = val;
  EEPROM.write(address, u.b[0]);
  EEPROM.write(address + 1, u.b[1]);
  EEPROM.write(address + 2, u.b[2]);
  EEPROM.write(address + 3, u.b[3]);
}

void writeEEPROM(){
  writeEEPROMlong(0, rec_dur);  //long
  writeEEPROMlong(4, rec_int);  //long
  EEPROM.write(8, startHour); //byte
  EEPROM.write(9, startMinute); //byte
  EEPROM.write(10, endHour); //byte
  EEPROM.write(11, endMinute); //byte
  EEPROM.write(12, recMode); //byte
  EEPROM.write(13, isf); //byte
  EEPROM.write(14, nBatPacks); //byte
  EEPROM.write(15, gainSetting); //byte
}

void displayMenu(){
  display.setTextSize(2);
  display.println(menuItem[curMenuItem]);
  display.setTextSize(1);
  display.println(helpText[curMenuItem]);
}

void displayVoltage(){
  display.setTextSize(1);
  display.setCursor(100, 0);
  display.print(readVoltage(),1);
  display.print("V");
}
