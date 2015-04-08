#include "iostream"
#include "allocore/io/al_App.hpp"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"
#include "allocore/io/al_MIDI.hpp"
#include <stdio.h>

using namespace al;

class Sequencer{
public:
	gam::Sine<> src[8];
	gam::ADSR<> env[8];
	gam::Accum<> tmr;
	
	Mesh quad[8][8];

	bool KeyOn[8][8], start;
	int pentaScale[8], counter, noteOn;
	float color[8], velocity[128];
	
	Sequencer(){
		counter = 0;
		start = false;
		tmr.freq(4);
		pentaScale[0] = 69;
		pentaScale[1] = 72;
		pentaScale[2] = 74;
		pentaScale[3] = 76;
		pentaScale[4] = 79;
		pentaScale[5] = 81;
		pentaScale[6] = 84;
		pentaScale[7] = 86;
		noteOn = 0;
		
		for(int i = 0; i < 128; i++){
			velocity[i] = 0;
		}
		
		for(int i = 0; i < 8; i++){
			src[i].freq((pow(2., ((pentaScale[i] - 69) / 12.))) * 440);
			env[i].sustainDisable();
			env[i].attack(1./1000);
			env[i].decay(0.05);
			env[i].sustain(0.5);
			env[i].release(0.2);
			for(int j = 0; j < 8; j++){
				KeyOn[j][i] = false;
				quad[j][i].primitive(Graphics::QUADS);
				quad[j][i].vertex(2.*(j/7.) - 1.0 , 2.*(i/7.) - 1.0);
				quad[j][i].vertex(2.*(j/7.) - 0.8 , 2.*(i/7.) - 1.0);
				quad[j][i].vertex(2.*(j/7.) - 0.8 , 2.*(i/7.) - 0.8);
				quad[j][i].vertex(2.*(j/7.) - 1.0 , 2.*(i/7.) - 0.8);
			}
		}		
	}
	
	void check(){
		counter++;
		if(counter == 8) counter = 0;
		for(int i = 0; i < 8; i++){
			if(KeyOn[i][counter]) env[i].reset();
		}
	}
	
	// manage the output	
	float output(float volume = 0.1){
		float out = 0;
		for(int i = 0; i < 8; i++){
			color[i] = env[i]();
			out += src[i]() * color[i];
			
		}
		out *= volume * start;	
		return out;
	}

	void getMousePos(const ViewpointWindow& w, const Mouse& m, Vec3d cam){
		start = true;
		// We have narrowed the camera lens constant down to be 0.26795
		float cX = cam.z * 0.26795 * w.aspect();
		float cY = cam.z * 0.26795;
		
		// Convert the pixel values to coordinates
		float mouseX = ( (2. * cX / (cX - cam.x)) * m.x() / w.width()  - 1) * (cX - cam.x);
		float mouseY = (-(2. * cY / (cY + cam.y)) * m.y() / w.height() + 1) * (cY + cam.y);
		
		
		
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++){
				if(mouseX > quad[i][j].vertices()[0].x && mouseX < quad[i][j].vertices()[2].x){
					if(mouseY > quad[i][j].vertices()[0].y && mouseY < quad[i][j].vertices()[2].y){
						KeyOn[j][i] = !KeyOn[j][i];
					}
				}
			}
		}
	}
	
	void draw(Graphics& g){
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++){
				if(KeyOn[j][i]){
					if(counter == i) g.color(HSV(0, 0, 0.5+color[j]*0.5));
					else g.color(HSV(0, 0, 1));
				}
				else g.color(HSV(0,0,0.3));
				g.draw(quad[i][j]);
			}
		}
	}
	void MIDI(const MIDIMessage& m){

		// Here we demonstrate how to parse common channel messages
		switch(m.type()){
		case MIDIByte::NOTE_ON:	noteOn = m.noteNumber(); velocity[noteOn] = m.velocity();
			if(velocity[noteOn] == 1){
				KeyOn[7 - noteOn/16][noteOn%16] = !KeyOn[7 - noteOn/16][noteOn%16];	
			} break;
		}
	}	
};

class MyApp : public App, public MIDIMessageHandler{
public:
	Sequencer seq;
	MIDIIn midiIn;
	Vec3d cam;

	MyApp(){
		nav().pos(0,0,5);
		cam = nav().pos();
		initWindow();
		initAudio();
		window().remove(navControl());
		
		if(midiIn.getPortCount() > 0){
			
			// Bind ourself to the MIDIIn
			MIDIMessageHandler::bindTo(midiIn);

			// Open the last device found
			int port = midiIn.getPortCount()-1;
			midiIn.openPort(port);
			printf("Opened port to %s\n", midiIn.getPortName(port).c_str());
		}
		else printf("Error: No MIDI devices found.\n");	
	}

	void onSound(AudioIOData& io){
		gam::sampleRate(io.fps());
		
		while(io()){
			if(seq.tmr()) seq.check();
			float out = seq.output();	
	
			io.out(0) = out;
			io.out(1) = out;	
		}
	}

	void onMIDIMessage(const MIDIMessage& m){
		seq.MIDI(m);	
	}
	virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){
		seq.getMousePos(w, m, cam);
	}
	void onDraw(Graphics& g, const Viewpoint& v){
		seq.draw(g);
	}
};

int main(){
	MyApp().start();
}