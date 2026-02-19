#![allow(non_snake_case)]

mod channel_hpf;

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
    println!("Channel HPF test");

    let ref_output = get_data(std::path::Path::new("data.bin"));

    let fs : f32 = 48000.0;

    let mut params = channel_hpf::Params {
        Sw1_closed : 1.0,
        R8_value : 1.0e+04,
        Lh_value : 3.0e+00,
        Ll_value : 7.0e+00,
        C17_Vin_cap_value : 4.7e-07,
        C18_value : 4.7e-07,
        R9_value : 5.1e+03,
    };
    let mut impedances = channel_hpf::Impedances::default();
    channel_hpf::calc_impedances(&mut impedances, fs, params);
    let mut state = channel_hpf::State::default();

    let mut max_error : f32 = 0.0;
    for i in 0..ref_output.len()/2 {
        let test_output = channel_hpf::process(&mut state, & impedances, if i == 0 { 1.0 } else { 0.0 });
        let error = f32::abs(test_output - ref_output[i]);
        max_error = f32::max(error, max_error);
    }

    params.Sw1_closed = 0.0;
    channel_hpf::calc_impedances (&mut impedances, fs, params);
    for i in 0..ref_output.len()/2 {
        let test_output = channel_hpf::process(&mut state, & impedances, if i == 0 { 1.0 } else { 0.0 });
        let error = f32::abs(test_output - ref_output[i + ref_output.len() / 2]);
        max_error = f32::max(error, max_error);
    }

    println!("Max error: {:e}", max_error);

    if max_error > 1.0e-4 {
        println!("Error is too large... failing test!");
        process::exit(1);
    }
}
