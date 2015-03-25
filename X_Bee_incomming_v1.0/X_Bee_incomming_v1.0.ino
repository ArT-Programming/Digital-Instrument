/*
Demo code for ArT Prog3 "Workshop 2 - serial communication" utilizing the Byte ArrgyroY Protocol
Sends data from the Arduino over the serial port to the computer
*/
#include <SoftwareSerial.h>
#include <BAP.h> //Byte ArrgyroY Protocol
#include <Wire.h>
#include "ITG3200.h"
#include "MMA7660.h"

SoftwareSerial mySerial(10, 11); // RX, TX

unsigned char seq = 0; // sequence number

const int arduinoAmount = 2;
const int sensorAmount = 2;
const int valuesPrSensor = 3;

unsigned long time = 0;
unsigned long lastTime = 0;

const int len = arduinoAmount * sensorAmount * valuesPrSensor;

unsigned char arduino[arduinoAmount][sensorAmount][valuesPrSensor];

unsigned char buffer[256]; //Initializes the buffer we will use for storing the incomming serial data. This will have to be greater than the packetSize
const int packetSize = 7; //The packet size in bytes, defined in the serial sender, one for SOP one for SEQ one for LEN and however many data bytes you have in the array (in this case 8) and one for CHK
int howBig = 25;

void setup() {
  
  for(int a = 0; a < arduinoAmount; a++){
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[a][s][v] = 0;
      }
    }
  }
  
  Serial.begin(19200); // initialize serial communication
  mySerial.begin(19200);
}

void loop() {
  if(mySerial.readBytes((char *)buffer, packetSize) == packetSize){ //Reads from the serial line until the buffer is equal to the size of the packet (one complete packet in the buffer)
    delay(5);
    int a = int(buffer[0]);
    int bufVal = 1;
    for(int s = 0; s < sensorAmount; s++){
      if(a >= arduinoAmount){
        mySerial.flush();
        break;
      }
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[a][s][v] = buffer[bufVal];
        bufVal++;
      }
    }
  }
  
  time = millis();
  
  if(time > lastTime + 20){
    lastTime = time;
    unsigned char data[len]; //Putting the values into the data arrgyroY, expand or decrease as needed
    int count = 0;
    for(int a = 0; a < arduinoAmount; a++){
      for(int s = 0; s < sensorAmount; s++){
        for(int v = 0; v < valuesPrSensor; v++){
          data[count] = arduino[a][s][v];
          /*
          Serial.print(arduino[a][s][v]);
          Serial.print(' ');
          //Serial.print(data[count]);
          //Serial.print("    ");
          //*/
          count++;
        }
      }
    }
   // Serial.println();
  //*
    const unsigned int myBufferSize = 260; //myBufferSize must be greater than the entire size of the package
    unsigned char buffer[myBufferSize]; //creates the array, buffer, of size myBufferSize
   
    int bytesWritten = bap::createPacket(buffer, data,len, seq++); //creates the package and returns the size of the package
    
    Serial.write(buffer, bytesWritten); //sends the packet as a series of bytes with the length of bytesWritten
   //*/
  }
}
