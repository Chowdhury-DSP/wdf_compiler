#![allow(nonstandard_style)]

mod custom_baxandall_rtype;
mod baxandall_eq;

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
    println!("Baxandall EQ test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    let params = baxandall_eq::Params {
        Ca_value : 1.0e-06,
        S4_res_value : 8333.333008,
        S4_cap_value : 6.4e-09,
        Rl_value : 1.0e+06,
        S5_res_value : 980.392151,
        S5_cap_value : 6.4e-08,
        Resc_value : 1.0e+04,
        Resb_value : 1.0e+03,
        P3_res_value : 50000.0,
        P3_cap_value : 2.2e-07,
        Resa_value : 1.0e+04,
        P2_res_value : 50000.0,
        P2_cap_value : 2.2e-08,
        R_params: custom_baxandall_rtype::R_Params::default(),
    };
    let mut impedances = baxandall_eq::Impedances::default();
    baxandall_eq::calc_impedances(&mut impedances, fs, params);
    let mut state = baxandall_eq::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len() {
        let test_output = baxandall_eq::process(&mut state, &impedances, 1.0);
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
