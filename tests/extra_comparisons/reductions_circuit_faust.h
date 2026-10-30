/* ------------------------------------------------------------
name: "reductions_circuit"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
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
	
	int iRec0[2];
	int iRec1[2];
	int iRec2[2];
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
	float fConst20;
	float fConst21;
	int iRec4[2];
	int iRec5[2];
	int iRec6[2];
	int iRec7[2];
	int iRec8[2];
	float fConst22;
	float fConst23;
	float fConst24;
	float fConst25;
	float fConst26;
	float fConst27;
	float fConst28;
	float fConst29;
	float fConst30;
	float fConst31;
	float fConst32;
	float fConst33;
	float fRec9[2];
	float fRec10[2];
	float fRec11[2];
	float fRec12[2];
	float fRec13[2];
	int iRec14[2];
	float fConst34;
	float fConst35;
	float fRec15[2];
	int iRec16[2];
	float fConst36;
	float fConst37;
	float fConst38;
	float fConst39;
	float fConst40;
	float fConst41;
	float fRec17[2];
	int iRec18[2];
	float fConst42;
	float fRec19[2];
	
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
		m->declare("filename", "reductions_circuit.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "reductions_circuit");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("signals.lib/name", "Faust Routing Library");
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
		m->declare("wdmodels.lib/resistor:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/resistor:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/resistor:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/resistor_Vout:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/resistor_Vout:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/resistor_Vout:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/series:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/series:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/series:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/u_voltage:author", "Dirk Roosenburg");
		m->declare("wdmodels.lib/u_voltage:copyright", "Copyright (C) 2020 by Dirk Roosenburg <dirk.roosenburg.30@gmail.com>");
		m->declare("wdmodels.lib/u_voltage:license", "MIT-style STK-4.3 license");
		m->declare("wdmodels.lib/version", "1.4.0");
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
		fConst1 = 290893.56f / fConst0;
		fConst2 = 1.0f / (2.02e-05f * fConst0 + 0.00011f);
		fConst3 = fConst1 + fConst2;
		fConst4 = fConst3 + 111875.0f;
		fConst5 = 0.5f / fConst4;
		fConst6 = fConst3 + 11875.0f;
		fConst7 = fConst3 + 116875.0f;
		fConst8 = 1e+05f / fConst7;
		fConst9 = 2e-05f * fConst0;
		fConst10 = fConst9 + 0.00011f;
		fConst11 = 1.0f / fConst10;
		fConst12 = 5e+06f / fConst0 + fConst11;
		fConst13 = 1.0f / fConst12;
		fConst14 = fConst9 + 0.0001f;
		fConst15 = 1.0f / fConst14;
		fConst16 = fConst15 + 1e+05f;
		fConst17 = 5e+06f / (fConst0 * fConst16);
		fConst18 = 5e+04f / fConst0;
		fConst19 = fConst18 + 1e+04f;
		fConst20 = 1e+05f / fConst19;
		fConst21 = 1.0f / (fConst3 + 126875.0f);
		fConst22 = 165893.55f / fConst0;
		fConst23 = fConst22 + 1875.0f;
		fConst24 = 1.0f / fConst23;
		fConst25 = fConst22 + 11875.0f;
		fConst26 = 1.0f / fConst25;
		fConst27 = fConst1 + 11875.0f;
		fConst28 = fConst23 / fConst27;
		fConst29 = fConst25 / fConst6;
		fConst30 = fConst27 / fConst4;
		fConst31 = 1.0f / fConst7;
		fConst32 = fConst27 / fConst12;
		fConst33 = 1.25e+05f / fConst0;
		fConst34 = 1.0f / fConst27;
		fConst35 = 1.0f / fConst6;
		fConst36 = 1.0f / fConst4;
		fConst37 = 1.0f / (fConst10 * fConst12);
		fConst38 = 1.0f / fConst16;
		fConst39 = 1.0f / (fConst14 * fConst16);
		fConst40 = 1.0f / fConst19;
		fConst41 = 5e+04f / (fConst0 * fConst19);
		fConst42 = 5e+06f / (fConst0 * fConst12);
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iRec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec1[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iRec2[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			iRec4[l3] = 0;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			iRec5[l4] = 0;
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
			fRec9[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec10[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec11[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec12[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec13[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			iRec14[l13] = 0;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec15[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			iRec16[l15] = 0;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec17[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			iRec18[l17] = 0;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec19[l18] = 0.0f;
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
		ui_interface->openVerticalBox("reductions_circuit");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iRec0[0] = 0;
			iRec1[0] = 0;
			iRec2[0] = 0;
			float fTemp0 = static_cast<float>(iRec2[1]);
			float fTemp1 = static_cast<float>(iRec16[1]);
			float fTemp2 = fConst18 * fTemp1 + 1e+04f * fRec17[1];
			float fTemp3 = static_cast<float>(iRec18[1]);
			float fTemp4 = fConst20 * fTemp2 + fConst15 * fTemp3;
			float fTemp5 = fConst17 * fTemp4 + fConst11 * fRec19[1];
			float fTemp6 = fConst13 * fTemp5;
			float fTemp7 = static_cast<float>(-iRec2[1]);
			float fTemp8 = static_cast<float>(iRec14[1]);
			float fTemp9 = fTemp6 + fTemp7 + fTemp8;
			float fTemp10 = 0.003322259f * fRec12[1] + 0.99667776f * fRec13[1];
			float fTemp11 = 0.9986792f * fTemp10;
			float fTemp12 = 0.1875f * static_cast<float>(iRec4[1] + iRec5[1] - iRec6[1]);
			float fTemp13 = 0.8125f * (0.23076923f * static_cast<float>(iRec7[1]) + 0.7692308f * static_cast<float>(iRec8[1]));
			float fTemp14 = fTemp12 + fTemp13;
			float fTemp15 = fRec9[1] + fRec10[1];
			float fTemp16 = fTemp15 - fRec11[1];
			float fTemp17 = 0.0013208085f * fTemp16;
			float fTemp18 = fTemp11 + fTemp14 + fTemp17;
			float fTemp19 = static_cast<float>(-iRec1[1]);
			float fTemp20 = 5e+03f * (fTemp9 - (fTemp18 + fRec15[1])) - fConst4 * (fConst21 * (1e+04f * (fRec15[1] + fTemp11 + fTemp17 + fTemp13 + fTemp19 + fTemp12 - fTemp9) - fConst7 * (fRec15[1] + fTemp11 + fTemp17 + fTemp13 + fTemp12 + 2.0f * static_cast<float>(input0[i0]) + fTemp19 - (fTemp6 + fTemp8 + static_cast<float>(-iRec0[1]) + fTemp7) + static_cast<float>(iRec0[1]))) + static_cast<float>(iRec1[1]));
			float fTemp21 = fTemp18 - fTemp8;
			float fTemp22 = fTemp21 + fRec15[1];
			float fRec3 = fConst5 * (fConst6 * fTemp0 - fConst8 * fTemp20 + 1e+05f * (fTemp6 - fTemp22));
			iRec4[0] = 0;
			iRec5[0] = 0;
			iRec6[0] = 0;
			iRec7[0] = 0;
			iRec8[0] = 0;
			float fTemp23 = 1e+05f * (fTemp22 - fTemp6) - fConst6 * (fConst31 * fTemp20 + fTemp0);
			float fTemp24 = fConst32 * fTemp5;
			float fTemp25 = fConst30 * fTemp23 + fConst2 * fTemp22 + fTemp24;
			float fTemp26 = fConst25 * fRec15[1];
			float fTemp27 = fConst24 * (1875.0f * (fTemp17 + fTemp11) + fConst22 * (fConst26 * (fConst28 * (fConst29 * fTemp25 + fConst33 * fTemp21 - fTemp26) + 1e+04f * fTemp18 + fConst23 * fTemp8) - fTemp14));
			float fTemp28 = fTemp16 - fTemp10;
			float fTemp29 = fTemp27 - 0.9986792f * fTemp28;
			float fTemp30 = 0.00477707f * fRec11[1];
			float fTemp31 = 0.00477707f * fTemp29 + 0.9952229f * fTemp15 + fTemp30;
			fRec9[0] = 0.16666667f * fRec9[1] - 0.8333333f * (fRec10[1] - fTemp31);
			fRec10[0] = 0.8333333f * fRec10[1] - 0.16666667f * (fRec9[1] - fTemp31);
			fRec11[0] = fTemp30 + 0.9952229f * (fTemp15 - fTemp29);
			float fTemp32 = fTemp27 + 0.0013208085f * fTemp28;
			float fTemp33 = fRec12[1] - fRec13[1];
			fRec12[0] = fTemp32 - 0.99667776f * fTemp33;
			fRec13[0] = fTemp32 + 0.003322259f * fTemp33;
			iRec14[0] = 0;
			fRec15[0] = fConst34 * (fTemp26 - fConst33 * (fTemp21 - fConst35 * fTemp25));
			iRec16[0] = 0;
			float fTemp34 = fConst35 * (fTemp24 + fConst2 * (fTemp22 - fConst36 * fTemp23));
			float fTemp35 = fConst38 * fTemp4;
			fRec17[0] = fTemp34 + fConst37 * (fRec19[1] - fTemp35) + fConst39 * (fTemp3 - fConst40 * fTemp2) + fConst41 * (fTemp1 - fRec17[1]);
			iRec18[0] = 0;
			fRec19[0] = fTemp34 + fConst42 * (fTemp35 - fRec19[1]);
			output0[i0] = static_cast<FAUSTFLOAT>(fRec3);
			iRec0[1] = iRec0[0];
			iRec1[1] = iRec1[0];
			iRec2[1] = iRec2[0];
			iRec4[1] = iRec4[0];
			iRec5[1] = iRec5[0];
			iRec6[1] = iRec6[0];
			iRec7[1] = iRec7[0];
			iRec8[1] = iRec8[0];
			fRec9[1] = fRec9[0];
			fRec10[1] = fRec10[0];
			fRec11[1] = fRec11[0];
			fRec12[1] = fRec12[0];
			fRec13[1] = fRec13[0];
			iRec14[1] = iRec14[0];
			fRec15[1] = fRec15[0];
			iRec16[1] = iRec16[0];
			fRec17[1] = fRec17[0];
			iRec18[1] = iRec18[0];
			fRec19[1] = fRec19[0];
		}
	}

};

#endif
