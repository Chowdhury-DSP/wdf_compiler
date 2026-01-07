/* ------------------------------------------------------------
name: "preamp_eq"
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
	
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	float fConst6;
	float fConst7;
	float fConst8;
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	float fConst15;
	float fConst16;
	float fConst17;
	float fConst18;
	float fConst19;
	float fRec0[2];
	int iRec1[2];
	int iRec2[2];
	float fConst20;
	float fConst21;
	float fConst22;
	float fConst23;
	float fConst24;
	float fRec3[2];
	float fConst25;
	float fVec0[2];
	float fRec4[2];
	int iRec6[2];
	int iRec7[2];
	float fConst26;
	float fConst27;
	float fConst28;
	float fRec8[2];
	
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
		m->declare("filename", "preamp_eq.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "preamp_eq");
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
		m->declare("wdmodels.lib/capacitor:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/capacitor:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/capacitor:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/capacitor_Vout:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/capacitor_Vout:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/capacitor_Vout:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/getres:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/getres:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/getres:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/name", "Faust Wave Digital Model Library");
		m->declare("wdmodels.lib/parallel:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/parallel:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/parallel:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/parres:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/parres:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/parres:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/resistor:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/resistor:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/resistor:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/series:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/series:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/series:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/u_voltage:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/u_voltage:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/u_voltage:license", "MIT-style STK-4.3 license");
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
		fConst1 = 1.0f / (5.4e-09f * fConst0 + 1.25e-05f);
		fConst2 = fConst1 + 4674.0f;
		fConst3 = 1.0f / fConst2;
		fConst4 = 6.6e-06f * fConst0 + fConst3;
		fConst5 = 1.0f / fConst4;
		fConst6 = 1.0638298e+08f / fConst0;
		fConst7 = 1.8518518e+08f / fConst0;
		fConst8 = fConst7 + 8e+04f;
		fConst9 = 1.0f / fConst8;
		fConst10 = fConst9 + 0.0001f;
		fConst11 = 1.0f / fConst10;
		fConst12 = fConst5 + fConst6 + fConst11;
		fConst13 = 1.0f / fConst12;
		fConst14 = fConst11 + fConst5;
		fConst15 = fConst7 + 9e+04f;
		fConst16 = 1.0f / fConst15;
		fConst17 = 151515.16f / fConst0;
		fConst18 = fConst1 + fConst17 + 4674.0f;
		fConst19 = 1.0f / fConst18;
		fConst20 = 1.0f / fConst14;
		fConst21 = 1.0f / (fConst10 * fConst12);
		fConst22 = 1.0f / (fConst15 * fConst4);
		fConst23 = 1.0f / (fConst18 * fConst10);
		fConst24 = fConst8 / fConst15;
		fConst25 = 151515.16f / (fConst0 * fConst18);
		fConst26 = 4674.0f / fConst8;
		fConst27 = fConst2 / fConst18;
		fConst28 = 1.8518518e+08f / (fConst0 * fConst8);
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec1[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iRec2[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec3[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fVec0[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec4[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			iRec6[l6] = 0;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			iRec7[l7] = 0;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec8[l8] = 0.0f;
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
		ui_interface->openVerticalBox("preamp_eq");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = static_cast<float>(iRec1[1]);
			float fTemp1 = static_cast<float>(-iRec2[1]) - fRec3[1];
			float fTemp2 = fConst8 * fTemp0 + 1e+04f * fTemp1;
			float fTemp3 = fConst16 * fTemp2;
			float fTemp4 = static_cast<float>(iRec7[1]);
			float fTemp5 = fConst7 * fTemp4 + 8e+04f * fRec8[1];
			float fTemp6 = static_cast<float>(-iRec6[1]) - fConst9 * fTemp5;
			float fTemp7 = fConst2 * fRec4[1] + fConst17 * fTemp6;
			float fTemp8 = fTemp3 + fConst19 * fTemp7;
			float fTemp9 = 2.0f * static_cast<float>(input0[i0]) + fRec0[1] - fTemp8;
			float fTemp10 = fConst6 * fTemp8;
			fRec0[0] = fConst13 * (fConst14 * fRec0[1] - fConst6 * fTemp9 + fTemp10);
			iRec1[0] = 0;
			iRec2[0] = 0;
			float fTemp11 = fTemp10 + fConst14 * (fRec0[1] + fTemp9);
			float fTemp12 = fConst23 * fTemp7;
			fRec3[0] = fConst9 * (8e+04f * fRec3[1] - fConst7 * (fConst20 * (fConst21 * fTemp11 + fConst22 * fTemp2 - fTemp12) + fConst24 * (fTemp0 - fTemp1) + static_cast<float>(iRec2[1])));
			float fTemp13 = fConst20 * (fTemp12 - fConst5 * (fTemp3 - fConst13 * fTemp11));
			float fTemp14 = fTemp13 + fConst25 * (fTemp6 - fRec4[1]);
			fVec0[0] = fTemp14;
			fRec4[0] = fTemp14;
			float fRec5 = 0.5f * (fTemp14 + fVec0[1]);
			iRec6[0] = 0;
			iRec7[0] = 0;
			fRec8[0] = fConst3 * (fConst26 * fTemp5 - fConst1 * (fTemp13 + fConst27 * (fRec4[1] - fTemp6) + static_cast<float>(iRec6[1]))) + fConst28 * (fTemp4 - fRec8[1]);
			output0[i0] = static_cast<FAUSTFLOAT>(fRec5);
			fRec0[1] = fRec0[0];
			iRec1[1] = iRec1[0];
			iRec2[1] = iRec2[0];
			fRec3[1] = fRec3[0];
			fVec0[1] = fVec0[0];
			fRec4[1] = fRec4[0];
			iRec6[1] = iRec6[0];
			iRec7[1] = iRec7[0];
			fRec8[1] = fRec8[0];
		}
	}

};

#endif
