#include "iostream"
#include "allocore/io/al_App.hpp"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"

using namespace al;

/*
	WaveTable, frequency modulation, synth class, mouse control
*/

class Synth{
public:
	gam::Sine<> src[13], mod;	
	gam::Osc<> osc[13];
	gam::ArrayPow2<float> table[4];	// Wavetable
	gam::ADSR<> env[13];
	
	bool Key[13], modulate;
	float freq[127], note[13],	mousePosX, mousePosY;
	int waveform, wavetable, oct;
	
	Synth(){
	std::cout << "Synthesizer!!" << "\n";
	std::cout << "controls: " << "\n";
	std::cout << "z - enable wavetable synth" << "\n";
	std::cout << "x - disable wavetable synth" << "\n";
	std::cout << "m - enable and disable FM - Use mouse to control ratio and index" << "\n";
	std::cout << "wavetable synthesis: 0 - Clausen function, 1 - Square wave, 2 - Saw wave, 3 - Triangle wave " << "\n";
	
		for(int i = 0; i <= 127; i++){
			freq[i] = (pow(2., ((i - 69) / 12.))) * 440;
		}
		// set variables
		mousePosX = 1;
		mousePosY = 1;
		oct = 5;
		waveform = 0;
		wavetable = 0;
		
		for(int i = 0; i < 13; i++){
			env[i].attack(0.5);
			env[i].decay(0.2);
			env[i].sustain(0.6);
			env[i].release(0.5);
			Key[i] = 0;
			note[i] = 0;
		}
		for(int i = 0; i < 4; i++){
			table[i].resize(2048);
		}
		
		for(int k=1; k<=16; k++){
			gam::addSine(table[0], k, 1./(k*k)); // Clausen function
			gam::addSine(table[1], 2*k-1, 1./(2*k-1)); // square wave
			gam::addSine(table[2], k, 1./k); // saw wave
			if(k%2) gam::addSine(table[3], k, 1./(k*k), 0.25); // triangle wave
		}	
	}
	// where all the magic happens.
	// Frequency, timbre and waveform
	void check(){
		if(waveform == 0){
			if(modulate){ // frequency modulation on sine
				for(int i = 0; i < 13; i++){	
					float fc = freq[oct * 12 + i];
					float ratio = mousePosX;
					float I = mousePosY;
		
					if(ratio == 0) ratio = 0.001;
		
					float fm = fc/ratio;
					mod.freq(fm);
		
					float df = mod() * fm * I;
		
					src[i].freq(fc + df);
					if(Key[i]){	
						note[i] = src[i]();	
					} 
				}		
		}else{
			for(int i = 0; i < 13; i++){
				src[i].freq(freq[oct * 12 + i]);
				if(Key[i]){	
					note[i] = src[i]();	
					} 
				}
			}
		}
	else if(waveform == 1){
		if(modulate){ // frequency modulation on osc
			for(int i = 0; i < 13; i++){	
				float fc = freq[oct * 12 + i];
				float ratio = mousePosX;
				float I = mousePosY;
		
				if(ratio == 0) ratio = 0.001;
		
				float fm = fc/ratio;
				mod.freq(fm);
	
				float df = mod() * fm * I;
			
				osc[i].source(table[wavetable]);
				osc[i].freq(fc + df);
				if(Key[i]){	
					note[i] = osc[i]();	
					} 
				}
			}else{
				for(int i = 0; i < 13; i++){
					osc[i].source(table[wavetable]);
					osc[i].freq(freq[oct * 12 + i]);
					if(Key[i]){		
						note[i] = osc[i]();	
						} 
					}
				}		
			}	
		}
	// manage the output	
	float output(float volume = 0.1){
		float out = 0;
			for(int i = 0; i < 13; i++){
				out += note[i] * env[i]();
			}
		out *= volume;	
		return out;
	}
		
