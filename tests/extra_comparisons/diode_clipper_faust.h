/* ------------------------------------------------------------
name: "diode_clipper"
Code generated with Faust 2.83.1 (https://faust.grame.fr)
Compilation options: -lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __mydsp_H__
#define  __mydsp_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif


class mydsp : public dsp {
	
 private:
	
	float fRec0[2];
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	float fConst6;
	float fConst7;
	float fVec0[2];
	float fRec1[2];
	
 public:
	mydsp() {
	}
	
	mydsp(const mydsp&) = default;
	
	virtual ~mydsp() = default;
	
	mydsp& operator=(const mydsp&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("filename", "diode_clipper.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "diode_clipper");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "1.6.0");
		m->declare("wdmodels.lib/builddown:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/builddown:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/builddown:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/buildout:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/buildout:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/buildout:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/buildtree:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/buildtree:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/buildtree:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/capacitor_Vout:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/capacitor_Vout:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/capacitor_Vout:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/getres:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/getres:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/getres:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/lambert:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/lambert:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/lambert:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/name", "Faust Wave Digital Model Library");
		m->declare("wdmodels.lib/parallel:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/parallel:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/parallel:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/parres:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/parres:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/parres:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/resVoltage:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/resVoltage:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/resVoltage:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/u_diodeAntiparallel:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/u_diodeAntiparallel:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/u_diodeAntiparallel:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/version", "1.2.1");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = 10638298.0f / fConst0;
		fConst2 = fConst1 + 4.7e+03f;
		fConst3 = 1.0f / fConst2;
		fConst4 = 9.4e-08f * fConst0 + 0.00021276595f;
		fConst5 = 9.748549e-08f / fConst4;
		fConst6 = 38.68472f / fConst2;
		fConst7 = 2.6499305e-07f / fConst4;
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fVec0[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec1[l2] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual mydsp* clone() {
		return new mydsp(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("diode_clipper");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fRec0[0] = static_cast<float>(input0[i0]);
			float fTemp0 = fConst1 * fRec0[1];
			float fTemp1 = fTemp0 + 4.7e+03f * fRec1[1];
			float fTemp2 = fConst3 * fTemp1;
			int iTemp3 = (fTemp2 > 0.0f) - (fTemp2 < 0.0f);
			float fTemp4 = std::exp(fConst6 * fTemp1 * static_cast<float>(iTemp3));
			float fTemp5 = fConst5 * fTemp4;
			float fTemp6 = std::sqrt(2.0f * (fConst7 * fTemp4 + 1.0f));
			float fTemp7 = ((fTemp5 < 1.0f) ? fTemp6 * (fTemp6 * (0.15277778f * fTemp6 + -0.33333334f) + 1.0f) + -1.0f : std::log(std::fabs(fTemp5)));
			float fTemp8 = std::exp(fTemp7);
			float fTemp9 = fTemp7 * fTemp8 - fTemp5;
			float fTemp10 = fTemp7 + 1.0f;
			float fTemp11 = fTemp9 / (fTemp8 * fTemp10 - 0.5f * (fTemp9 * (fTemp7 + 2.0f) / fTemp10));
			float fTemp12 = fTemp7 - fTemp11;
			float fTemp13 = std::exp(fTemp12);
			float fTemp14 = fTemp12 * fTemp13 - fTemp5;
			float fTemp15 = fTemp7 + (1.0f - fTemp11);
			float fTemp16 = fTemp11 + fTemp14 / (fTemp13 * fTemp15 - 0.5f * (fTemp14 * (fTemp7 + (2.0f - fTemp11)) / fTemp15));
			float fTemp17 = fTemp7 - fTemp16;
			float fTemp18 = std::exp(fTemp17);
			float fTemp19 = fTemp17 * fTemp18 - fTemp5;
			float fTemp20 = fTemp7 + (1.0f - fTemp16);
			float fTemp21 = fTemp5 + 0.36787945f;
			float fTemp22 = std::sqrt(fTemp21);
			float fTemp23 = std::exp(fConst6 * fTemp1 * static_cast<float>(-iTemp3));
			float fTemp24 = fConst5 * fTemp23;
			float fTemp25 = std::sqrt(2.0f * (1.0f - fConst7 * fTemp23));
			float fTemp26 = ((-fTemp24 < 1.0f) ? fTemp25 * (fTemp25 * (0.15277778f * fTemp25 + -0.33333334f) + 1.0f) + -1.0f : std::log(std::fabs(-fTemp24)));
			float fTemp27 = std::exp(fTemp26);
			float fTemp28 = fTemp24 + fTemp26 * fTemp27;
			float fTemp29 = fTemp26 + 1.0f;
			float fTemp30 = fTemp28 / (fTemp27 * fTemp29 - 0.5f * (fTemp28 * (fTemp26 + 2.0f) / fTemp29));
			float fTemp31 = fTemp26 - fTemp30;
			float fTemp32 = std::exp(fTemp31);
			float fTemp33 = fTemp24 + fTemp31 * fTemp32;
			float fTemp34 = fTemp26 + (1.0f - fTemp30);
			float fTemp35 = fTemp30 + fTemp33 / (fTemp32 * fTemp34 - 0.5f * (fTemp33 * (fTemp26 + (2.0f - fTemp30)) / fTemp34));
			float fTemp36 = fTemp26 - fTemp35;
			float fTemp37 = std::exp(fTemp36);
			float fTemp38 = fTemp24 + fTemp36 * fTemp37;
			float fTemp39 = fTemp26 + (1.0f - fTemp35);
			float fTemp40 = 0.36787945f - fTemp24;
			float fTemp41 = std::sqrt(fTemp40);
			float fTemp42 = fConst3 * (fTemp0 + fTemp1 - fConst1 * fRec1[1]) - 0.02585f * static_cast<float>(2 * iTemp3) * (((fTemp5 < -0.36777943f) ? 2.331644f * fTemp22 + fTemp21 * (1.9366311f * fTemp22 + -1.8121879f - 2.3535511f * fTemp21) + -1.0f : fTemp7 - (fTemp16 + fTemp19 / (fTemp18 * fTemp20 - 0.5f * (fTemp19 * (fTemp7 + (2.0f - fTemp16)) / fTemp20)))) + ((-fTemp24 < -0.36777943f) ? 2.331644f * fTemp41 + fTemp40 * (1.9366311f * fTemp41 + -1.8121879f - 2.3535511f * fTemp40) + -1.0f : fTemp26 - (fTemp35 + fTemp38 / (fTemp37 * fTemp39 - 0.5f * (fTemp38 * (fTemp26 + (2.0f - fTemp35)) / fTemp39)))));
			fVec0[0] = fTemp42;
			fRec1[0] = fTemp42;
			float fRec2 = 0.5f * (fTemp42 + fVec0[1]);
			output0[i0] = static_cast<FAUSTFLOAT>(fRec2);
			fRec0[1] = fRec0[0];
			fVec0[1] = fVec0[0];
			fRec1[1] = fRec1[0];
		}
	}

};

#endif
