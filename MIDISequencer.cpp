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

class Synth {
public:
	gam::Sine<> src[8], mod, car[128];	
	gam::Osc<> osc[128], carOsc[128];
	gam::ArrayPow2<float> table[4];	// Wavetable
	gam::ADSR<> env[8];
	gam::Accum<> tmr;

	
	bool modulate, KeyOn[8][8], KeyOff[128], waveform, seqOn;
	float freq[128], note[128], velocity[128], oldVelo[128], mousePosX, mousePosY, seqDuration;
	int wavetable;
	int keyAmount, noteOn, noteOff, counter, pentaScale[8];
	
	Synth(){
		std::cout << "MIDISynthesizer!!" << "\n";
		std::cout << "controls: " << "\n";
		std::cout << "z - enable and disable wavetable synth" << "\n";
		std::cout << "m - enable and disable FM - Use mouse to control ratio and index" << "\n";
		std::cout << "wavetable synthesis: 0 - Clausen function, 1 - Square wave, 2 - Saw wave, 3 - Triangle wave " << "\n";
		//keyAmount = 127;
		noteOn = noteOff = modulate = waveform = seqOn = 0;
		for(int i = 0; i <= keyAmount; i++){
			freq[i] = (pow(2., ((i - 69) / 12.))) * 440;
			env[i].sustainDisable();
			env[i].attack(1./1000);
			env[i].decay(1./1000);
			env[i].sustain(0.8);
			env[i].release(0.2);
			KeyOn[i] = 0;
			KeyOff[i] = 1;
			note[i] = 0;
			velocity[i] = 0;
			oldVelo[i] = 0;
			src[i].freq(freq[i]);
			osc[i].freq(freq[i]);
		}
		tmr.freq(20);
		// set variables
		mousePosX = 1;
		mousePosY = 1;
		wavetable = 0;
		
		//pentaScale[] = {69, 72, 74, 76, 79, 81, 84, 86}
		
		
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
		for(int i = 0; i <= keyAmount; i++){
				if(KeyOn[i]){	
					note[i] = src[i]();	
					}
				}
			//if(seqOn){
				if(counter == 72) counter = 64;
					//std::cout << "hallo" << "\n";
					//tmr.freq(seqDuration * 100 + 20);
				if(tmr()){
					//env[counter].reset();
					//std::cout << "hallo" << "\n";
					counter++;
					}
				for(int i = 0; i < 8; i++){
					if(KeyOn[counter][i]) env[i].reset();
				}						
				//}
		}
	// manage the output	
	float output(float velocity[], float volume = 0.1){
		float out = 0;
			for(int i = 0; i < 8; i++){
					out += note[i]  * env[i]();
				
			}
		out *= volume;	
		return out;
	}
		
	void KeyPress(const Keyboard& k){
		switch(k.key()){
		
		case 'w':
			if(!waveform){
				waveform = 1;
				std::cout<< "Wavetable mode enabled" << "\n";
			} else if (waveform){
				waveform = 0;
				std::cout<< "Sine mode enabled" << "\n";
			}
		break;
		case 's':
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
	
	void Mouse(const ViewpointWindow& w, const Mouse& m){
		seqDuration = float(float(m.x()) / w.width());
		mousePosX = float(m.x());
		mousePosY = float(m.y());
	}	

	void MIDI(const MIDIMessage& m){

		// Here we demonstrate how to parse common channel messages
		switch(m.type()){
		case MIDIByte::NOTE_ON:
			noteOn = m.noteNumber();
			velocity[noteOn] = m.velocity();
		if(velocity[noteOn] == 1){
			KeyOn[noteOn/16][noteOn%16] = !KeyOn[noteOn/16][noteOn%16];
			std::cout << "y: " << noteOn/16 << "   x: " << noteOn%16 << "\n";
			//KeyOff[noteOn] = !KeyOff[noteOn];
			
				} 
					break;
		}
	}	
	
};

class MyApp : public App, public MIDIMessageHandler{
public:
	Synth synth;
	MIDIIn midiIn;

	MyApp(){
		nav().pos(0,0,4);
		//initWindow();
		initAudio();
		//window().remove(navControl());
		
		if(midiIn.getPortCount() > 0){

			// Bind ourself to the MIDIIn
			MIDIMessageHandler::bindTo(midiIn);

			// Open the last device found
			int port = midiIn.getPortCount()-1;
			midiIn.openPort(0);
			printf("Opened port to %s\n", midiIn.getPortName(port).c_str());
		}
		else{
			printf("Error: No MIDI devices found.\n");
		}
	}

	void onSound(AudioIOData& io){
		gam::sampleRate(io.fps());
		
		while(io()){
			synth.check();
		
			float out = synth.output(synth.velocity);	
	
			io.out(0) = out;
			io.out(1) = out;	
		}
	}
	// Keyboard/mouse input callbacks
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		synth.KeyPress(k);
	}

	virtual void onMouseMove(const ViewpointWindow& w, const Mouse& m){
		synth.Mouse(w, m);
	}

	void onMIDIMessage(const MIDIMessage& m){
		synth.MIDI(m);	
	}
};

int main(){
	MyApp().start();
}