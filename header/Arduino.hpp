#include "Kalman.h"
#include "BAP/BAP.h"
#include "allocore/io/al_Serial.hpp"

class Values{
public:
	int arduinoAmount;
	int sensorAmount;
	int valuesPrSensor;
	//unsigned char *arduino;
	unsigned char arduino[4][2][3];
	serial::Serial serial;
    bap::Parser parser;
	
	Values(const int a, const int s, const int v, std::string port, unsigned int baudrate):
	arduinoAmount(a), sensorAmount(s), valuesPrSensor(v)//,
	//arduino(new unsigned char[arduinoAmount][sensorAmount][valuesPrSensor])
	{
		serial.setPort(port);	// SERIAL PORT!!!
        serial.setBaudrate(baudrate);
        serial.setTimeout(25);
        serial.open();
		
		for(int ar = 0; ar < arduinoAmount; ar++){
			for(int se = 0; se < sensorAmount; se++){
				for(int va = 0; va < valuesPrSensor; va++){
					arduino[ar][se][va] = 0;
				}
			}
		}
		
		
        if(!serial.isOpen()){
            std::cout << "Error opening serial port.\n";
            exit(0);
        }
	}
	
	Values():
	arduinoAmount(1), sensorAmount(2), valuesPrSensor(3)
	{
		serial.setPort("COM3");	// SERIAL PORT!!!
        serial.setBaudrate(19200);
        serial.setTimeout(25);
        serial.open();
		
		for(int ar = 0; ar < arduinoAmount; ar++){
			for(int se = 0; se < sensorAmount; se++){
				for(int va = 0; va < valuesPrSensor; va++){
					arduino[ar][se][va] = 0;
				}
			}
		}
		
		
        if(!serial.isOpen()){
            std::cout << "Error opening serial port.\n";
            exit(0);
        }
	}
	
	bool unpack(){
		bool read = false; 
		unsigned char buffer[256];
        int bytesRead = serial.read(buffer, sizeof(buffer));
        for(int i=0; i<bytesRead; ++i){
            if(parser.inputByte(buffer[i])){
            	read = true;
				for(int a = 0; a < arduinoAmount; a++){
					for(int s = 0; s < sensorAmount; s++){
						for(int v = 0; v < valuesPrSensor; v++){
							int parserNumber = (a*sensorAmount*valuesPrSensor)+(s*valuesPrSensor)+v;
							arduino[a][s][v] = parser.data(parserNumber);
						}
					}
				}
            }
        }
      return read;  
	}
};

class MeanFilter{
public:
	MeanFilter(){}
	
	int averageFilter(int x[], int length){ //the function collects an array of integers and an integer to get the length of the array. It returns an integer
  	int sum = 0; //varible to store the sum of the array elements
  	for(int i = 0; i < length; i++) sum += x[i]; //sum up the most current readings
 		return sum/length; //return the average by dividing by the number of elements
	}

	//function to sort the most current mean values and return the median value.
	int medianFilter(int x[], int length){ //the function collects an array of integers and an integer to get the length of the array. It returns an integer
  	int sorted[length]; //create an array the same size as the reading array
  	for(int i = 0; i < length; i++) sorted[i] = -1; //all elements of sorted[] array has to be set to a value to be able to compare them to other elements later on. I choose -1 because then i know any input will not be equal to that
  		for(int i = 0; i < length; i++){
    		int sortElement = 0; //variable sortElement is reset to 0 every time i is changed
   				 for(int j = 0; j < length; j++){
      				if(x[i] > x[j]) sortElement++; //compare element i of reading with all ten readings, and add one to sortElement when it is more than another
      				if(x[i] == sorted[j]) sortElement++; //if the checked element is same value as another reading that has already been sorted, add one to sortElement so it will be put into next element of sorted array
    			}
   			sorted[sortElement] = x[i]; //store x[i] into sorted[] and use the value of sortElement to select which element it has
  		}
  	return sorted[length/2]; //return the median value, which is the middle element of the sorted array
	}
};

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
			if(roll < 0)
			{
				roll = -180-roll;
			}else
			{
				roll = 180-roll;
			}
			if(pitch < 0)
			{
				pitch = -180-pitch;
			}else
			{
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
