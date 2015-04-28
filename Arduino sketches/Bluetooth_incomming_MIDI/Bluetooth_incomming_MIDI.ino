#include <Kalman.h>
#include <MIDI.h>
#include <SoftwareSerial.h>   //Software Serial Port

#define RxD 7
#define TxD 6

class Arduino{
public:
  float gyro[3];
  float acc[3];
  Kalman kalmanX,kalmanY;
  float angleX,angleY,angleZ, oldAngleX, oldAngleY, oldAngleZ, veloX, veloY, veloZ;

	
  Arduino(){
    for(int i = 0; i < 3; i++){
	 gyro[i] = 0;
	  acc[i] = 0;
	 }
     angleX = angleY = angleZ = oldAngleX = oldAngleY = oldAngleZ = veloX = veloY = veloZ = 0;
		//kalmanX.setQangle(0.01); //0.001 default
		//kalmanX.setQbias(0.03); //0.003 default
		//kalmanX.setRmeasure(0.3); //0.03 default
	}
	
     void getValues(int ID, unsigned char arduino[][2][3]){
       for(int s = 0; s < 2; s++){ 
         for(int v = 0; v < 3; v++){
           if(s == 0){
             gyro[v] = arduino[ID][s][v] - 127;
             }
           if(s == 1){
             acc[v] =  ((arduino[ID][s][v] - 127) / 127.) * 1.52;
             }
           }
         }
       }
	
  void setAngles(double dt){
     double roll = (atan2(float(acc[0]), float(acc[2])) / (2.*M_PI)) * 360.;
     double pitch = (atan2(float(acc[1]), float(acc[2])) / (2.*M_PI)) * 360.;
     float rateX = -gyro[0]*20;
     float rateY = -gyro[1]*20;
        		
	if(acc[2] < 0){
	  if(roll < 0){
	     roll = -180-roll;
	    }else{
	      roll = 180-roll;
	}
	if(pitch < 0){
          	pitch = -180-pitch;
        } else{
	  pitch = 180-pitch;
	}
    }
    angleX = kalmanX.getAngle(roll, rateX, dt);
    angleY = kalmanY.getAngle(pitch, rateY, dt);
    angleZ += gyro[2];
    //angleZ *= 0.99;
  }
	
  void velocity(double dt){
	
    veloX = fabs(angleX - oldAngleX)/ dt;
    veloY = fabs(angleY - oldAngleY)/ dt;
    veloZ = fabs(angleZ - oldAngleZ)/ dt;
    oldAngleX = angleX;
    oldAngleY = angleY;
    oldAngleZ = angleZ;
  }
};

Arduino ard[4];

String retSymb = "+RTINQ=";//start symble when there's any return
String slaveName = ";SeeedBTSlave";//Set the Slave name ,caution that ';'must be included
int nameIndex = 0;
int addrIndex = 0;

String recvBuf;
String slaveAddr;

String connectCmd = "\r\n+CONN=";

SoftwareSerial blueToothSerial(RxD,TxD);

//------------
//unsigned char seq = 0; // sequence number

const int arduinoAmount = 2;
const int sensorAmount = 2;
const int valuesPrSensor = 3;

unsigned long time = 0;
unsigned long oldTime = 0;
unsigned long lastTime = 0;

//const int len = arduinoAmount * sensorAmount * valuesPrSensor;

unsigned char arduino[arduinoAmount][sensorAmount][valuesPrSensor];

unsigned char buffer[256]; //Initializes the buffer we will use for storing the incomming serial data. This will have to be greater than the packetSize
const int packetSize = 7; //The packet size in bytes, defined in the serial sender, one for SOP one for SEQ one for LEN and however many data bytes you have in the array (in this case 8) and one for CHK
//int howBig = 25;

//---------------

void setup() 
{ 
  MIDI.begin(4);
  for(int a = 0; a < arduinoAmount; a++){
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[a][s][v] = 0;
      }
    }
  }
  
  //Serial.begin(19200);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  masterSetup();
  //wait 1s and flush the serial buffer
  delay(1000);
  //Serial.flush();
  blueToothSerial.flush();
} 
 
void loop() 
{
  if(blueToothSerial.readBytes((char *)buffer, packetSize) == packetSize){ //Reads from the serial line until the buffer is equal to the size of the packet (one complete packet in the buffer)
    delay(5);
    int a = int(buffer[0]);
    int bufVal = 1;
    for(int s = 0; s < sensorAmount; s++){
      if(a >= arduinoAmount){
        blueToothSerial.flush();
        break;
      }
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[a][s][v] = buffer[bufVal];
        bufVal++;
      }
    }
  }
  time = millis();
  float dt = calDT(time);
  // calculate kalman and set the angles and velocity
  for(int i = 0; i < 1; i++){
     ard[i].getValues(i, arduino);
     ard[i].setAngles(dt);
     ard[i].velocity(dt);
   }
  // 0 is gyro and 1 is acc
  
  
  
    // send the midi data
    MIDI.sendControlChange(20, data[0], 1);
    //MIDI.sendControlChange(21, data[0], 1);
    //MIDI.sendControlChange(22, data[0], 1);
    //MIDI.sendControlChange(23, data[0], 1);
  }
  

float calDT(unsigned long time){
  float dt = time - oldTime;  
  oldTime = time;
  return dt;
  }	
 
void masterSetup()
{
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
  blueToothSerial.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  blueToothSerial.print("\r\n+STNA=master\r\n");//set the bluetooth name as "master"
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set Master pincode"0000",it must be same as Slave pincode
  delay(2000); // This delay is required.
  blueToothSerial.flush();
  blueToothSerial.print("\r\n+INQ=1\r\n");//make the master inquire
  //Serial.println("Master is inquiring!");
  delay(2000); // This delay is required.
    
  //blueToothSerial.print("\r\n+CONN=0,18,E4,C,67,FA\r\n");
  blueToothSerial.print("\r\n+CONN=aa,bb,cc,dd,ee,ff\r\n");  
}
 

