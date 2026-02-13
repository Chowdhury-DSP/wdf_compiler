#![allow(nonstandard_style)]

mod custom_bassman_rtype;
mod bassman_tone_stack;

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

fn main() {
    println!("Bassman Tone Stack test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    let params = bassman_tone_stack::Params {
        Vin_Res3m_res_value : 12.5e3,
        Res2_Res3p_value : 512.5e3,
        Res1p_Res1m_value : 250.0e3,
        Cap1_value : 2.5e-10,
        Cap2_value : 2.0e-08,
        Res4_value : 5.6e+04,
        Cap3_value : 2.0e-08,
        R_params : custom_bassman_rtype::R_Params::default(),
    };
    let mut impedances = bassman_tone_stack::Impedances::default();
    bassman_tone_stack::calc_impedances(&mut impedances, fs, params);
    let mut state = bassman_tone_stack::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len() {
        let v = 1.0_f32;
        let (v_Res1p_Res1m_Cap1, v_Vin_Res3m) = bassman_tone_stack::process(&mut state, &impedances, v);
        let test_output = v_Res1p_Res1m_Cap1 + (v_Vin_Res3m - v);
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
