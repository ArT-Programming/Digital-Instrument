#include <Wire.h>
#include "ITG3200.h"
#include "MMA7660.h" 
#include <SoftwareSerial.h>   //Software Serial Port
#define RxD 7
#define TxD 6
 
SoftwareSerial blueToothSerial(RxD,TxD);

ITG3200 gyro;
MMA7660 accelemeter;
unsigned char ArduinoNumber = 0;
 
void setup() 
{ 
  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  slaveSetup(); 
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
     
   unsigned char s[6] = {GX, GY, GZ, AX, AY, AZ};
  /*for(int i = 0; i < 6; i++){
    Serial.print(s[i]);  
    Serial.print(" "); 
  }Serial.println("");*/
   blueToothSerial.write(s, 6);
   delay(20);
} 
 
void slaveSetup(){
  
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  blueToothSerial.print("\r\n+STNA=slave\r\n"); //set the bluetooth name as "slave"
  blueToothSerial.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STPIN=0000\r\n"); //Set SLAVE pincode"0000"
  delay(2000); // This delay is required.
  Serial.println("Starting INQ in 1 sec");
  delay(1000);
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  blueToothSerial.print("\r\n+RTINQ=aa,bb,cc,dd,ee,ff;slave\r\n");
  Serial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  //blueToothSerial.print("\r\n+RTINQ=aa,bb,cc,dd,ee,ff;slave\r\n");
  blueToothSerial.flush();
}

