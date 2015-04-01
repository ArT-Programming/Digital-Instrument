#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"

class Synth{
public:
gam::Sine<> car;	// Carrier sine (gets its frequency modulated)
gam::Sine<> mod;	// Modulator sine (used to modulate frequency)
gam::Osc<> osc[6];// Wavetable oscillators
gam::AD<> env;
gam::ArrayPow2<float> table, table2;	// Wavetable

float freq;
float volume[20000];
float resetTime;

	Synth(){
		freq = 440;
		for(int i = 0; i<100; i++){
			volume[i] = 0;
		}
		resetTime = 1;
		env.attack(0.1);
		env.decay(0.5);
		
		// Set the table size; must be a power of 2
		//for(int i = 0)
		table.resize(2048);
		table2.resize(2048);

		/* Add sine waves to wavetable to generate a Fourier series
		The addSine function adds a sine wave to an existing array of samples.
		It takes the wavetable as the first argument followed by the number of
		cycles of the sine, its amplitude, and then its phase through the cycle
		in [0, 1].*/
		for(int k=1; k<=16; k++){
			//gam::addSine(table, k, 1./k); // saw wave
			gam::addSine(table, k, 1./(k*k)); // Clausen function
			gam::addSine(table2, 2*k, (2*k-1)); // square wave
			//gam::addSine(table[2], k, 1./k); // square wave
			//gam::addSine(table, k, 1./(k*k), 0.25); // triangle wave
		}

		// Assign a wavetable to the oscillator
		for(int i = 0; i < 3; i++){
			osc[i].source(table);
			osc[i+3].source(table2);		
		}
	}
	
	float wavOsc(float angleX, float angleY){
		float freq1 = ((angleX) / 90.) * 50;
		float volumeX = ((angleX) / 180.) + 0.5; 
		float freq2 = ((angleY) / 90.) * 10 + 5;
		float volumeY = ((angleY) / 180.) + 0.5; 
		float wav1 = 0;
		float wav2 = 0;
		float up = 0;
		for(int i = 0; i < 3; i++){
			
			osc[i].freq(20 + up);
			wav1 += osc[i]();
			up += 0.1;
			}
		for(int i = 3; i < 6; i++){
			osc[i].freq(300 + up);
			wav2 += osc[i]();
			up += 0.1;
		}			
	
		//mixing
		float out = (wav1 * volumeX) + (wav2 * volumeY); 
		return out;
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
	float averageFilter(float x[], int length){ //the function collects an array of integers and an integer to get the length of the array. It returns an integer
  		float sum = 0; //varible to store the sum of the array elements
  		for(int i = 0; i < length; i++) sum += x[i]; //sum up the most current readings
 			return sum/length; //return the average by dividing by the number of elements
		}	
	
	float currentVolume(float x = 0, float y = 0, float z = 0){
		float mean = (x + y + z) / 3.;
		mean = mean / 360.;
		if(mean > 1.5) mean = 1.5;
		if(mean < 0.15) mean = 0;
		return mean;
	}
	
	void resetEnvelope(float x = 0, float y = 0, float z = 0){
		if(x > 180) env.reset();
		else if(y > 180) env.reset();
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