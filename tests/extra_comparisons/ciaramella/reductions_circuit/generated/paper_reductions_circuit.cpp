#include "paper_reductions_circuit.h"


static const float bRl = 0.0f;
static const float al_34 = bRl;
static const float Rl_34 = 100000.0f;
static const float Rl_10 = ((1000.0f + 5000.0f) + 4000.0f);
static const float Rl_9 = 10000.0f;
static const float Rr_9 = 3000.0f;
static const float R0_9 = (1.0f / ((1.0f / Rl_9) + (1.0f / Rr_9)));
static const float Rr_10 = R0_9;
static const float R0_10 = (1.0f / ((1.0f / Rl_10) + (1.0f / Rr_10)));
static const float Rl_20 = R0_10;
static const float Rr_22 = 10000.0f;
static const float Rl_27 = 10000.0f;
static const float paper_reductions_circuit_extra_0 = (1.0f / Rl_27);
static const float Rr_29 = 100000.0f;
static const float paper_reductions_circuit_extra_1 = (1.0f / Rr_29);
static const float al_20 = (((R0_10 / Rl_10) * -((-((0.0f + 0.0f)) + 0.0f))) + ((R0_10 / Rr_10) * (((R0_9 / Rl_9) * 0.0f) + ((R0_9 / Rr_9) * 0.0f))));
static const float ar_22 = 0.0f;
static const float al_27 = 0.0f;
static const float ar_29 = 0.0f;
static const float Rl_36 = 5000.0f;
static const float al_36 = 0.0f;
static const float Rl_39 = 10000.0f;
static const float al_39 = 0.0f;


void paper_reductions_circuit::reset()
{
	firstRun = 1;
}

void paper_reductions_circuit::setSampleRate(float sampleRate)
{
	fs = sampleRate;
	const float Rl_13 = (0.5f / (0.000001f * fs));
	const float Rr_13 = (0.5f / (0.000005f * fs));
	const float R0_13 = (Rl_13 + Rr_13);
	const float Rl_15 = R0_13;
	const float Rr_15 = (0.5f / (4e-9f * fs));
	const float R0_15 = (Rl_15 + Rr_15);
	const float Rl_19 = R0_15;
	const float Rl_18 = (0.5f / (1e-8f * fs));
	const float Rr_18 = (0.5f / (0.000003f * fs));
	const float R0_18 = (1.0f / ((1.0f / Rl_18) + (1.0f / Rr_18)));
	const float Rr_19 = R0_18;
	const float R0_19 = (1.0f / ((1.0f / Rl_19) + (1.0f / Rr_19)));
	const float Rr_20 = R0_19;
	const float R0_20 = (Rl_20 + Rr_20);
	const float Rl_22 = R0_20;
	const float R0_22 = (Rl_22 + Rr_22);
	const float Rl_24 = R0_22;
	const float Rr_24 = (0.5f / (0.000004f * fs));
	const float R0_24 = (Rl_24 + Rr_24);
	const float Rl_32 = R0_24;
	const float Rr_27 = (0.5f / (0.00001f * fs));
	const float R0_27 = (1.0f / (paper_reductions_circuit_extra_0 + (1.0f / Rr_27)));
	const float Rl_29 = R0_27;
	const float R0_29 = (1.0f / ((1.0f / Rl_29) + paper_reductions_circuit_extra_1));
	const float Rl_31 = R0_29;
	const float Rr_31 = (0.5f / (1e-7f * fs));
	const float R0_31 = (1.0f / ((1.0f / Rl_31) + (1.0f / Rr_31)));
	const float Rr_32 = R0_31;
	const float R0_32 = (Rl_32 + Rr_32);
	const float Rr_34 = R0_32;
	const float R0_34 = (Rl_34 + Rr_34);
	paper_reductions_circuit_extra_2 = (Rl_34 / R0_34);
	paper_reductions_circuit_extra_3 = (R0_19 / Rl_19);
	paper_reductions_circuit_extra_4 = (R0_19 / Rr_19);
	paper_reductions_circuit_extra_5 = (R0_18 / Rl_18);
	paper_reductions_circuit_extra_6 = (R0_18 / Rr_18);
	paper_reductions_circuit_extra_7 = (R0_31 / Rl_31);
	paper_reductions_circuit_extra_8 = (R0_29 / Rl_29);
	paper_reductions_circuit_extra_9 = ((R0_27 / Rl_27) * al_27);
	paper_reductions_circuit_extra_10 = (R0_27 / Rr_27);
	paper_reductions_circuit_extra_11 = ((R0_29 / Rr_29) * ar_29);
	paper_reductions_circuit_extra_12 = (R0_31 / Rr_31);
	const float Rr_36 = R0_34;
	const float R0_36 = (Rl_36 + Rr_36);
	paper_reductions_circuit_extra_13 = (Rr_36 / R0_36);
	const float Rr_39 = R0_36;
	paper_reductions_circuit_extra_14 = (Rr_39 / (Rl_39 + Rr_39));
	paper_reductions_circuit_extra_15 = (Rr_32 / R0_32);
	paper_reductions_circuit_extra_16 = (Rr_34 / R0_34);
	paper_reductions_circuit_extra_19 = (Rr_24 / R0_24);
	paper_reductions_circuit_extra_20 = (Rl_32 / R0_32);
	paper_reductions_circuit_extra_22 = (Rr_15 / R0_15);
	paper_reductions_circuit_extra_23 = (Rr_20 / R0_20);
	paper_reductions_circuit_extra_24 = (Rl_22 / R0_22);
	paper_reductions_circuit_extra_25 = (Rl_24 / R0_24);
	paper_reductions_circuit_extra_27 = (Rr_13 / R0_13);
	paper_reductions_circuit_extra_28 = (Rl_15 / R0_15);
	paper_reductions_circuit_extra_30 = (Rl_13 / R0_13);
	
}

