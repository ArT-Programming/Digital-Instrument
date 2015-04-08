#include "iostream"
#include "allocore/io/al_App.hpp"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"
#include "allocore/io/al_MIDI.hpp"
#include <stdio.h>

using namespace al;

/*
	WaveTable, frequency modulation, synth class, mouse control
*/

class Sequencer{
public:
	gam::Sine<> src[8];
	gam::ADSR<> env[8];
	gam::Accum<> tmr;

	bool KeyOn[8][4];
	int pentaScale[8], counter;
	
	Sequencer(){
		counter = 0;
		tmr.freq(4);
		pentaScale[0] = 69;
		pentaScale[1] = 72;
		pentaScale[2] = 74;
		pentaScale[3] = 76;
		pentaScale[4] = 79;
		pentaScale[5] = 81;
		pentaScale[6] = 84;
		pentaScale[7] = 86;
		
		for(int i = 0; i < 8; i++){
			src[i].freq((pow(2., ((pentaScale[i] - 69) / 12.))) * 440);
			env[i].sustainDisable();
			env[i].attack(1./1000);
			env[i].decay(0.05);
			env[i].sustain(0.5);
			env[i].release(0.2);
			for(int j = 0; j < 4; j++){
				KeyOn[i][j] = false;
			}
		}	
	}
	
	void check(){
		for(int i = 0; i < 8; i++){
			if(KeyOn[i][counter]) env[i].reset();
		}
		if(tmr()) counter++;
		if(counter == 4) counter = 0;
	}
	
	// manage the output	
	float output(float volume = 0.1){
		float out = 0;
		for(int i = 0; i < 8; i++){
			out += src[i]() * env[i]();
		}
		out *= volume;	
		return out;
	}
		
	void KeyPress(const Keyboard& k){
		switch(k.key()){
		case '1': KeyOn[0][0] = !KeyOn[0][0]; break;
		case '2': KeyOn[1][0] = !KeyOn[1][0]; break;
		case '3': KeyOn[2][0] = !KeyOn[2][0]; break;
		case '4': KeyOn[3][0] = !KeyOn[3][0]; break;
		case '5': KeyOn[4][0] = !KeyOn[4][0]; break;
		case '6': KeyOn[5][0] = !KeyOn[5][0]; break;
		case '7': KeyOn[6][0] = !KeyOn[6][0]; break;
		case '8': KeyOn[7][0] = !KeyOn[7][0]; break;
		
		case 'q': KeyOn[0][1] = !KeyOn[0][1]; break;
		case 'w': KeyOn[1][1] = !KeyOn[1][1]; break;
		case 'e': KeyOn[2][1] = !KeyOn[2][1]; break;
		case 'r': KeyOn[3][1] = !KeyOn[3][1]; break;
		case 't': KeyOn[4][1] = !KeyOn[4][1]; break;
		case 'y': KeyOn[5][1] = !KeyOn[5][1]; break;
		case 'u': KeyOn[6][1] = !KeyOn[6][1]; break;
		case 'i': KeyOn[7][1] = !KeyOn[7][1]; break;
		
		case 'a': KeyOn[0][2] = !KeyOn[0][2]; break;
		case 's': KeyOn[1][2] = !KeyOn[1][2]; break;
		case 'd': KeyOn[2][2] = !KeyOn[2][2]; break;
		case 'f': KeyOn[3][2] = !KeyOn[3][2]; break;
		case 'g': KeyOn[4][2] = !KeyOn[4][2]; break;
		case 'h': KeyOn[5][2] = !KeyOn[5][2]; break;
		case 'j': KeyOn[6][2] = !KeyOn[6][2]; break;
		case 'k': KeyOn[7][2] = !KeyOn[7][2]; break;
		
		case 'z': KeyOn[0][3] = !KeyOn[0][3]; break;
		case 'x': KeyOn[1][3] = !KeyOn[1][3]; break;
		case 'c': KeyOn[2][3] = !KeyOn[2][3]; break;
		case 'v': KeyOn[3][3] = !KeyOn[3][3]; break;
		case 'b': KeyOn[4][3] = !KeyOn[4][3]; break;
		case 'n': KeyOn[5][3] = !KeyOn[5][3]; break;
		case 'm': KeyOn[6][3] = !KeyOn[6][3]; break;
		case ',': KeyOn[7][3] = !KeyOn[7][3]; break;
		}
	}
};

class MyApp : public App{
public:
	Sequencer seq;

	MyApp(){
		nav().pos(0,0,4);
		initWindow();
		initAudio();
		window().remove(navControl());
	}

	void onSound(AudioIOData& io){
		gam::sampleRate(io.fps());
		while(io()){
			seq.check();
			float out = seq.output();	
			io.out(0) = out;
			io.out(1) = out;	
		}
	}
	// Keyboard/mouse input callbacks
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		seq.KeyPress(k);
	}
};

int main(){
	MyApp().start();
}