	void KeyPress(const Keyboard& k){
		switch(k.key()){
		case 'a': 
			env[0].reset();
			Key[0] = 1; 
		break;
		case 'w': 
			env[1].reset();
			Key[1] = 1;
		break;
		case 's': 
			env[2].reset();
			Key[2] = 1;
		break;
		case 'e': 
			env[3].reset();
			Key[3] = 1;
		break;
		case 'd': 
			env[4].reset();
			Key[4] = 1;		
		break;
		case 'f': 				
			env[5].reset();
			Key[5] = 1;		
		break;
		case 't': 
			env[6].reset();
			Key[6] = 1;		
		break;
		case 'g': 
			env[7].reset();
			Key[7] = 1;		
		break;
		case 'y': 
			env[8].reset();
			Key[8] = 1;		
		break;
		case 'h': 
			env[9].reset();
			Key[9] = 1;		
		break;
		case 'u': 
			env[10].reset();
			Key[10] = 1;		
		break;
		case 'j': 
			env[11].reset();
			Key[11] = 1;		
		break;
		case 'k': 
			env[12].reset();
			Key[12] = 1;		
		break;
		case Keyboard::UP:
			oct++;
			if(oct>10) oct = 10;
		break;
		case Keyboard::DOWN:
			oct--;
			if(oct<0) oct = 0;
		break;
		case Keyboard::LEFT:
			oct--;
			if(oct<0) oct = 0;
		break;
		case Keyboard::RIGHT:
			oct--;
			if(oct<0) oct = 0;
		break;
		case 'z':
			waveform = 1;
			std::cout<< "Wavetable mode enabled" << "\n";
		break;
		case 'x':
			waveform = 0;
			std::cout<< "Sine mode enabled" << "\n";
		break;
		case 'm':
			if(!modulate){
				modulate = 1;
				std::cout<< "Frequency modulator enabled" << "\n";
			} else if(modulate){
				modulate = 0;
				std::cout<< "Frequency modulator disabled" << "\n";			
			}
		break;
		case '0':
			std::cout<< "Clausen function" << "\n";
			wavetable = 0;
		break;	
		case '1':
			std::cout<< "Square wave" << "\n";
			wavetable = 1;
		break;
		case '2':
			std::cout<< "Saw wave" << "\n";
			wavetable = 2;
		break;		
		case '3':
			std::cout<< "Triangle wave" << "\n";
			wavetable = 3;
		break;	
				
		}
	}
	
	void KeySlip(const Keyboard& k){
		switch(k.key()){
		case 'a': 
		env[0].release(); 
		break;
		case 'w': 
		env[1].release(); 
		break;
		case 's': 
		env[2].release(); 
		break;
		case 'e': 
		env[3].release(); 
		break;
		case 'd': 
		env[4].release(); 
		break;
		case 'f': 
		env[5].release(); 
		break;
		case 't': 
		env[6].release(); 
		break;
		case 'g': 
		env[7].release(); 
		break;
		case 'y': 
		env[8].release(); 
		break;
		case 'h': 
		env[9].release(); 
		break;
		case 'u': 
		env[10].release(); 
		break;
		case 'j': 
		env[11].release(); 
		break;
		case 'k': 
		env[12].release(); 
		break;
		}
	}
	
	void Mouse(const Mouse& m){
		mousePosX = float(m.x());
		mousePosY = float(m.y());
	}	
};

class MyApp : public App{
public:
	Synth synth;

	MyApp(){
		nav().pos(0,0,4);
		initWindow();
		initAudio();
		window().remove(navControl());
	}

	void onSound(AudioIOData& io){
		gam::sampleRate(io.fps());
		
		while(io()){
			synth.check();
		
			float out = synth.output();	
	
			io.out(0) = out;
			io.out(1) = out;	
		}
	}
	// Keyboard/mouse input callbacks
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		synth.KeyPress(k);
	}
	virtual void onKeyUp(const ViewpointWindow& w, const Keyboard& k){
		synth.KeySlip(k);		
	}
	virtual void onMouseDrag(const ViewpointWindow& w, const Mouse& m){
		synth.Mouse(m);
	}
	virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){
		synth.Mouse(m);
	}
};

int main(){
	MyApp().start();
}