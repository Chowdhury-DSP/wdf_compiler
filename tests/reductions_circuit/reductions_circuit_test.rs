#![allow(non_snake_case)]

mod reductions_circuit;

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
    println!("Reductions Circuit test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    let params = reductions_circuit::Params {
        Rv1_value : 1.0e+04,
        Rv2_value : 5.0e+03,
        Rl_value : 1.0e+05,
        R7_value : 1.0e+04,
        C7_value : 1.0e-05,
        R8_value : 1.0e+05,
        C8_value : 9.999999e-08,
        R1_value : 1.0e+03,
        R2_value : 5.0e3,
        R3_value : 4.0e+03,
        R4_value : 10.0e3,
        R5_value : 3.0e+03,
        C1_value : 1.0e-06,
        C2_value : 5.0e-6,
        C3_value : 4.0e-09,
        C4_value : 10.0e-9,
        C5_value : 3.0e-06,
        R6_value : 1.0e+04,
        C6_value : 4.0e-06,
    };
    let mut impedances = reductions_circuit::Impedances::default();
    reductions_circuit::calc_impedances(&mut impedances, fs, params);
    let mut state = reductions_circuit::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len() {
        let test_output = reductions_circuit::process(&mut state, &impedances, 1.0);
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
