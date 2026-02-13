#![allow(nonstandard_style)]

mod custom_analog_eq_rtype;
mod custom_nonlinear_inductor;
mod analog_eq;

use std::fs::File;
use std::io::{Read, Seek, SeekFrom};
use std::path::Path;
use std::convert::TryInto;
use std::process;

fn get_data(path: &Path) -> Vec<f32> {
    let mut file = File::open(path).unwrap();

    let file_size = file.seek(SeekFrom::End(0)).unwrap();
    file.seek(SeekFrom::Start(0)).unwrap();

    let mut buffer = vec![0u8; file_size as usize];
    file.read_exact(&mut buffer).unwrap();

    buffer
        .chunks_exact(4)
        .map(|chunk| f32::from_le_bytes(chunk.try_into().unwrap()))
        .collect()
}

pub fn get_parameters(
    treble_boost: f32,
    treble_boost_q: f32,
    treble_boost_freq: f32,
    treble_cut: f32,
    treble_cut_freq: f32,
    low_boost: f32,
    low_cut: f32,
    bass_freq: f32,
) -> analog_eq::Params
{
    // see sim/pultec_bass_freq_curve_fit.py
    let P_low_cut =
        1.0f32 + low_cut.powf(1.443f32) * 99.999e3f32;

    let P_low_boost =
        1.0f32 + low_boost.powf(2.587f32) * 9.999e3f32;

    let bass_boost_cap_val =
        1.0f32
            / (2.0f32 * std::f32::consts::PI
                * 1.13062281e+04f32
                * (bass_freq - 1.30692349e+01f32))
        + 1.68909808e-07f32;

    let bass_cut_cap_val =
        1.0f32
            / (2.0f32 * std::f32::consts::PI
                * 2.27142549e+05f32
                * (bass_freq - 1.24565293e+01f32))
        + 7.10814416e-09f32;

    let treble_cut_skew = treble_cut.powf(0.395f32);

    let P_treble_cut_plus =
        1.0e3f32 * (1.0f32 - treble_cut_skew);

    let P_treble_cut_minus =
        1.0e3f32 * treble_cut_skew;

    let treble_cut_cap_val =
        1.0f32
            / (2.0f32 * std::f32::consts::PI
                * 169.3f32
                * treble_cut_freq);

    let treble_boost_skew = treble_boost.powf(0.621f32);

    let P_treble_boost_plus =
        10.0e3f32 * (1.0f32 - treble_boost_skew);

    let P_treble_boost_minus =
        10.0e3f32 * treble_boost_skew;

    let R_treble_boost_bw =
        1.0f32 + (1.0f32 - treble_boost_q) * 2499.0f32;

    // see sim/pultec_treble_boost_freq_curve_fit.py
    let A_r = (1.0e7f32).sqrt();

    let g_val =
        9.75043069e-05f32
            + treble_boost_freq
                * (-2.05889893e-08f32
                    + treble_boost_freq
                        * (1.73050404e-12f32
                            + treble_boost_freq
                                * -4.96665892e-17f32));

    let treble_boost_cap_val = g_val / A_r;

    let treble_boost_ind_val =
        A_r * A_r * treble_boost_cap_val;

    analog_eq::Params {
        P_treble_boost_plus_value: P_treble_boost_plus,
        P_treble_boost_minus_value: P_treble_boost_minus,
        R_treble_boost_bw_value: R_treble_boost_bw,
        C_treble_boost_value: treble_boost_cap_val,
        L_treble_boost_params: custom_nonlinear_inductor::L_Params {
            value: treble_boost_ind_val,
        },
        P_low_cut_value: P_low_cut,
        C_low_cut_value: bass_cut_cap_val,
        P_treble_cut_plus_value: P_treble_cut_plus,
        P_treble_cut_minus_value: P_treble_cut_minus,
        C_treble_cut_value: treble_cut_cap_val,
        P_low_boost_value: P_low_boost,
        C_low_boost_value: bass_boost_cap_val,
        R_params: custom_analog_eq_rtype::R_Params::default(),
        Ra_value: 6.2e+02,
        Rlc_value: 1.0e+03,
        Rc_value: 7.5e+01,
        Rb_value: 1.0e+04,
        Rload_value: 1.0e+06,
    }
}

fn main() {
    println!("Analog EQ test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    const treble_boost : f32 = 0.5;
    const treble_boost_q : f32 = 0.5;
    const treble_boost_freq : f32 = 5000.0;
    const treble_cut : f32 = 0.25;
    const treble_cut_freq : f32 = 10000.0;
    const low_boost : f32 = 0.75;
    const low_cut : f32 = 0.3;
    const bass_freq : f32 = 100.0;
    let params = get_parameters(treble_boost, treble_boost_q, treble_boost_freq, treble_cut, treble_cut_freq, low_boost, low_cut, bass_freq);

    let mut impedances = analog_eq::Impedances::default();
    analog_eq::calc_impedances(&mut impedances, fs, params);
    let mut state = analog_eq::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len() {
        let test_output = analog_eq::process(&mut state, &impedances, 1.0);
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
