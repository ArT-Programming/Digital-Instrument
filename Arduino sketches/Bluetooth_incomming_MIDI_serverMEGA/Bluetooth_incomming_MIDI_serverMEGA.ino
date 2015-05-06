#include <Kalman.h>
#include <MIDI.h>

String slaveAdd0 = "\r\n+CONN=0,18,E4,C,67,FD\r\n"; //ART01158 paired with master ART01155; pin 0000 OK
String slaveAdd1 = "\r\n+CONN=0,18,E4,C,68,1\r\n"; //ART01154 paired with master ART01151; pin 1111 OK
//String slaveAdd2 = "\r\n+CONN=0,18,E4,C,67,FA\r\n"; //ART01159 paired with master ART01152; pin 2222
//String slaveAdd3 = "\r\n+CONN=0,18,E4,C,68,A\r\n"; //ART01135 paired with master ART01156; pin 3333 OK

void masterSetup0(){
  Serial2.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  Serial2.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
  Serial2.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  Serial2.print("\r\n+STNA=master0\r\n");//set the bluetooth name as "master"
  Serial2.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  Serial2.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  Serial2.print("\r\n+STPIN=0000\r\n");//Set Master pincode"0000",it must be same as Slave pincode
  delay(2000); // This delay is required.
  Serial2.flush();
  Serial2.print("\r\n+INQ=1\r\n");//make the master inquire
  //Serial.println("Master is inquiring!");
  delay(2000); // This delay is required.
  Serial2.print(slaveAdd0);
}

void masterSetup1(){
  Serial3.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  Serial3.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
  Serial3.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  Serial3.print("\r\n+STNA=master1\r\n");//set the bluetooth name as "master"
  Serial3.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  Serial3.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  Serial3.print("\r\n+STPIN=1111\r\n");//Set Master pincode"0000",it must be same as Slave pincode
  delay(2000); // This delay is required.
  Serial3.flush();
  Serial3.print("\r\n+INQ=1\r\n");//make the master inquire
  //Serial.println("Master is inquiring!");
  delay(2000); // This delay is required.
  Serial3.print(slaveAdd1);
}

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

//------------

const int arduinoAmount = 2;
const int sensorAmount = 2;
const int valuesPrSensor = 3;

Arduino ard[arduinoAmount];
int volume[arduinoAmount];

unsigned long time = 0;
unsigned long oldTime = 0;

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
    volume[a] = 0;
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[a][s][v] = 0;
      }
    }
  }
  
  //Serial.begin(19200);
  Serial2.begin(38400);
  masterSetup0();
  
  Serial3.begin(38400);
  masterSetup1();
  
  //wait 1s and flush the serial buffer
  delay(1000);
  Serial2.flush();
  Serial3.flush();
} 
 
void loop() 
{
 // Serial2.listen();
  if(Serial2.readBytes((char *)buffer, packetSize) == packetSize){ //Reads from the serial line until the buffer is equal to the size of the packet (one complete packet in the buffer)
    int bufVal = 0;
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[0][s][v] = buffer[bufVal];
        bufVal++;
      }
    }
  }
  //Serial3.listen();
  if(Serial3.readBytes((char *)buffer, packetSize) == packetSize){ //Reads from the serial line until the buffer is equal to the size of the packet (one complete packet in the buffer)
    int bufVal = 0;
    for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        arduino[1][s][v] = buffer[bufVal];
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
       
       volume[a] = currentVolume(ard[0].veloX, ard[0].veloY, ard[0].veloZ);
       
       data[a][0] = ((ard[a].angleX + 90) / 180.) * 127;
       data[a][1] = ((ard[a].angleY + 90) / 180.) * 127;
       data[a][2] = volume[a];
     }
  
    int MidiNo = 20;
    
    for(int a = 0; a < arduinoAmount; a++){
      for(int v = 0; v < 3; v++){
        MIDI.sendControlChange(MidiNo, data[a][v], 1);
        MidiNo++;
    }
    MidiNo += 17;
  }
  delay(5);
}
  

float calDT(unsigned long time){
  float dt = time - oldTime;  
  oldTime = time;
  return dt;
}
