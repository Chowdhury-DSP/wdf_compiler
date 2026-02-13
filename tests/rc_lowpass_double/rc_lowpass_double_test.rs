#![allow(non_snake_case)]

mod rc_lowpass_double;

use std::fs::File;
use std::io::{Read, Seek, SeekFrom};
use std::path::Path;
use std::convert::TryInto;
use std::process;

fn get_data(path: &Path) -> Vec<f64> {
    let mut file = File::open(path).unwrap();

    let file_size = file.seek(SeekFrom::End(0)).unwrap();
    file.seek(SeekFrom::Start(0)).unwrap();

    let mut buffer = vec![0u8; file_size as usize];
    file.read_exact(&mut buffer).unwrap();

    buffer
        .chunks_exact(8)
        .map(|chunk| f64::from_le_bytes(chunk.try_into().unwrap()))
        .collect()
}

fn main() {
    println!("RC Lowpass test (double-precision)");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f64 = 48000.0;

    let params = rc_lowpass_double::Params { R1_value: 1.0e+03, C1_value: 1.0e-06 };
    let mut impedances = rc_lowpass_double::Impedances::default();
    rc_lowpass_double::calc_impedances(&mut impedances, fs, params);
    let mut state = rc_lowpass_double::State::default();

    let mut max_error : f64 = 0.0;
    for i in 0..ref_output.len() {
        let test_output = rc_lowpass_double::process(&mut state, & impedances, 1.0);
        let error = f64::abs(test_output - ref_output[i]);
        max_error = f64::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