void paper_reductions_circuit::process(float *x, float *y_out_, int nSamples)
{
	if (firstRun) {
	}
	else {
	}
	
	

	if (firstRun) {
		
		_delayed_17 = 0.0f;
		_delayed_18 = 0.0f;
		_delayed_21 = 0.0f;
		_delayed_26 = 0.0f;
		_delayed_29 = 0.0f;
		_delayed_31 = 0.0f;
		_delayed_32 = 0.0f;
		_delayed_33 = 0.0f;
	}

	for (int i = 0; i < nSamples; i++) {
		
		const float al_13 = _delayed_31;
		const float ar_13 = _delayed_29;
		const float al_15 = -((al_13 + ar_13));
		const float ar_15 = _delayed_26;
		const float al_19 = -((al_15 + ar_15));
		const float al_18 = _delayed_32;
		const float ar_18 = _delayed_33;
		const float bu_18 = ((paper_reductions_circuit_extra_5 * al_18) + (paper_reductions_circuit_extra_6 * ar_18));
		const float ar_19 = bu_18;
		const float bu_19 = ((paper_reductions_circuit_extra_3 * al_19) + (paper_reductions_circuit_extra_4 * ar_19));
		const float ar_20 = bu_19;
		const float al_22 = -((al_20 + ar_20));
		const float al_24 = -((al_22 + ar_22));
		const float ar_24 = _delayed_21;
		const float al_32 = -((al_24 + ar_24));
		const float ar_27 = _delayed_17;
		const float bu_27 = (paper_reductions_circuit_extra_9 + (paper_reductions_circuit_extra_10 * ar_27));
		const float al_29 = bu_27;
		const float bu_29 = ((paper_reductions_circuit_extra_8 * al_29) + paper_reductions_circuit_extra_11);
		const float al_31 = bu_29;
		const float ar_31 = _delayed_18;
		const float bu_31 = ((paper_reductions_circuit_extra_7 * al_31) + (paper_reductions_circuit_extra_12 * ar_31));
		const float ar_32 = bu_31;
		const float ar_34 = -((al_32 + ar_32));
		const float ar_36 = -((al_34 + ar_34));
		const float ar_39 = -((al_36 + ar_36));
		const float sum_34 = (((ar_36 - (paper_reductions_circuit_extra_13 * (((ar_39 - (paper_reductions_circuit_extra_14 * ((((2.0f * x[i]) - -((al_39 + ar_39))) + al_39) + ar_39))) + al_36) + ar_36))) + al_34) + ar_34);
		const float y = (0.5f * ((al_34 - (paper_reductions_circuit_extra_2 * sum_34)) + bRl));
		const float sum_32 = (((ar_34 - (paper_reductions_circuit_extra_16 * sum_34)) + al_32) + ar_32);
		const float junction_31 = ((ar_32 - (paper_reductions_circuit_extra_15 * sum_32)) + bu_31);
		const float sum_24 = (((al_32 - (paper_reductions_circuit_extra_20 * sum_32)) + al_24) + ar_24);
		const float junction_19 = ((ar_20 - (paper_reductions_circuit_extra_23 * (((al_22 - (paper_reductions_circuit_extra_24 * (((al_24 - (paper_reductions_circuit_extra_25 * sum_24)) + al_22) + ar_22))) + al_20) + ar_20))) + bu_19);
		const float sum_15 = (((junction_19 - al_19) + al_15) + ar_15);
		const float sum_13 = (((al_15 - (paper_reductions_circuit_extra_28 * sum_15)) + al_13) + ar_13);
		const float junction_18 = ((junction_19 - ar_19) + bu_18);
		
		_delayed_17 = (((((junction_31 - al_31) + bu_29) - al_29) + bu_27) - ar_27);
		_delayed_18 = (junction_31 - ar_31);
		_delayed_21 = (ar_24 - (paper_reductions_circuit_extra_19 * sum_24));
		_delayed_26 = (ar_15 - (paper_reductions_circuit_extra_22 * sum_15));
		_delayed_29 = (ar_13 - (paper_reductions_circuit_extra_27 * sum_13));
		_delayed_31 = (al_13 - (paper_reductions_circuit_extra_30 * sum_13));
		_delayed_32 = (junction_18 - al_18);
		_delayed_33 = (junction_18 - ar_18);
		
		
		y_out_[i] = y;
	}

	
	firstRun = 0;
}


