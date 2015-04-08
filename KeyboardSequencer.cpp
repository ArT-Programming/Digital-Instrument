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
	
	Mesh quad[8][8];

	bool KeyOn[8][8], start;
	int pentaScale[8], counter;
	float color[8];
	
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
		
		for(int i = 0; i < 8; i++){
			src[i].freq((pow(2., ((pentaScale[i] - 69) / 12.))) * 440);
			env[i].sustainDisable();
			env[i].attack(1./1000);
			env[i].decay(0.05);
			env[i].sustain(0.5);
			env[i].release(0.2);
			for(int j = 0; j < 8; j++){
				KeyOn[i][j] = false;
				quad[i][j].primitive(Graphics::QUADS);
				quad[i][j].vertex(2.*(i/7.) - 1.0 , 2.*(j/7.) - 1.0);
				quad[i][j].vertex(2.*(i/7.) - 0.8 , 2.*(j/7.) - 1.0);
				quad[i][j].vertex(2.*(i/7.) - 0.8 , 2.*(j/7.) - 0.8);
				quad[i][j].vertex(2.*(i/7.) - 1.0 , 2.*(j/7.) - 0.8);
			}
		}
		
		
	}
	
	void check(){
		counter++;
		if(counter == 8) counter = 0;
		for(int i = 0; i < 8; i++){
			if(KeyOn[counter][i]) env[i].reset();
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
	/*
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
	*/
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
						KeyOn[i][j] = !KeyOn[i][j];
						
					}
				}
			}
		}
	}
	
	void draw(Graphics& g){
		for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++){
				if(counter == j) g.color(RGB(color[i]+0.1));
				else g.color(RGB(0.1));
				g.draw(quad[j][i]);
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
	/*// Keyboard/mouse input callbacks
	virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
		seq.KeyPress(k);
	}*/
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