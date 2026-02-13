#![allow(non_snake_case)]

mod preamp_eq_comb;

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
    println!("Preamp EQ test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    let params = preamp_eq_comb::Params {
        v_in_v_value : 0.0e+00,
        lfc_cap_cap_value : 4.7e-09,
        r_series_value : 1.0e+04,
        hfl_res_cap_res_value : 8.0e+04,
        hfl_res_cap_cap_value : 2.7e-09,
        hfc_cap_value : 3.3e-06,
        r_shunt_value : 4.674e+03,
        lfl_res_cap_res_value : 1.5e+04,
        lfl_res_cap_cap_value : 1.5e-07,
    };
    let mut impedances = preamp_eq_comb::Impedances::default();
    preamp_eq_comb::calc_impedances(&mut impedances, fs, params);
    let mut state = preamp_eq_comb::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len() {
        let test_output = preamp_eq_comb::process(&mut state, &impedances, 1.0);
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }
    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
