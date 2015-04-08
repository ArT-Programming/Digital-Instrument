#include "iostream"
#include "al_AudioApp.hpp"
#include "allocore/io/al_App.hpp"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"

using namespace al;

/*
	WaveTable, frequency modulation, synth class, mouse control.
	Duration, pitch, frequency mod
*/

class Synth{
public:
	
	gam::Sine<> src[13], mod;	
	gam::Osc<> osc[13];
	gam::ArrayPow2<float> table[4];	// Wavetable
	gam::ADSR<> env[13];
	gam::Accum<> tmr;
	
	bool Key[13], modulate, waveform, seqOn;
	float freq[127], note[13],	mousePosX, mousePosY, seq[8], seqDuration;
	int wavetable, oct, keyAmount, counter;
	double time;
	
	Synth(){
	std::cout << "Synthesizer!!" << "\n";
	std::cout << "controls: " << "\n";
	std::cout << "z - enable and disable wavetable synth" << "\n";
	//std::cout << "x - disable wavetable synth" << "\n";
	std::cout << "m - enable and disable FM - Use mouse to control ratio and index" << "\n";
	std::cout << "wavetable synthesis control keys: 0 - Clausen function, 1 - Square wave, 2 - Saw wave, 3 - Triangle wave " << "\n";
	
		for(int i = 0; i <= 127; i++){
			freq[i] = (pow(2., ((i - 69) / 12.))) * 440;
		}
		// set variables
		mousePosX = mousePosY = seqDuration = 1;
		oct = 5;
		waveform = 0;
		wavetable = time = counter = 0;
		keyAmount = 13;
		tmr.freq(4);
		
		for(int i = 0; i < 13; i++){
			env[i].sustainDisable();
			env[i].attack(1./1000);
			env[i].decay(0.3);
			env[i].sustain(0.8);
			env[i].release(0.1);
			Key[i] = 0;
			note[i] = 0;
		}
		for(int i = 0; i < 4; i++){
			table[i].resize(2048);
		}
		for(int i = 0; i < 8; i++){
			seq[i] = 0;
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
				for(int i = 0; i < keyAmount; i++){	
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
			for(int i = 0; i < keyAmount; i++){
				src[i].freq(freq[oct * 12 + i]);
				if(Key[i]){	
					note[i] = src[i]();	
					} 
				}
			}
		}
	else if(waveform == 1){
		if(modulate){ // frequency modulation on osc
			for(int i = 0; i < keyAmount; i++){	
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
				for(int i = 0; i < keyAmount; i++){
					osc[i].source(table[wavetable]);
					osc[i].freq(freq[oct * 12 + i]);
					if(Key[i]){		
						note[i] = osc[i]();	
						} 
					}
				}		
			}
			if(seqOn){
				if(counter == 13) counter = 0;
				tmr.freq(seqDuration * 100);
				if(tmr()){
					env[counter].reset();
					counter++;
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
			//Key[0] = 1; 
			Key[0] = !Key[0];
		break;
		case 'w': 
			env[1].reset();
			Key[1] = !Key[1];
		break;
		case 's': 
			env[2].reset();
			Key[2] = !Key[2];
		break;
		case 'e': 
			env[3].reset();
			Key[3] = !Key[3];
		break;
		case 'd': 
			env[4].reset();
			Key[4] = !Key[4];
		break;
		case 'f': 				
			env[5].reset();
			Key[5] = !Key[5];
		break;
		case 't': 
			env[6].reset();
			Key[6] = !Key[6];
		break;
		case 'g': 
			env[7].reset();
			Key[7] = !Key[7];
		break;
		case 'y': 
			env[8].reset();
			Key[8] = !Key[8];
		break;
		case 'h': 
			env[9].reset();
			Key[9] = !Key[9];
		break;
		case 'u': 
			env[10].reset();
			Key[10] = !Key[10];
		break;
		case 'j': 
			env[11].reset();
			Key[11] = !Key[11];
		break;
		case 'k': 
			env[12].reset();
			Key[12] = !Key[12];
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
			if(!waveform){
				waveform = 1;
				std::cout<< "Wavetable mode enabled" << "\n";
			} else if (waveform){
				waveform = 0;
				std::cout<< "Sine mode enabled" << "\n";
			}
		break;
		case 'x':
				if(!seqOn){
				seqOn = 1;
				std::cout<< "Sequencer mode enabled" << "\n";
			} else if (seqOn){
				seqOn = 0;
				std::cout<< "Sequencer mode disabled" << "\n";
			}
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
	
	void Mouse(const ViewpointWindow& w, const Mouse& m){
		seqDuration = float(float(m.x()) / w.width());
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
		synth.time += io.secondsPerBuffer();
	
		while(io()){
			synth.check();
			//synth.sequencer();
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
	virtual void onMouseMove(const ViewpointWindow& w, const Mouse& m){
		synth.Mouse(w, m);
	}
};

int main(){
	MyApp().start();
}