void checkSerial(){
  unsigned char dtr;
  int rd, wr, n;
  char buffer[80];
  // check if any data has arrived on the hardware serial port
  rd = HWSERIAL.available();
  if (rd > 0) {
      // read data from the hardware serial port
      n = HWSERIAL.readBytes((char *)buffer, 1);
      if (buffer[0] == 'a'){
        Serial.println("Particle sync");
        HWSERIAL.print(dataPacket);
        HWSERIAL.flush();
      }
  }
}

// dataPacket should look like this for AWS
//  String data = "{\"dt\":\"20160117T160000\",\"rms\":145,\"w\":8}";
void packData(){
  float spectrumLevel;
  int iSpectrumLevel;
    dataPacket = "{\"dt\":";
    dataPacket += t;
 
    for (int i=0; i<4; i++){
          if(meanBand[i]>0.00001){
            spectrumLevel = 20*log10(meanBand[i] / fftCount) - (10 * log10(binwidth)); 
          }
          else{
            spectrumLevel = -96 - (10 * log10(binwidth));
           }
          spectrumLevel = spectrumLevel - hydroCal - gainDb;
          iSpectrumLevel = (int) spectrumLevel;
          dataPacket += ",\"";
          dataPacket += i;
          dataPacket += "\":";
          dataPacket += iSpectrumLevel;
     }
     dataPacket += ",\"w\":";
     dataPacket += whistleCount;
     dataPacket += "}";

     if(printDiags) Serial.println(dataPacket);
}


