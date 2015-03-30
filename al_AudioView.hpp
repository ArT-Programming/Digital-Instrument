#ifndef INCLUDE_AL_AUDIO_VIEW_HPP
#define INCLUDE_AL_AUDIO_VIEW_HPP

#include "GLV/glv.h"
#include "Gamma/FFT.h"
#include "Gamma/Containers.h"
#include "Gamma/Types.h"

namespace glv{

inline float linLog2(float v, float recMin = 1./16){
	v = ::log2(::fabsf(v) + 0.000001f);	// offset to avoid -inf
	v *= recMin;
	return v > -1.f ? v + 1.f : 0.f;
}



class RasterScope : public Plot{
public:

	struct WhiteHeat : GraphicsMap{
		void onMap(GraphicsData& gd, const Data& d, const Indexer& i){
			Color col = gd.colors()[0];
			while(i()){
				float w0 = d.at<float>(0,i[0],i[1],i[2]);
				w0 = ::fabs(w0);
				Color c = col.mixRGB(Color(1), w0);
				c.scale(w0);
				gd.addColor(c);
			}
		}
	} whiteHot;

	RasterScope(const glv::Rect& r=glv::Rect(0), int sizeX=0, int sizeY=0)
	:	Plot(r), mWritePos(-1), mRowsUpdated(0)
	{
		mPlotDensity.remove(mPlotDensity);
		mPlotDensity.add(whiteHot);
		add(mPlotDensity);
		mPlotDensity.useStyleColor(true);
		//mPlotDensity.interpolate(1);
		resize(sizeX, sizeY);
	}

	void resize(int sizeX, int sizeY){
		data().resize(glv::Data::FLOAT, 1, sizeX, sizeY);
		//data().print();
	}

	template <int Stride, class T>
	void writeRow(const T* src, int len){
		++mWritePos;
		if(mWritePos >= data().size(2)) mWritePos=0;

		int n = glv::min(len, data().size(1));
		for(int i=0; i<n; ++i){
			data().Data::elem<float>(0,i,mWritePos) = src[i*Stride];
		}
		
		++mRowsUpdated;
	}
	
	virtual void onDraw(GLV& g){

		// Updating only changed regions seems no faster...
		/*
		mRowsUpdated = glv::min(mRowsUpdated, data().size(2));
	
		int ty = mWritePos - mRowsUpdated;
		int th = mRowsUpdated;
		if(ty < 0){
			ty += data().size(2);
			th = data().size(2) - ty;
		}
		
		//printf("%d + %d\n", ty, th);
		
		mPlotDensity.texture().updateRegion(0, ty, -1, th);
		mRowsUpdated -= th;
		//*/
		
		Plot::onDraw(g);
	}

	const char * className() const { return "RasterScope"; }

protected:
	int mWritePos;
	int mRowsUpdated;
	PlotDensity mPlotDensity;
};



class FreqScope : public Plot{
public:
	FreqScope(const glv::Rect& r=glv::Rect(0), int n=512)
	{
		minLog(-12);
		range(0,1, 1);
		minor(12, 0);
		//minor(1, 1);
		//rangeY(0,1);
		//showAxes(false);
		//mPlot1D.pathStyle(PlotFunction1D::ZIGZAG);
		//mPlot1D.prim(draw::TriangleStrip);
		mPlot1D.useStyleColor(true);
		add(mPlot1D);
		sizeDFT(n);
	}

	void sizeDFT(int n){
		mFFT.resize(n);
		mBuf.resize(n*2); // twice since complex
		
		int numBins = n/2; // only real, for now
		data().set(const_cast<float*>(mBuf.readBuf().elems()), 1, numBins).stride(2);
		//data().set(const_cast<float*>(mBuf.back()), 1, numBins).stride(2);
		range(-1, numBins+1, 0);
		//major(numBins/12., 0);
		major(numBins, 0);
	}

	int sizeDFT() const { return mFFT.size(); }

	FreqScope& sampleRate(float SR, float freqSpacing = 1000){
		minor(SR*0.5/freqSpacing, 0);
		return *this;
	}

