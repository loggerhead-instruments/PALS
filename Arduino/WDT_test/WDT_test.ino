void setup() {
  Serial.begin(115200);
  delay(1000); // see if need to start WDT right away
  Serial.println("WDT Test");
    // Setup WDT
  noInterrupts();                                         // don't allow interrupts while setting up WDOG
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;                         // unlock access to WDOG registers
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
  delayMicroseconds(1);                                   // Need to wait a bit..
  
//  // 10 s timeout
//  WDOG_TOVALH = 0x044A;
//  WDOG_TOVALL = 0xA200;

// 30 s timeout
  WDOG_TOVALH = 0x0CDF;
  WDOG_TOVALL = 0xE600;

  
  //
  //  Tick at 7.2 MHz
  WDOG_PRESC  = 0x400;
  
  // Set options to enable WDT. You must always do this as a SINGLE write to WDOG_CTRLH
  WDOG_STCTRLH |= WDOG_STCTRLH_ALLOWUPDATE |
      WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN |
      WDOG_STCTRLH_STOPEN | WDOG_STCTRLH_CLKSRC;
}

void loop() {
  resetWDT();
  for(int32_t x=0; x<1000000; x+=5000){
    Serial.print("Delay: ");
    Serial.println(x);
    delay(x);
    resetWDT();
  }
}

void resetWDT(){
  noInterrupts();  //   reset WDT
  WDOG_REFRESH = 0xA602;
  WDOG_REFRESH = 0xB480;
  interrupts();
}

