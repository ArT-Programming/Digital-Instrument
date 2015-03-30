//#include "allocore/io/al_App.hpp"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"
#include "Gamma/Delay.h"
#include "Gamma/Filter.h"
#include "Gamma/Noise.h"
#include "Gamma/SamplePlayer.h"
#include "header/al_AudioApp.hpp"
#include "header/header01.hpp"

using namespace al;

class MyApp : public AudioApp{
public:
	Arduino arduino[2];
	Values values;
	Synth fmSynth[2];
	
	
	MyApp():
	values(2,2,3,"COM3",19200)
	{
		initAudio(44100,128,2,0);
	}

	// Audio callback
	virtual void onSound(AudioIOData& io){	
		gam::sampleRate(io.fps());
	
		
		float index = ((arduino[0].angleX + 90) / 180.) * 20;
		float ratio = ((arduino[0].angleX + 90) / 180.) * 5;
		float fc = 500;	
	
		
		while(io()){
			float volume = fmSynth[0].volume(arduino[0].veloX, arduino[0].veloY, arduino[0].veloZ );
			float out = fmSynth[0].modulate(fc, 1, index);
			
			io.out(0) = out * volume;
			io.out(1) = out * volume;
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
		}	
	}
};


int main(){
	MyApp().start();
}