#include <Wire.h>
#include "ITG3200.h"
#include "MMA7660.h"
#include <SoftwareSerial.h>

ITG3200 gyro;
MMA7660 accelemeter;
unsigned char ArduinoNumber = 0;

SoftwareSerial Bi(10, 11); // RX, TX

void setup() {
  Bi.begin(19200);
  gyro.init();
  gyro.zeroCalibrate(200,10);
  accelemeter.init();
}

void loop() {
  float gyroX,gyroY,gyroZ;
  unsigned char GX, GY, GZ, AX, AY, AZ;
  gyro.getAngularVelocity(&gyroX,&gyroY,&gyroZ);
  
  gyroX = int(gyroX) / 20 + 127;
    if(gyroX < 0){ gyroX = 0; }
      else if(gyroX > 255){ gyroX = 255; }
  GX = char(gyroX);
  
  gyroY = int(gyroY) / 20 + 127;
    if(gyroY < 0){ gyroY = 0; }
      else if(gyroY > 255){ gyroY = 255; }
  GY = char(gyroY);
  
  gyroZ = int(gyroZ) / 20 + 127;
    if(gyroZ < 0){ gyroZ = 0; }
      else if(gyroZ > 255){ gyroZ = 255; }
  GZ = char(gyroZ);
  
  float accX,accY,accZ;
  accelemeter.getAcceleration(&accX,&accY,&accZ);
 
  accX = int((accX / 1.52 + 1) * 127);
    AX = char(accX);
  accY = int((accY / 1.52 + 1) * 127);
    AY = char(accY);
  accZ = int((accZ / 1.52 + 1) * 127);
    AZ = char(accZ);
     
   unsigned char s[7] = {ArduinoNumber, GX, GY, GZ, AX, AY, AZ};
     
     Bi.write(s, 7);
  
delay(20);
}
