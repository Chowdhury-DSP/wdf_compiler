#![allow(non_snake_case)]

mod rl_lowpass;

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
    println!("RL Lowpass test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    let params = rl_lowpass::Params {
        R1_value : 1.0e+03,
        L1_value : 1.0e-03,
    };
    let mut impedances = rl_lowpass::Impedances::default();
    rl_lowpass::calc_impedances(&mut impedances, fs, params);
    let mut state = rl_lowpass::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len() {
        let test_output = rl_lowpass::process(&mut state, & impedances, 1.0);
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
