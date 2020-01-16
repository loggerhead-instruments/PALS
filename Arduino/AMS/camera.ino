void cam_wake() {
  digitalWrite(CAM_SW, HIGH);
  delay(2000); //power on camera (if off)
  digitalWrite(CAM_SW, LOW);     
  CAMON = 1;   
}

void cam_start() {
  digitalWrite(CAM_SW, HIGH);
  delay(200);  // simulate  button press
  digitalWrite(CAM_SW, LOW);      
  CAMON = 2;
}

void cam_stop(){
  digitalWrite(CAM_SW, HIGH);
  delay(200);  // simulate  button press
  digitalWrite(CAM_SW, LOW);  
  delay(100);
  CAMON = 1;   
}

void cam_off() {
  digitalWrite(CAM_SW, HIGH);
  delay(3000);
  digitalWrite(CAM_SW, LOW);     
  CAMON = 0;
}
