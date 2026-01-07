/* ------------------------------------------------------------
name: "baxandall_eq"
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
	FAUSTFLOAT fHslider0;
	float fConst2;
	float fConst3;
	FAUSTFLOAT fHslider1;
	float fConst4;
	float fConst5;
	float fConst6;
	float fConst7;
	float fConst8;
	float fConst9;
	float fRec0[2];
	int iRec1[2];
	int iRec2[2];
	int iRec3[2];
	float fRec5[2];
	int iRec6[2];
	int iRec7[2];
	int iRec8[2];
	int iRec9[2];
	int iRec10[2];
	float fRec11[2];
	int iRec12[2];
	int iRec13[2];
	float fRec14[2];
	float fRec15[2];
	float fConst10;
	float fConst11;
	float fRec16[2];
	
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
		m->declare("filename", "baxandall_eq.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "baxandall_eq");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "1.2.0");
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
		m->declare("wdmodels.lib/resVoltage:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/resVoltage:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/resVoltage:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/resistor:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/resistor:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/resistor:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/resistor_Vout:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/resistor_Vout:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/resistor_Vout:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/series:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/series:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/series:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/u_genericNode:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/u_genericNode:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/u_genericNode:license", "MIT-style STK-4.3 license");
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
		fConst1 = 7.8125e+07f / fConst0;
		fConst2 = 7.8125e+06f / fConst0;
		fConst3 = 4.4e-07f * fConst0;
		fConst4 = 5e+05f / fConst0;
		fConst5 = fConst4 + 1.0f;
		fConst6 = 4.4e-08f * fConst0;
		fConst7 = 1e+04f * fConst5;
		fConst8 = 2272727.2f / fConst0;
		fConst9 = 22727272.0f / fConst0;
		fConst10 = 1.0f / fConst5;
		fConst11 = 2.0f * fConst5;
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = static_cast<FAUSTFLOAT>(5e+01f);
		fHslider1 = static_cast<FAUSTFLOAT>(5e+01f);
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
			iRec3[l3] = 0;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec5[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			iRec6[l5] = 0;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			iRec7[l6] = 0;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			iRec8[l7] = 0;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			iRec9[l8] = 0;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			iRec10[l9] = 0;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec11[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			iRec12[l11] = 0;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			iRec13[l12] = 0;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec14[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec15[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec16[l15] = 0.0f;
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
		ui_interface->openVerticalBox("baxandall_eq");
		ui_interface->addHorizontalSlider("Bass", &fHslider1, FAUSTFLOAT(5e+01f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(1.0f));
		ui_interface->addHorizontalSlider("Treble", &fHslider0, FAUSTFLOAT(5e+01f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(1.0f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = static_cast<float>(fHslider0);
		float fSlow1 = 1e+03f * fSlow0;
		float fSlow2 = 1e+05f - fSlow1;
		float fSlow3 = 1.0f / (1.0f / fSlow2 + 0.0001f);
		float fSlow4 = fConst1 + fSlow3;
		float fSlow5 = 1.0f / fSlow4;
		float fSlow6 = 1e+03f / (1.0f / fSlow0 + 1.0f);
		float fSlow7 = fConst2 + fSlow6;
		float fSlow8 = 1.0f / fSlow7;
		float fSlow9 = fSlow8 + 1e-06f;
		float fSlow10 = 1.0f / fSlow9;
		float fSlow11 = static_cast<float>(fHslider1);
		float fSlow12 = 1.0f / (fConst3 + 0.001f / fSlow11);
		float fSlow13 = fSlow12 + 1e+03f;
		float fSlow14 = fSlow10 + fSlow12;
		float fSlow15 = fSlow14 + 1.1e+04f;
		float fSlow16 = 1e+03f * fSlow11;
		float fSlow17 = 1e+05f - fSlow16;
		float fSlow18 = 1.0f / (fConst6 + 1.0f / fSlow17);
		float fSlow19 = fSlow18 + 1e+04f;
		float fSlow20 = fSlow4 / fSlow9;
		float fSlow21 = fConst1 + fSlow10 + fSlow3;
		float fSlow22 = 1e+04f * fSlow21;
		float fSlow23 = fSlow21 * (fSlow13 + 1e+04f);
		float fSlow24 = fSlow13 * (fConst1 + fSlow3 + 1e+04f);
		float fSlow25 = fSlow15 * fSlow19;
		float fSlow26 = fSlow24 + fSlow22 + fSlow25 + fSlow20;
		float fSlow27 = fSlow13 * (fSlow20 + fSlow22) + fSlow19 * (fSlow20 + fSlow23) + fConst5 * fSlow26;
		float fSlow28 = 1.0f - 2.0f * (((fSlow10 + 1e+04f) * fSlow13 + fSlow15 * (fConst5 + fSlow19)) * fSlow4 / fSlow27);
		float fSlow29 = 1.1e+05f - fSlow1;
		float fSlow30 = 1.0f / fSlow29;
		float fSlow31 = 2.0f * (fSlow4 / fSlow27);
		float fSlow32 = fSlow19 * (fSlow12 + 1.1e+04f);
		float fSlow33 = fSlow32 + 1e+04f * (fConst5 + fSlow13);
		float fSlow34 = fConst2 + fSlow6 + 1e+06f;
		float fSlow35 = fSlow33 / fSlow34;
		float fSlow36 = fSlow0 + 1.0f;
		float fSlow37 = 1.0f / fSlow36;
		float fSlow38 = fSlow13 + fSlow19;
		float fSlow39 = fConst5 * (fSlow14 + 1e+03f);
		float fSlow40 = fSlow38 / fSlow9 + fSlow39;
		float fSlow41 = fSlow19 / fSlow9;
		float fSlow42 = fConst7 - fSlow41;
		float fSlow43 = fConst8 + fSlow16;
		float fSlow44 = 1.0f / fSlow43;
		float fSlow45 = fConst5 * fSlow15;
		float fSlow46 = fSlow45 + fSlow13 / fSlow9;
		float fSlow47 = fConst9 + fSlow17;
		float fSlow48 = 1.0f / fSlow47;
		float fSlow49 = fSlow25 + 1e+04f * fSlow13;
		float fSlow50 = fConst1 / fSlow29;
		float fSlow51 = fSlow9 * fSlow27;
		float fSlow52 = 2.0f / fSlow51;
		float fSlow53 = fSlow18 + fSlow3;
		float fSlow54 = fConst5 * (fConst1 + fSlow53 + 1e+04f);
		float fSlow55 = fSlow4 * fSlow38 + fSlow54;
		float fSlow56 = fConst5 * (fConst1 + fSlow53 + 2e+04f);
		float fSlow57 = fSlow56 + fSlow19 * fSlow4;
		float fSlow58 = fSlow13 * fSlow4;
		float fSlow59 = fConst7 - fSlow58;
		float fSlow60 = fSlow24 + fSlow32;
		float fSlow61 = 1.0f / fSlow34;
		float fSlow62 = 1.0f - 2.0f * ((fSlow24 + fSlow19 * (fConst1 + fSlow12 + fSlow3 + 1.1e+04f) + fSlow56) / fSlow51);
		float fSlow63 = fConst2 / fSlow36;
		float fSlow64 = 1.0f / fSlow13;
		float fSlow65 = 1e+03f / fSlow43;
		float fSlow66 = 1.0f - 2.0f * (fSlow13 * (fSlow21 * (fSlow19 + 1e+04f) + fSlow20 + fSlow56) / fSlow27);
		float fSlow67 = 2.0f * (fSlow13 / fSlow27);
		float fSlow68 = fSlow57 / fSlow34;
		float fSlow69 = fSlow54 + fSlow19 * fSlow21;
		float fSlow70 = fSlow20 + 1e+04f * (fConst5 + fSlow21);
		float fSlow71 = fSlow22 + (fSlow19 + fSlow4) / fSlow9;
		float fSlow72 = fConst8 / fSlow43;
		float fSlow73 = 1.0f / fSlow19;
		float fSlow74 = 1e+04f / fSlow47;
		float fSlow75 = 1.0f - 2.0f * (fSlow19 * (fSlow23 + fSlow45 + fSlow20) / fSlow27);
		float fSlow76 = 2.0f * (fSlow19 / fSlow27);
		float fSlow77 = fSlow59 / fSlow34;
		float fSlow78 = fSlow13 * fSlow21 + fSlow39;
		float fSlow79 = fSlow22 + fSlow4 * (fSlow10 + fSlow13);
		float fSlow80 = fConst9 / fSlow47;
		float fSlow81 = fConst11 / fSlow27;
		float fSlow82 = fSlow60 / fSlow34;
		float fSlow83 = fSlow41 - fSlow58;
		float fSlow84 = 1.0f - fConst11 * (fSlow26 / fSlow27);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = 1e+04f * static_cast<float>(iRec1[1]) + fSlow2 * static_cast<float>(iRec2[1]);
			float fTemp1 = fRec0[1] + fSlow30 * fTemp0;
			float fTemp2 = static_cast<float>(iRec3[1]);
			float fTemp3 = fSlow7 * fTemp2;
			float fTemp4 = static_cast<float>(iRec6[1]) + fSlow0 * static_cast<float>(iRec7[1]);
			float fTemp5 = fRec5[1] + fSlow37 * fTemp4;
			float fTemp6 = 1e+06f * fTemp5;
			float fTemp7 = fTemp3 - fTemp6;
			float fTemp8 = static_cast<float>(iRec8[1]);
			float fTemp9 = static_cast<float>(iRec10[1]);
			float fTemp10 = fConst8 * fTemp9 + fSlow16 * fRec11[1];
			float fTemp11 = static_cast<float>(-iRec9[1]) - fSlow44 * fTemp10;
			float fTemp12 = static_cast<float>(iRec13[1]);
			float fTemp13 = fConst9 * fTemp12 + fSlow17 * fRec14[1];
			float fTemp14 = static_cast<float>(-iRec12[1]) - fSlow48 * fTemp13;
			float fTemp15 = fRec15[1] + fRec16[1];
			fRec0[0] = fSlow5 * (fConst1 * (fSlow28 * fTemp1 + fSlow31 * (fSlow35 * fTemp7 - (fSlow40 * fTemp8 + fSlow42 * fTemp11) - (fSlow46 * fTemp14 + fSlow49 * fTemp15))) + fSlow3 * fRec0[1] - fSlow50 * fTemp0);
			iRec1[0] = 0;
			iRec2[0] = 0;
			iRec3[0] = 0;
			float fTemp16 = fSlow52 * (fSlow33 * fTemp1 - fSlow55 * fTemp8 + fSlow57 * fTemp11 + fSlow59 * fTemp14 + fSlow60 * fTemp15);
			float fTemp17 = fSlow62 * fTemp7;
			float fRec4 = 0.5f * (fTemp16 + fSlow61 * (fTemp17 - 1e+06f * fTemp2 - fTemp6));
			fRec5[0] = fSlow8 * (fSlow6 * fRec5[1] - fConst2 * (fTemp16 + fSlow61 * (fTemp3 + fTemp17 + fSlow7 * fTemp5)) - fSlow63 * fTemp4);
			iRec6[0] = 0;
			iRec7[0] = 0;
			iRec8[0] = 0;
			iRec9[0] = 0;
			iRec10[0] = 0;
			fRec11[0] = fSlow64 * (fSlow65 * fTemp10 - fSlow12 * (fSlow66 * fTemp11 - fSlow67 * (fSlow42 * fTemp1 - fSlow68 * fTemp7 - fSlow69 * fTemp8 + fSlow70 * fTemp14 - fSlow71 * fTemp15) + static_cast<float>(iRec9[1]))) + fSlow72 * (fTemp9 - fRec11[1]);
			iRec12[0] = 0;
			iRec13[0] = 0;
			fRec14[0] = fSlow73 * (fSlow74 * fTemp13 - fSlow18 * (fSlow75 * fTemp14 - fSlow76 * (fSlow46 * fTemp1 - fSlow77 * fTemp7 + fSlow78 * fTemp8 + fSlow70 * fTemp11 - fSlow79 * fTemp15) + static_cast<float>(iRec12[1]))) + fSlow80 * (fTemp12 - fRec14[1]);
			fRec15[0] = static_cast<float>(input0[i0]);
			fRec16[0] = fConst10 * (fRec16[1] - fConst4 * (fRec15[1] + (fSlow81 * (fSlow49 * fTemp1 - fSlow82 * fTemp7 + fSlow83 * fTemp8 - fSlow71 * fTemp11 - fSlow79 * fTemp14) - fSlow84 * fTemp15)));
			output0[i0] = static_cast<FAUSTFLOAT>(fRec4);
			fRec0[1] = fRec0[0];
			iRec1[1] = iRec1[0];
			iRec2[1] = iRec2[0];
			iRec3[1] = iRec3[0];
			fRec5[1] = fRec5[0];
			iRec6[1] = iRec6[0];
			iRec7[1] = iRec7[0];
			iRec8[1] = iRec8[0];
			iRec9[1] = iRec9[0];
			iRec10[1] = iRec10[0];
			fRec11[1] = fRec11[0];
			iRec12[1] = iRec12[0];
			iRec13[1] = iRec13[0];
			fRec14[1] = fRec14[0];
			fRec15[1] = fRec15[0];
			fRec16[1] = fRec16[0];
		}
	}

};

#endif