	FreqScope& minLog(float v){
		mMinLog = -1./v;
		major(mMinLog, 1);
		minor(v, 1);
		return *this;
	}

	bool update(float * samples, int frames){
		bool res = false;
		for(int i=0; i<frames; ++i){
			res |= (*this)(samples[i]);
		}
		return res;
	}

	
	const float * spectrum() const {
		return mBuf.readBuf().elems();
	}


	bool operator()(float v){
		if(!enabled(glv::Animate)) return false;
	
		mBuf(v);
		mBuf(0);
		//mBuf.writeFront(v);
		//mBuf.writeFront(0);
		
		if(mBuf.reachedEnd()){
			float * buf = mBuf.elems();
			//float * buf = mBuf.front();
			
			// Apply window.
			// We approximate the actual function with a cubic function since it
			// matches the actual function very closely and takes considerably
			// less time to compute. Small numerical errors are not so important
			// since this is only for visualization. 
			unsigned N   = mBuf.size();
			unsigned N_2 = N/2;
			double winArea = 0; // area under window used for gain compensation
			for(unsigned i=0; i<N_2; i+=2){
				float x = float(i)/N_2; // in (0, 1]
				//float w = 1;
				//float w = 0.50 - 0.50*cos(x*M_PI); // Hann
				//float w = 0.54 - 0.46*cos(x*M_PI); // Hamming
				//float w = x*x*(3.f - 2.f*x); // Hann cubic apx
				float w = x*x*(3.f*.92f - 2.f*.92f*x) + .08f; // Hamming cubic apx
				//float w = x*(1.5f - .5f*x*x); // Cosine cubic apx #1 (better fit)
				//float w = x*(2.f - x); // Cosine cubic apx #2 (parabolic)
				buf[i+0] *= w;
				buf[i+1] *= w;
				buf[N-1-(i+0)] *= w;
				buf[N-1-(i+1)] *= w;
				
				winArea += w;
			}
			
			mFFT.forward(buf, true, N_2 / (winArea*2.));

			// convert to polar
			for(unsigned i=2; i<mBuf.size()-2; i+=2){
				
				//gam::Complex<float>& c = *(gam::Complex<float> *)(mBuf.elems() + i);
				gam::Complex<float>& c = *(gam::Complex<float> *)(buf + i);
				
				float m = gam::scl::sqrt<1>(c.magSqr());
				m += m; // add negative frequency amplitude
				m = gam::scl::linLog2(m, mMinLog);
				
				c(m, c.arg());
				//c(float(i)/mBuf.size(), float(i)/mBuf.size());
			}

			mBuf.copy();	// this chucks the FFT results into the second
							// buffer for drawing
			//mBuf.swap();
			//data().set(const_cast<float*>(mBuf.back()), 1, mBuf.size()/2).stride(2);
			return true;
		}
		return false;
	}

	bool onEvent(Event::t e, GLV& g){
		const Keyboard& k = g.keyboard();
		switch(e){
		case Event::KeyDown:
			switch(k.key()){
			case ' ': toggle(glv::Animate); return false;
			default:;
			}
		default:;
		}
		
		return Plot::onEvent(e,g);
	}
	
	const char * className() const { return "FreqScope"; }

protected:

	class DoubleBuffer{
	public:
		DoubleBuffer(int sz=0){
			resize(sz);
		}

		int size() const { return mBuf.size(); }

		void writeFront(float v){
			++mWritePos;
			if(mWritePos == size()) mWritePos=0;
			mFront[mWritePos] = v;
		}
		
		bool reachedEnd() const {
			return mWritePos == (size()-1);
		}

		float * back(){ return mBack; }
		float * front(){ return mFront; }

		void swap(){
			float * temp = mBack;
			mBack = mFront;
			mFront= temp;
		}

		void resize(int n){
			mBuf.resize(n*2);
			mFront= &mBuf[0];
			mBack = &mBuf[n];
			mWritePos = -1;
		}

	private:
		std::vector<float> mBuf;
		float * mFront;
		float * mBack;
		int mWritePos;
	};

	float mMinLog;
	gam::CFFT<float> mFFT;
	gam::DoubleRing<float> mBuf;
	//DoubleBuffer mBuf;
	PlotFunction1D mPlot1D;
};



class AudioView : public View {
public:

