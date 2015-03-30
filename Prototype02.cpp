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
	Notes note;
	Synth fmSynth[2];
	Mesh quad;
    gam::Biquad<> hpf, lpf;
	
	gam::SamplePlayer<> beat;
	
	MyApp():
	values(2,2,3,"COM3",19200)
	{
		quad.primitive(Graphics::QUADS);
		quad.color(RGB(1));
        quad.vertex(-1, 0,  0.5);
        quad.vertex( 1, 0,  0.5);
        quad.vertex( 1, 0, -0.5);
        quad.vertex(-1, 0, -0.5);
		
		hpf.type(gam::HIGH_PASS);
		lpf.type(gam::LOW_PASS);
		
		beat.load(RUN_MAIN_SOURCE_PATH "soundfiles/beat01.wav");
		
		initAudio();
	}

	// Audio callback
	virtual void onSound(AudioIOData& io){	
		gam::sampleRate(io.fps());
		beat.loop();
		/*int hpCutoff = pow(((arduino[0].angleX / 90.) * 2.78),10); // 0 grader = 0 Hz   	 -- 90 grader = 22050 Hz
		int lpCutoff = pow((((arduino[0].angleX / 90.) + 1) * 2.78),10);// 0 grader = 22050 Hz -- -90 grader = 0 Hz
		
		if(hpCutoff < 0) hpCutoff = 0;
		if(lpCutoff < 0) lpCutoff = 0;
		if(lpCutoff > 22050) lpCutoff = 22050;
		if(hpCutoff > 22050) hpCutoff = 22050;
		
		hpf.freq(hpCutoff);
		lpf.freq(lpCutoff);
		
		float beatRate = int((1 + arduino[0].angleY / 90.) * 100)/100.;
		
		//std::cout<<"hpf "<<hpCutoff<<"        lpf "<<lpCutoff<<"        beatRate "<<beatRate<<"\n";
		int n = 5;
		if(arduino[1].angleY > 81) n = 0;
		else if(arduino[1].angleY > 63) n = 1;
		else if(arduino[1].angleY > 45) n = 2;
		else if(arduino[1].angleY > 27) n = 3;
		else if(arduino[1].angleY > 9) n = 4;
		else if(arduino[1].angleY > -9) n = 5;
		else if(arduino[1].angleY > -27) n = 6;
		else if(arduino[1].angleY > -45) n = 7;
		else if(arduino[1].angleY > -63) n = 8;
		else if(arduino[1].angleY > -81) n = 9;
		
		note.calcFreq(n);
		
		beat.rate(beatRate / 2.);*/
		
		
		//int n = 5;
		/*if(arduino[0].angleY > 81) n = 0;
		else if(arduino[0].angleY > 63) n = 1;
		else if(arduino[0].angleY > 45) n = 2;
		else if(arduino[0].angleY > 27) n = 3;
		else if(arduino[0].angleY > 9) n = 4;
		else if(arduino[0].angleY > -9) n = 5;
		else if(arduino[0].angleY > -27) n = 6;
		else if(arduino[0].angleY > -45) n = 7;
		else if(arduino[0].angleY > -63) n = 8;
		else if(arduino[0].angleY > -81) n = 9;*/
		
		float index = ((arduino[0].angleX + 90) / 180.) * 20;
		float ratio = ((arduino[0].angleX + 90) / 180.) * 5;
		float n = ((arduino[0].angleY + 90) / 180.) * 100;
		//std::cout<<n<<"\n";
		float fc = note.penta(n); 
		float fc2 = note.penta(n+7);
		
		//float fc = 220;
		//float fc2 = 290;
		
		//float index2 = ((arduino[1].angleY + 90) / 180.) * 20;
		//float ratio2 = ((arduino[1].angleX + 90) / 180.) * 5;
		//float fc2 = ((arduino[1].angleY + 90) / 180.) * 1000 + 100; // note.penta(n); 
		while(io()){
			//float b = beat() * 2;
			//float out2 = fmSynth[1].modulate(fc2, ratio2, index2);
			float out = fmSynth[0].modulate(fc, 1, index) + fmSynth[1].modulate(fc2, 1, index);
			
			io.out(0) = out * 0.8;
			io.out(1) = out * 0.8;
		}
	} 

	// Graphics callbacks
	virtual void onAnimate(double dt){
		values.unpack();
		for(int i = 0; i < 2; i++){
			arduino[i].getValues(i, values.arduino);
			arduino[i].setAngles(dt);
			//std::cout<<arduino[i].gyro<<"       "<<arduino[i].acc<<"\n";
		}
		//std::cout<<"\n";
	}
	
	/*virtual void onDraw(Graphics& g, const Viewpoint& v){
		g.pushMatrix(Graphics::MODELVIEW);
        g.translate(0,-0.5,0);
        //g.rotate(arduino.angleZ, 0,1,0);
        g.rotate(-arduino[0].angleY, 1,0,0);
        g.rotate(-arduino[0].angleX, 0,0,1);
        
        g.draw(quad);
        
        g.popMatrix();
		
		g.pushMatrix(Graphics::MODELVIEW);
        g.translate(0,0.5,0);
        //g.rotate(arduino.angleZ, 0,1,0);
        g.rotate(-arduino[1].angleY, 1,0,0);
        g.rotate(-arduino[1].angleX, 0,0,1);
        
        g.draw(quad);
        
        g.popMatrix();
	}*/
};


int main(){
	MyApp().start();
}