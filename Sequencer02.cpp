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
	static const int cols = 16; // |||
	static const int rows = 16; // ---
	gam::Sine<> src[rows];
	gam::ADSR<> env[rows],colorEnv[rows];
	gam::Accum<> tmr;
	
	Mesh quad;

	bool KeyOn[cols][rows];
	int scale[rows], counter;
	float color[rows], size;
	
	
	int penta[5], dorian[7];
	
	// scales found at: http://www.grantmuller.com/MidiReference/doc/midiReference/ScaleReference.html 
	Sequencer():
	penta{0, 2, 4, 7, 9},
	dorian{0, 2, 3, 5, 7, 9, 10}
	{
		counter = 0;
		tmr.freq(4);
		
		for(int r = 0; r < rows; r++){
			scale[r] = 60 + penta[r%5] + 12*(r/5);
			src[r].freq((pow(2., ((scale[r] - 69) / 12.))) * 440);
			
			env[r].sustainDisable();
			env[r].attack(0);
			env[r].decay(0);
			env[r].sustain(0);
			env[r].release(0);
			
			colorEnv[r].sustainDisable();
			colorEnv[r].attack(1./1000);
			colorEnv[r].decay(0.1);
			colorEnv[r].sustain(0.8);
			colorEnv[r].release(.15);
		}
		
		for(int c = 0; c < cols; c++){
			for(int r = 0; r < rows; r++){
				KeyOn[c][r] = false;
			}
		}
		
		quad.primitive(Graphics::QUADS);
		quad.vertex(-0.5, -0.5);
		quad.vertex( 0.5, -0.5);
		quad.vertex( 0.5,  0.5);
		quad.vertex(-0.5,  0.5);
	}
	
	void check(){
		counter++;
		if(counter == cols) counter = 0;
		for(int r = 0; r < rows; r++){
			if(KeyOn[counter][r]){
				env[r].reset();
				colorEnv[r].reset();
			}
		}
	}
	
	// manage the output	
	float output(float volume = .1){
		float out = 0;
		for(int r = 0; r < rows; r++){
			color[r] = colorEnv[r]();
			out += src[r]() * env[r]();
		}
		out *= volume;
		for(int r = 0; r < rows; r++){
			env[r].attack(1./1000);
			env[r].decay(0.05);
			env[r].sustain(0.1);
			env[r].release(3);
		}
		return out;
	}
	
	void KeyPress(const Keyboard& k){
		switch(k.key()){
		case 'c': //clear
			for(int c = 0; c < cols; c++){
				for(int r = 0; r < rows; r++){
					KeyOn[c][r] = false; 
				}
			}
			break;
		
		case 'r': //random
			for(int c = 0; c < cols; c++){
				for(int r = 0; r < rows; r++){
					if(rand() % int(rows * 1.5) == 0)	KeyOn[c][r] = true; 
					else KeyOn[c][r] = false;
				}
			}
			counter = -1;
			break;
			
		case 'p': //pentatonic scale
			for(int r = 0; r < rows; r++){
				scale[r] = 60 + penta[r%5] + 12*(r/5);
				src[r].freq((pow(2., ((scale[r] - 69) / 12.))) * 440);
			}
			break;
			
		case 'd': //turkish scale
			for(int r = 0; r < rows; r++){
				scale[r] = 60 + dorian[r%7] + 12*(r/7);
				src[r].freq((pow(2., ((scale[r] - 69) / 12.))) * 440);
			}
			break;
		}
	}
	
	void getMousePos(const ViewpointWindow& w, const Mouse& m, Vec3d cam){
		// We have narrowed the camera lens constant down to be 0.26795
		float cX = cam.z * 0.26795 * w.aspect();
		float cY = cam.z * 0.26795;
		
		// Convert the pixel values to coordinates
		float mouseX = ( (2. * cX / (cX - cam.x)) * m.x() / w.width()  - 1) * (cX - cam.x);
		float mouseY = (-(2. * cY / (cY + cam.y)) * m.y() / w.height() + 1) * (cY + cam.y);
		
		
		
		for(int c = 0; c < cols; c++){
			for(int r = 0; r < rows; r++){
				float x = (c * 2 * cX / cols) - (cX) + (cX/(cols));
				float y = (r * 2 * cY / rows) - (cY) + (cY/(rows));
				if(mouseX > x - size/2
				&& mouseX < x + size/2
				&& mouseY > y - size/2
				&& mouseY < y + size/2){
					KeyOn[c][r] = !KeyOn[c][r];
				}
			}
		}
	}
	
	void draw(Graphics& g, const Viewpoint& v, Vec3d cam){
		// Get the coordinates of the window
		double width = 2 * cam.z * 0.26795 * v.viewport().aspect(); 
		double height = 2 * cam.z * 0.26795;
		size = min(width / cols, height / rows);
		size -= 0.05;
		for(int c = 0; c < cols; c++){
			for(int r = 0; r < rows; r++){
				
				g.pushMatrix(Graphics::MODELVIEW);
				if(KeyOn[c][r]){
					if(counter == c) g.color(HSV(1/3., fabs(0.7*color[r]-0.7), 0.7+color[r]*0.3));
					else g.color(HSV(1/3., 0.7, 0.7));
				}
				else g.color(HSV(0,0,0.3));
				float x = (c * width / cols) - (width/2) + (width/(cols*2));
				float y = (r * height / rows) - (height/2) + (height/(rows*2));
				g.translate(x,y);
				g.scale(size);
				g.draw(quad);
				g.popMatrix();
			}
		}
	}
};

class MyApp : public App{
public:
	Sequencer seq;
	Vec3d cam;

	MyApp(){
		nav().pos(0,0,5);
		cam = nav().pos();
		initWindow();
		initAudio();
		window().remove(navControl());
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
	// Keyboard/mouse input callbacks
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		seq.KeyPress(k);
	}
	virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){
		seq.getMousePos(w, m, cam);
	}
	void onDraw(Graphics& g, const Viewpoint& v){
		seq.draw(g,v,cam);
	}
};

int main(){
	srand(time(0));
	MyApp().start();
}