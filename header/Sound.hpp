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
	
	float volume(float x = 0, float y = 0, float z = 0){
		
		float mean = (x + y + z) / 3.;
		mean = mean / 180.;
		if(mean > 1) mean = 1;
		if(mean < 0.1) mean = 0;
		return mean;
	
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