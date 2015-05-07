#include <Wire.h>
#include "ITG3200.h"
#include "MMA7660.h" 
#include <SoftwareSerial.h>   //Software Serial Port

#define RxD 7
#define TxD 6
 
SoftwareSerial blueToothSerial(RxD,TxD);

ITG3200 gyro;
MMA7660 accelemeter;

//-----
void slaveSetup(){
  
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  blueToothSerial.print("\r\n+STNA=slave3\r\n"); //set the bluetooth name as "slave"
  blueToothSerial.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STPIN=3333\r\n"); //Set SLAVE pincode"0000"
  delay(2000); // This delay is required.
  Serial.println("Starting INQ in 1 sec");
  delay(1000);
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  Serial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  blueToothSerial.flush();
}

//------------- 

float gyroX,gyroY,gyroZ, accX, accY, accZ;
unsigned char GX, GY, GZ, AX, AY, AZ, SOP, ID, CHK;

void setup(){ 
  //Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  slaveSetup(); 
  gyro.init();
  gyro.zeroCalibrate(200,10);
  accelemeter.init();
  gyroX = gyroY = gyroZ = accX = accY = accZ = 0;
  GX = GY = GZ = AX = AY = AZ = CHK = 0;
  SOP = 0xFF; // Start of packet
  ID = 3; // Arduino ID NR 
}
 
void loop() { 
  
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
  
  accelemeter.getAcceleration(&accX,&accY,&accZ);
 
  accX = int((accX / 1.52 + 1) * 127);
    AX = char(accX);
  accY = int((accY / 1.52 + 1) * 127);
    AY = char(accY);
  accZ = int((accZ / 1.52 + 1) * 127);
    AZ = char(accZ);
     // SOP, ID, VALUEx6, CHK  
   unsigned char s[9] = {SOP, ID, GX, GY, GZ, AX, AY, AZ, CHK};
   CHK = 0;
   for(int i = 0; i < 6; i++){
      CHK += s[i+2];
     }
   s[8] = CHK; 

   blueToothSerial.write(s, 9);
   delay(10);
} 
  
 


