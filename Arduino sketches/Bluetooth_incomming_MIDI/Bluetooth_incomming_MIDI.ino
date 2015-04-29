#include <Kalman.h>
#include <MIDI.h>
#include <SoftwareSerial.h>   //Software Serial Port

#define RxD 7
#define TxD 6

String slaveAdd = "\r\n+CONN=bb,bb,cc,dd,ee,ff\r\n";

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
    
    if(angleX > 90.) angleX = 90;
    else if(angleX < -90.) angleX = -90;
    if(angleY > 90.) angleY = 90;
    else if(angleY < -90.) angleY = -90;
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


SoftwareSerial blueToothSerial(RxD,TxD);

//------------
//unsigned char seq = 0; // sequence number

const int arduinoAmount = 1;
const int sensorAmount = 2;
const int valuesPrSensor = 3;

unsigned long time = 0;
unsigned long oldTime = 0;
unsigned long lastTime = 0;

unsigned char arduino[arduinoAmount][sensorAmount][valuesPrSensor];
unsigned char data[arduinoAmount][3]; // 4 arduinos with angleX, angleY, volume

unsigned char buffer[256]; //Initializes the buffer we will use for storing the incomming serial data. This will have to be greater than the packetSize
const int packetSize = 6; //The packet size in bytes, defined in the serial sender, one for SOP one for SEQ one for LEN and however many data bytes you have in the array (in this case 8) and one for CHK

//---------------


int currentVolume(float x = 0, float y = 0, float z = 0){
  int mean = (x + y + z) / 3.;
  mean = (mean / 360.) * 127;
  if(mean > 127) mean = 127;
  if(mean < 2) mean = 0;
  return mean;
}

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
    int bufVal = 0;
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[0][s][v] = buffer[bufVal];
        bufVal++;
      }
    }
  }
  
  time = millis();
  float dt = calDT(time) / 1000.; //frame time in seconds
  
  //if(time > lastTime + 20){
    // calculate kalman and set the angles and velocity
    for(int a = 0; a < arduinoAmount; a++){
       ard[a].getValues(a, arduino);
       ard[a].setAngles(dt);
       ard[a].velocity(dt);
       
       /*Serial.print(ard[a].angleX);
       Serial.print(" ");
       Serial.print(ard[a].angleY);
       Serial.print(" ");
       Serial.print(ard[a].angleZ);
       Serial.print(" ");*/
       int volume = currentVolume(ard[0].veloX, ard[0].veloY, ard[0].veloZ);
       
       data[a][0] = ((ard[a].angleX + 90) / 180.) * 127;
       data[a][1] = ((ard[a].angleY + 90) / 180.) * 127;
       data[a][2] = volume;
     }
     /*
      for(int s = 0; s < sensorAmount; s++){
        for(int v = 0; v < valuesPrSensor; v++){
          Serial.print(arduino[0][s][v]);
          Serial.print(" ");
        }
      }
     lastTime = time;
     Serial.println();*/
  //}
  // 0 is gyro and 1 is acc
  
    int MidiNo = 20;
    
    for(int a = 0; a < arduinoAmount; a++){
      for(int v = 0; v < 3; v++){
        MIDI.sendControlChange(MidiNo, data[a][v], 1);
        MidiNo++;
    }
    MidiNo += 7;
  }
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
  blueToothSerial.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set Master pincode"0000",it must be same as Slave pincode
  delay(2000); // This delay is required.
  blueToothSerial.flush();
  blueToothSerial.print("\r\n+INQ=1\r\n");//make the master inquire
  //Serial.println("Master is inquiring!");
  delay(2000); // This delay is required.
    
  //blueToothSerial.print("\r\n+CONN=0,18,E4,C,67,A\r\n");
  blueToothSerial.print(slaveAdd);  
}
