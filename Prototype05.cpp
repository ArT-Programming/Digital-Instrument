//#include "allocore/io/al_App.hpp"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"
#include "Gamma/Delay.h"
#include "Gamma/Filter.h"
#include "Gamma/Noise.h"
#include "Gamma/SamplePlayer.h"
#include "header/al_AudioApp.hpp"
#include "header/Arduino.hpp"
#include "header/Sound.hpp"
using namespace al;

class MyApp : public AudioApp{
public:
	static const int A = 2;
	Arduino arduino[A];
	Values values;
	Synth fmSynth[A];
	int currentRead;
	int meanArray;
	float time;
	
	MyApp():
	values(A,2,3,"COM3",19200) // arduino, sensor, amount of values pr sensor, COMPORT, baudrate 
	//values(A,2,3,"/dev/tty.usbmodem1411",19200) // arduino, sensor, amount of values pr sensor, COMPORT, baudrate 
	{
		initAudio(44100,128,2,0);
		currentRead = 0;
		meanArray = 2500;
		time = 0;
	}

	// Audio callback
	virtual void onSound(AudioIOData& io){	
		gam::sampleRate(io.fps());
	
		float index = ((arduino[0].angleX + 90) / 180.) * 20;
		float ratio = ((arduino[0].angleX + 90) / 180.) * 5;
		float fc = ((arduino[0].angleY + 90) / 180.) * 1000 + 40;	
		
		float index1 = ((arduino[1].angleX + 90) / 180.) * 20;
		float ratio1 = ((arduino[1].angleX + 90) / 180.) * 5;
		float fc1 = ((arduino[1].angleY + 90) / 180.) * 1000 + 40;	
		
		//fmSynth[0].resetEnvelope(arduino[1].veloX, arduino[1].veloY, arduino[1].veloZ);
		
		while(io()){
			
			fmSynth[0].volume[currentRead] = fmSynth[0].currentVolume(arduino[0].veloX, arduino[0].veloY, arduino[0].veloZ);	
			fmSynth[1].volume[currentRead] = fmSynth[1].currentVolume(arduino[1].veloX, arduino[1].veloY, arduino[1].veloZ);	
			
			currentRead++;
			if(currentRead == meanArray){
				currentRead = 0;		
			}
			
			float volume[2];
			volume[0] = fmSynth[0].averageFilter(fmSynth[0].volume, meanArray);
			volume[1] = fmSynth[1].averageFilter(fmSynth[1].volume, meanArray);
			
			float out[2];
			out[0] = fmSynth[0].modulate(fc, ratio, index);
			out[1] = fmSynth[1].modulate(fc1, ratio1, index1);
			//out[1] = fmSynth[1].wavOsc(arduino[1].angleX, arduino[1].angleY);
			
			io.out(0) = out[0] * volume[0];
			io.out(1) = out[1] * volume[1];
		}
	} 

	// Graphics callbacks
	virtual void onAnimate(double dt){
		
		if(values.unpack()){
			for(int i = 0; i < 2; i++){
				arduino[i].getValues(i, values.arduino);
				arduino[i].setAngles(dt);
				arduino[i].velocity(dt);
			}
			//std::cout<<"acc: "<<arduino[0].acc[0]<<"     "<<arduino[0].acc[1]<<"     "<<arduino[0].acc[2]<<"\n";
			//std::cout<<arduino[0].angleX<<"     "<<arduino[0].angleY<<"\n";
		}
		
		
	}
};


int main(){
	MyApp().start();
}