#![allow(nonstandard_style)]

mod custom_baxandall_rtype;
mod baxandall_eq;

use std::fs::File;
use std::io::{Read, Seek, SeekFrom};
use std::path::Path;
use std::convert::TryInto;
use std::process;
use std::time::Instant;

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

    #[cfg(RUN_BENCH)]
    {
        println!("Running bench...");
        const M: usize = 10_000_000;
        const N_ITER: usize = 4;

        let mut data_in = vec![0.0f32; M];
        let mut data_out = vec![0.0f32; M];

        fn lcg_rand(seed: &mut u64) -> f32 {
            *seed = seed.wrapping_mul(6364136223846793005).wrapping_add(1);
            let x = (*seed >> 32) as u32;
            (x as f32 / u32::MAX as f32) * 2.0 - 1.0
        }

        let mut seed = 123456789u64;
        let mut time_accum: u128 = 0;
        let mut save_out: f32 = 0.0;
        for _ in 0..N_ITER {
            // Fill input with random floats in [-1, 1]
            for x in &mut data_in {
                *x = lcg_rand(&mut seed);
            }

            let start = Instant::now();

            for n in 0..M {
                data_out[n] = baxandall_eq::process(&mut state, &impedances, data_in[n]);
            }

            let elapsed = start.elapsed();
            time_accum += elapsed.as_nanos();

            save_out += data_out[M - 1];
        }

        println!("{}", save_out);

        let ns_per_sample =
            (time_accum as f64) / (N_ITER as f64) / (M as f64);

        println!("{:.6} ns/sample", ns_per_sample);
    }
}
