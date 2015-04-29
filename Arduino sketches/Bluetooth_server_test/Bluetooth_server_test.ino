/* Upload this sketch into Seeeduino and press reset*/
#include <SoftwareSerial.h>   //Software Serial Port
#include <BAP.h> //Byte ArrgyroY Protocol

#define RxD 7
#define TxD 6

String retSymb = "+RTINQ=";//start symble when there's any return
/*
String slaveName = "slave0";//Set the Slave name ,caution that ';'must be included
String slaveAddr = "\r\n+CONN=0,18,E4,C,67,FD\r\n";
String pinCode = "\r\n+STPIN=0000\r\n";
*/

String slaveName = "slave1";//Set the Slave name ,caution that ';'must be included
String slaveAddr = "\r\n+CONN=0,18,E4,C,68,1\r\n";
String pinCode = "\r\n+STPIN=1111\r\n";

/*
String slaveName = "slave2";//Set the Slave name ,caution that ';'must be included
String slaveAddr = "\r\n+CONN=0,18,E4,C,67,FA\r\n";
String pinCode = "\r\n+STPIN=2222\r\n";
*/
/*
String slaveName = "slave3";//Set the Slave name ,caution that ';'must be included
String slaveAddr = "\r\n+CONN=0,18,E4,C,68,A\r\n";
String pinCode = "\r\n+STPIN=3333\r\n";
*/
int nameIndex = 0;
int addrIndex = 0;

String recvBuf;

String connectCmd = "\r\n+CONN=";

SoftwareSerial blueToothSerial(RxD,TxD);

//------------
unsigned char seq = 0; // sequence number

const int arduinoAmount = 1;
const int sensorAmount = 2;
const int valuesPrSensor = 3;

unsigned long time = 0;
unsigned long lastTime = 0;

const int len = arduinoAmount * sensorAmount * valuesPrSensor;

unsigned char arduino[arduinoAmount][sensorAmount][valuesPrSensor];

unsigned char buffer[256]; //Initializes the buffer we will use for storing the incomming serial data. This will have to be greater than the packetSize
const int packetSize = 6; //The packet size in bytes, defined in the serial sender, one for SOP one for SEQ one for LEN and however many data bytes you have in the array (in this case 8) and one for CHK
int howBig = 25;

//---------------

void masterSetup()
{
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
  blueToothSerial.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  blueToothSerial.print("\r\n+STNA=master\r\n");//set the bluetooth name as "master"
  blueToothSerial.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print(pinCode);//Set Master pincode"0000",it must be same as Slave pincode
  delay(2000); // This delay is required.
  blueToothSerial.flush();
  blueToothSerial.print("\r\n+INQ=1\r\n");//make the master inquire
  Serial.println("Master is inquiring!");
  delay(2000); // This delay is required.
    
  blueToothSerial.print(slaveAddr);
  
  //blueToothSerial.print("\r\n+RTADDR\r\n");
  //find the target slave
  char recvChar;
  while(1){
    if(blueToothSerial.available()){
      //Serial.println("I saw something!");
      recvChar = blueToothSerial.read();
      Serial.print(recvChar);
      recvBuf += recvChar;
      nameIndex = recvBuf.indexOf(slaveName);//get the position of slave name
      //Serial.print(" ---- ");
      //Serial.println(nameIndex);
      //nameIndex -= 1;//decrease the ';' in front of the slave name, to get the position of the end of the slave address
      if ( nameIndex != -1 ){
 	//addrIndex = (recvBuf.indexOf(retSymb,(nameIndex - retSymb.length()- 18) ) + retSymb.length());//get the start position of slave address	 		
 	//slaveAddr = recvBuf.substring(addrIndex, nameIndex);//get the string of slave address
 	break;
      }
    }
  }
  //form the full connection command
  connectCmd += slaveAddr;
  connectCmd += "\r\n";
  int connectOK = 0;
  Serial.print("Connecting to slave:");
  Serial.print(slaveAddr);
  Serial.println(slaveName);
  //connecting the slave till they are connected
  do{
    blueToothSerial.print(connectCmd);//send connection command
    recvBuf = "";
    while(1){
      if(blueToothSerial.available()){
        recvChar = blueToothSerial.read();
        Serial.print(recvChar);
 	recvBuf += recvChar;
 	if(recvBuf.indexOf("CONNECT:OK") != -1){
          connectOK = 1;
 	  Serial.println("Connected!");
 	  blueToothSerial.print("Connected!");
 	  break;
 	}else if(recvBuf.indexOf("CONNECT:FAIL") != -1){
 	  Serial.println("Connect again!");
 	  break;
 	}
      }
    }
  }while(0 == connectOK);
}

void setup() 
{ 
  for(int a = 0; a < arduinoAmount; a++){
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[a][s][v] = 0;
      }
    }
  }
  
  Serial.begin(19200);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  masterSetup();
  //wait 1s and flush the serial buffer
  delay(1000);
  Serial.flush();
  blueToothSerial.flush();
} 
 
void loop() 
{
  if(blueToothSerial.readBytes((char *)buffer, packetSize) == packetSize){ //Reads from the serial line until the buffer is equal to the size of the packet (one complete packet in the buffer)
    int bufVal = 0;
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[0][s][v] = buffer[bufVal];
        bufVal++;
      }
    }
  }
  
  time = millis();
  
  if(time > lastTime + 20){
    lastTime = time;
    unsigned char data[len]; //Putting the values into the data arrgyroY, expand or decrease as needed
    int count = 0;
    for(int a = 0; a < 1; a++){
      for(int s = 0; s < sensorAmount; s++){
        for(int v = 0; v < valuesPrSensor; v++){
          Serial.print(int(arduino[a][s][v]));
          Serial.print(' ');
          count++;
        }
      }
    }
    Serial.println();
  }
}