	AudioView(const Rect& r = Rect(400,200), int dftSize = 1024)
	:	View(r), mFreqZoom(1)
	{
		mFreqScope.sizeDFT(dftSize);
		mTimeScope.resize(1024*1, 1);
		mSonogram.resize(dftSize/2+1, 128);

		mTimeScope.stretch(1,0.3).anchor(0,0.0);
		mFreqScope.stretch(1,0.3).anchor(0,0.3);
		mSonogram .stretch(1,0.4).anchor(0,0.6);

		mTimeScope.disable(glv::DrawBorder | glv::FocusToTop);
		mFreqScope.disable(glv::DrawBorder | glv::FocusToTop);
		mSonogram .disable(glv::DrawBorder | glv::FocusToTop);

		mFreqScope.showAxis(false);
		//mTimeScope.plottables()[0]->stroke(2);
		//mFreqScope.plottables()[0]->stroke(2);

		mTimeScope
			.lockZoom(true,0).lockZoom(true,1)
			.lockScroll(true,0).lockScroll(true,1);
		mFreqScope
			.lockZoom(true,0).lockZoom(true,1)
			.lockScroll(true,0).lockScroll(true,1);
		mSonogram
			.lockZoom(true,0).lockZoom(true,1)
			.lockScroll(true,0).lockScroll(true,1);

		(*this) << mTimeScope << mFreqScope << mSonogram;
	}

//	/// Add next audio sample
//	void operator()(float v){
//		mFreqScope(v);
//	}

	void update(float * samples, int frames, int chans){
		if(mFreqScope.update(samples, frames)){
			mSonogram.writeRow<2>(mFreqScope.spectrum(), mFreqScope.sizeDFT()/2);
		}
		mTimeScope.update(samples, frames, chans);
	}

	TimeScope& timeScope(){ return mTimeScope; }
	FreqScope& freqScope(){ return mFreqScope; }
	RasterScope& sonogram(){return mSonogram; }


	void tileChildren(int nx=1, int ny=-1){
	
		struct F : TraversalAction{
			const AudioView& outer;
			F(const AudioView& v): outer(v){}
			bool operator()(View * v, int depth){
				return v->enabled(Visible) 
					&& (v==&outer.mTimeScope || v==&outer.mFreqScope || v==&outer.mSonogram);
			}
		};
		
		std::vector<View *> tilables;
		F pred(*this);
		getChildren(tilables, pred);
		//printf("%d\n", tilables.size());
	
		if(ny < 1 && nx > 0){
			ny = tilables.size() / nx;
		}
		else{
			return;
		}
	
		float fx = 1./nx;
		float fy = 1./ny;
		int x = 0, y = 0;
		for(unsigned i=0; i<tilables.size(); ++i){
			View& v = *tilables[i];
			float cw = fx * width();
			float ch = fy * height();
			v.anchor(x*fx, y*fy).stretch(fx,fy);
			v.Rect::set(x*cw, y*ch, cw, ch);
			++y;
			if(y == ny){
				y=0;
				++x;
			}
		}
	}

	bool onEvent(Event::t e, GLV& g){
		switch(e){
		case Event::KeyDown:
			switch(g.keyboard().key()){
			case 't': mTimeScope.toggle(Visible); tileChildren(); return false;
			case 'f': mFreqScope.toggle(Visible); tileChildren(); return false;
			case 'c': mSonogram.toggle(Visible); tileChildren(); return false;
			case 'h': 
				if(1 == mFreqZoom){
					mFreqZoom=2;
					mFreqScope.interval(0).scale(0.5, 0);
					mSonogram .interval(0).scale(0.5, 0);
				}
				else{
					mFreqZoom=1;
					mFreqScope.interval(0).scale(2.0, 0);
					mSonogram .interval(0).scale(2.0, 0);
				}
				return false;
			}
		default:;
		}
		return true;
	}

	const char * className() const { return "AudioView"; }

private:
	TimeScope mTimeScope;
	FreqScope mFreqScope;
	RasterScope mSonogram;
	int mFreqZoom;
};

} // glv::

#endif
