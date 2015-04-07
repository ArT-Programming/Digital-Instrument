#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"
#include "Gamma/Delay.h"
#include "Gamma/Filter.h"
#include <iostream>

class Synth{
public:
gam::Sine<> car;	// Carrier sine (gets its frequency modulated)
gam::Saw<> mod;	// Modulator sine (used to modulate frequency)
gam::Osc<> osc[4][3];// Wavetable oscillators
gam::AD<> env;
gam::ArrayPow2<float> table[4];	// Wavetable
gam::Delay<> delay;
gam::OnePole<> lpf;

float freq;
float volume[20000];
float resetTime;

	Synth(){
		delay.maxDelay(3);
		
		freq = 440;
		for(int i = 0; i<100; i++){
			volume[i] = 0;
		}
		resetTime = 1;
		env.attack(0.01);
		env.decay(0.3);
		
		// Set the table size; must be a power of 2
		for(int i = 0; i < 4; i++){
			table[i].resize(2048);
		}

		/* Add sine waves to wavetable to generate a Fourier series
		The addSine function adds a sine wave to an existing array of samples.
		It takes the wavetable as the first argument followed by the number of
		cycles of the sine, its amplitude, and then its phase through the cycle
		in [0, 1].*/
		for(int k=1; k<=16; k++){
			//gam::addSine(table, k, 1./k); // saw wave
			gam::addSine(table[0], k, (2*k-1)); // Clausen function
			gam::addSine(table[1], k, (2*k-1)); // square wave
			gam::addSine(table[2], k, (2*k-1)); // square wave
			gam::addSine(table[3], k, (2*k-1)); // triangle wave
		}

		// Assign a wavetable to the oscillator
		for(int i = 0; i < 4; i++){
			for(int j = 0; j < 3; j++){
				osc[i][j].source(table[0]);		
			}
		}
	}
	
	float echo(float source, float delayTime, float feedback = 0.9, float cutoff = 5000){
		delay.delay(delayTime);
		float Echo = delay();
		lpf.freq(cutoff);
		Echo = lpf(Echo);
		delay(source + Echo * feedback);
		return Echo;
	}
	
	void clip(float& source, double max, double min){
		if(source > max) source = max;
		else if(source < min) source = min;
	}
	
	float wavOsc(float angleX, float angleY){
		float panX = ((angleX+90) / 180.);
		float panY = ((angleY+90) / 180.); 
		
		//std::cout<<panX<<"     "<<panY<<"\n";
		
		if(panX > 1) panX = 1;
		else if(panX < -1) panX = -1;
		if(panY > 1) panY = 1;
		else if(panY < -1) panY = -1;
		
		float volume[4];
		/*if(panX > 0){
			volume[0] = panX;
			volume[1] = 0;
		}else{
			volume[0] = 0;
			volume[1] = fabs(panX - 1);
		}
		
		if(panY > 0){
			volume[2] = panY;
			volume[3] = 0;
		}else{
			volume[2] = 0;
			volume[3] = fabs(panY - 1);
		}*/
		volume[0] = panX;
		volume[1] = panY;//fabs(panX - 1);
		volume[2] = panY;
		volume[3] = fabs(panY - 1);
		
		
		float wav[4] = {0,0,0,0};
				
		float freq[4][3] = {
			{panX * 14 + 4, 100 ,panX * 10 + 20},
			{panY * 8 + 2, 20 ,panY * 20 + 20},
			{110.2,11.,1},
			{144.4,44.5,4.7}};
		
		for(int i = 0; i < 2; i++){
			for(int j = 0; j < 1; j++){
				osc[i][j].freq(freq[i][j]);
				wav[i] += osc[i][j]();
			}
		}

		
	
		//mixing
		float out = 0; 
		
		for(int i = 0; i < 2; i++){
			out += wav[i] * volume[i];
		}
		
		out *= 0.01;
		
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
		//if(resetTime > 0.5){
			//resetTime -= 0.5;
			if(x > 180){ 
				env.reset();
				//std::cout<<"reset on X \n";
			}
			else if(y > 180){ 
				env.reset();
				//std::cout<<"reset on Y \n"; env.reset();
			}
		//}
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