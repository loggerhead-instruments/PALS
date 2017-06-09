
void setup(){
    Serial1.begin(115200);
    Serial.begin(115200);
}
void loop() {
    int nowMinute = Time.minute();
    int nowSecond = Time.second();
    if(((nowMinute==0) | (nowMinute==10) | (nowMinute==20) | (nowMinute==30) | (nowMinute==40) | (nowMinute==50)
             | (nowMinute==5) | (nowMinute==15) | (nowMinute==25) | (nowMinute==35) | (nowMinute==45)  | (nowMinute==55)) & (nowSecond==30) & (nowMinute!=lastMinuteSent)){
        lastMinuteSent = nowMinute;
        checkData();
        if(incomingString.length()>0){
          //String data = "{\"dt\":\"20160117T160000\",\"rms\":145,\"w\":8}";
          Particle.publish("data", incomingString);
        }
    }
    // respond to commands from AMS
    char incomingByte;
    while(Serial1.available()){
        incomingByte = Serial1.read();
        Serial.print(incomingByte);
        if(incomingByte=='t'){  //send Unix time in response to a 't'
            Serial.print(Time.now());
            Serial1.print(Time.now());
        }
    }

}

void checkData(){
    // send an 'a' to request data
    Serial1.write('a');
    delay(100);
    incomingString = Serial1.readString();
}
Ready.  morphing-gerbil  v0.4.8