#![allow(non_snake_case)]

mod reductions_circuit2;

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
    println!("Reductions Circuit 2 test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    let params = reductions_circuit2::Params {
        Vb_v_value : 1.5e+00,
        Rl1_value : 1.0e+03,
        L1_value : 2.0e-01,
        Rl2_value : 1.0e+04,
        Rp_value : 4.0e+03,
        S4_v_value : 1.2e+00,
        Vcc_cap_value : 1.0e-6,
        C1_value :  1.0e-08,
        Cp2_value : 7.0e-09,
        Vin_res_value : 5.0e+03,
        R1_value : 1.0e+04,
        R2_value : 11.0e3,
        Cp_value : 4.0e-06,
        Rp2_value : 7.0e+03,
        Rl_v_value : -1.1e+00,
        Rl_res_value : 1.0e+05,
        Rl_cap_value : 1.0e-05,
    };
    let mut impedances = reductions_circuit2::Impedances::default();
    reductions_circuit2::calc_impedances(&mut impedances, fs, params);
    let mut state = reductions_circuit2::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len() {
        let test_output = reductions_circuit2::process(&mut state, &impedances, 1.0);
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
