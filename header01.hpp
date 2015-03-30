
#include "Kalman.h"
#include "BAP/BAP.h"
#include "allocore/io/al_Serial.hpp"
#include "Gamma/Oscillator.h"

class Synth{
public:
gam::Sine<> car;	// Carrier sine (gets its frequency modulated)
gam::Sine<> mod;	// Modulator sine (used to modulate frequency)
float freq;
	Synth(){
		freq = 440;
	}
	
	float modulate(float fc, float ratio, float I){
		
		if(ratio == 0) ratio = 0.001;
		
		float fm = fc/ratio;
		mod.freq(fm);
		

		// Compute frequency deviation signal
		float df = mod() * fm * I;
		freq = df;	
		// Set frequency of carrier
		car.freq(fc + df);

		// Most of FM is just mapping to the carrier frequency!
		// Here, we generate the carrier's next sample.
		float s = car() * 0.5;
		
		return s;
	}
	float volume(float x = 0, float y = 0, float z = 0){
		float mean = (x + y + z) / 3.;
		mean = mean / 180.;
		if(mean > 1) mean = 1;
		if(mean < 0.1) mean = 0;
		return mean;
	
	}
};

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
	
	/*~Values(){
		delete arduino;
	}*/
	
	void unpack(){
		unsigned char buffer[256];
        int bytesRead = serial.read(buffer, sizeof(buffer));
        for(int i=0; i<bytesRead; ++i){
            if(parser.inputByte(buffer[i])){
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

class Notes{
public:
	gam::Sine<> src[1],bass[10];
	gam::AD<> srcEnv[1],bassEnv[10];
	int noteNo[10];
	int wScale[10];
	Notes(){
		noteNo[0] = 48;
		noteNo[1] = 50;
		noteNo[2] = 52;
		noteNo[3] = 55;
		noteNo[4] = 57;
		noteNo[5] = 60;
		noteNo[6] = 62;
		noteNo[7] = 64;
		noteNo[8] = 67;
		noteNo[9] = 69;
		
		wScale[0] = 45;
		wScale[1] = 47;
		wScale[2] = 49;
		wScale[3] = 50;
		wScale[4] = 53;
		wScale[5] = 54;
		wScale[6] = 56;
		wScale[7] = 57;
		wScale[8] = 59;
		wScale[9] = 61;
		
		
		int bassNo[10];
		for(int i = 0; i < 10; i++){
			bassNo[i] = noteNo[i] - 4;
		}
		
		for(int i = 0; i < 10; i++){
			src[i].freq((pow(2., (((noteNo[i] + 12) - 69) / 12.))) * 440);
			bass[i].freq((pow(2., (((bassNo[i] + 12) - 69) / 12.))) * 440);
			srcEnv[i].attack(0.1);
			srcEnv[i].decay(0.8);
			bassEnv[i].attack(0.1);
			bassEnv[i].decay(1.2);
		}
	}
	
	float penta(float n){
		if(n >= 0 && n <= 127){
			return (pow(2., (n - 69) / 12.)) * 440;
		}
		else return 0;
		//src[0].freq((pow(2., (((noteNo[n]-12) - 69) / 12.))) * 440);
		//srcEnv[i].attack(0.1);
		//srcEnv[i].decay(0.8);
	}
	float egypt(int n){
		return (pow(2., (((wScale[n] + 12) - 69) / 12.))) * 440;
		
	}
	
	void attack(){
		//std::cout<<"ATTACK!!!!!"<<"\n";
		int n = rand() % 10;
		while(!srcEnv[n].done()){
			n = rand() % 10;
		}
		std::cout<< n <<"\n";
		srcEnv[n].reset();
		bassEnv[n].reset();
	}
	
	float note(){
		float output = 0;
		for(int i = 0; i < 10; i++){
			output += src[i]() * srcEnv[i]() + bass[i]() * bassEnv[i]();
		}
		return output*0.2;
	}
};