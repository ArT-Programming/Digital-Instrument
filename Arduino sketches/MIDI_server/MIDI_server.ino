#include <Kalman.h>
#include <MIDI.h>
#include <SoftwareSerial.h>   //Software Serial Port

#define RxD0 5 //Yellow
#define TxD0 4 //White

#define RxD1 7 //Yelllow
#define TxD1 6 //White

SoftwareSerial masterSerial0(RxD0,TxD0);
SoftwareSerial masterSerial1(RxD1,TxD1);

String slaveAdd0 = "\r\n+CONN=0,18,E4,C,67,FD\r\n"; //ART01158 paired with master ART01155; pin 0000 OK
String slaveAdd1 = "\r\n+CONN=0,18,E4,C,68,1\r\n"; //ART01154 paired with master ART01151; pin 1111 OK
//String slaveAdd2 = "\r\n+CONN=0,18,E4,C,67,FA\r\n"; //ART01159 paired with master ART01152; pin 2222
//String slaveAdd3 = "\r\n+CONN=0,18,E4,C,68,A\r\n"; //ART01135 paired with master ART01156; pin 3333 OK

void masterSetup0(){
  masterSerial0.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  masterSerial0.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
  masterSerial0.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  masterSerial0.print("\r\n+STNA=master0\r\n");//set the bluetooth name as "master"
  masterSerial0.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  masterSerial0.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  masterSerial0.print("\r\n+STPIN=0000\r\n");//Set Master pincode"0000",it must be same as Slave pincode
  delay(2000); // This delay is required.
  masterSerial0.flush();
  masterSerial0.print("\r\n+INQ=1\r\n");//make the master inquire
  //Serial.println("Master is inquiring!");
  delay(2000); // This delay is required.
  masterSerial0.print(slaveAdd0);
}

void masterSetup1(){
  masterSerial1.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  masterSerial1.print("\r\n+STWMOD=1\r\n");//set the bluetooth work in master mode
  masterSerial1.print("\r\n+STBD=38400\r\n"); //tell the bluetooth to communicate at baudrate 38400
  masterSerial1.print("\r\n+STNA=master1\r\n");//set the bluetooth name as "master"
  masterSerial1.print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
  masterSerial1.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  masterSerial1.print("\r\n+STPIN=1111\r\n");//Set Master pincode"0000",it must be same as Slave pincode
  delay(2000); // This delay is required.
  masterSerial1.flush();
  masterSerial1.print("\r\n+INQ=1\r\n");//make the master inquire
  //Serial.println("Master is inquiring!");
  delay(2000); // This delay is required.
  masterSerial1.print(slaveAdd1);
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
const int packetSize = 18; //The packet size in bytes, defined in the serial sender, one for SOP one for SEQ one for LEN and however many data bytes you have in the array (in this case 8) and one for CHK
unsigned char SOP = 0xFF;
unsigned char ID[4] = {0,1,2,3};

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
  
  Serial.begin(19200);
  pinMode(RxD0, INPUT);
  pinMode(TxD0, OUTPUT);
  masterSetup0();
  
  pinMode(RxD1, INPUT);
  pinMode(TxD1, OUTPUT);
  masterSetup1();
  
  //wait 1s and flush the serial buffer
  delay(1000);
  masterSerial0.flush();
  masterSerial1.flush();
} 
 
void loop() 
{
  
  masterSerial0.listen();
  if(masterSerial0.readBytes((char *)buffer, packetSize) == packetSize){ //Reads from the serial line until the buffer is equal to the size of the packet (one complete packet in the buffer)
    int start = 0;
    int bufVal = 0;
    unsigned char CHK = 0;
    
    for(int i = 0; i < packetSize; i++){
      //Serial.print(buffer[i]);
      //Serial.print(' ');
    }
    //Serial.println();
      
    for(int i = 0; i < packetSize; i++){
      start = i;
      if(buffer[i] == SOP && buffer[i+1] == ID[0]) break;
    }
    //Serial.print("start = ");
    //Serial.println(start);
    
    if(start < 9){
      for(int i = 2; i < 8; i++){
        CHK += buffer[start+i];
        //Serial.print(CHK);
        //Serial.print(" -> ");
      }
      //Serial.println();
      
      //Serial.print("CHK =       ");
      //Serial.println(CHK);
      //Serial.print("required = ");
      //Serial.println(buffer[start+8]);
      
      if(buffer[start+8] == CHK){
        //Serial.println("IT'S WORKING!!");
        bufVal = start + 2;
        for(int s = 0; s < sensorAmount; s++){
          for(int v = 0; v < valuesPrSensor; v++){
            arduino[0][s][v] = buffer[bufVal];
            bufVal++;
          }
        }
      }
      //masterSerial0.flush();
    }
    //Serial.println();
  }
  
  masterSerial1.listen();
  if(masterSerial1.readBytes((char *)buffer, packetSize) == packetSize){ //Reads from the serial line until the buffer is equal to the size of the packet (one complete packet in the buffer)
    int start = 0;
    int bufVal = 0;
    unsigned char CHK = 0;
    
    for(int i = 0; i < packetSize; i++){
      //Serial.print(buffer[i]);
      //Serial.print(' ');
    }
    //Serial.println();
      
    for(int i = 0; i < packetSize; i++){
      start = i;
      if(buffer[i] == SOP && buffer[i+1] == ID[1]) break;
    }
    //Serial.print("start = ");
    //Serial.println(start);
    
    if(start < 9){
      for(int i = 2; i < 8; i++){
        CHK += buffer[start+i];
        //Serial.print(CHK);
        //Serial.print(" -> ");
      }
      //Serial.println();
      
      //Serial.print("CHK =       ");
      //Serial.println(CHK);
      //Serial.print("required = ");
      //Serial.println(buffer[start+8]);
      
      if(buffer[start+8] == CHK){
        //Serial.println("IT'S WORKING!!");
        bufVal = start + 2;
        for(int s = 0; s < sensorAmount; s++){
          for(int v = 0; v < valuesPrSensor; v++){
            arduino[1][s][v] = buffer[bufVal];
            bufVal++;
          }
        }
      }
      //masterSerial0.flush();
    }
    //Serial.println();
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
      volume[a] = currentVolume(ard[a].veloX, ard[a].veloY, ard[a].veloZ);
      
      data[a][0] = ((ard[a].angleX + 90) / 180.) * 127;
      data[a][1] = ((ard[a].angleY + 90) / 180.) * 127;
      data[a][2] = volume[a];
    }
    
    /*for(int s = 0; s < sensorAmount; s++){
      for(int v = 0; v < valuesPrSensor; v++){
        Serial.print(arduino[0][s][v]);
        Serial.print(" ");
      }
    }
    lastTime = time;
    Serial.println();
  //}*/
  // 0 is gyro and 1 is acc
  
    int MidiNo = 20;
    
    for(int a = 0; a < arduinoAmount; a++){
      for(int v = 0; v < 3; v++){
        MIDI.sendControlChange(MidiNo, data[a][v], 1);
        MidiNo++;
    }
    MidiNo += 17;
  }
}
  

float calDT(unsigned long time){
  float dt = time - oldTime;  
  oldTime = time;
  return dt;
}
