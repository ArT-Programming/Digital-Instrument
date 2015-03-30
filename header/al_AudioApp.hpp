#include "allocore/io/al_App.hpp"
#include "allocore/io/al_MIDI.hpp"
#include "alloGLV/al_ControlGLV.hpp"
#include "GLV/glv.h"
#include "al_AudioView.hpp"

namespace al{

class AudioApp : public App, public MIDIMessageHandler {
public:
	GLVBinding gui;
	glv::AudioView av;
	MIDIIn midiIn;

	AudioApp()
	:	av(glv::Rect(0), 1024), mScopeAudioCallback(*this)
	{
		// Check for connected MIDI devices
		if(midiIn.getPortCount() > 0){
			MIDIMessageHandler::bindTo(midiIn);
			// Open the first device found
			midiIn.openPort(0);
			printf("Opened MIDI port to %s\n", midiIn.getPortName(0).c_str());
		}

		initWindow(Window::Dim(600,400));
		window().remove(navControl());
		av.stretch(1,1);
		//gui.colors().set(glv::HSV(0.6,0.9,1), 0.92);
		gui.colors().set(glv::HSV(0.15,0.8,1), 0.92);
		gui << av;
		gui.setFocus(&av);
		gui.bindTo(window());
		
		audioIO().append(mScopeAudioCallback);
	}

	// override pure virtual
	void onMIDIMessage(const MIDIMessage& m){
		//m.print();
	}

private:
	class ScopeAudioCallback : public AudioCallback {
	public:
		AudioApp& outer;
		ScopeAudioCallback(AudioApp& o): outer(o){}
		virtual void onAudioCB(AudioIOData& io){
			outer.av.freqScope().sampleRate(io.fps());
			outer.av.update(
				io.outBuffer(), io.framesPerBuffer(), io.channelsOut());
		}
	};
	
	ScopeAudioCallback mScopeAudioCallback;
};

}
