#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
binary="${repo_root}/build/dip"

if [[ ! -x "${binary}" ]]; then
    echo "Executable not found: ${binary}" >&2
    echo "Build the project with: cmake -S . -B build && cmake --build build" >&2
    exit 1
fi

found_any=0

input_dir="${repo_root}/images/lab01/input"
output_dir="${repo_root}/images/lab01/output"
mkdir -p "${output_dir}"

while IFS= read -r -d '' image_path; do
    found_any=1
    file_name="$(basename "${image_path}")"
    stem="${file_name%.*}"
    noise_variance=400
    noise_seed=1

    case "${stem}" in
        bnw)
            noise_variance=100
            ;;
        dog)
            noise_variance=225
            ;;
        flat_histogram_gradient_512x512)
            noise_variance=400
            ;;
        flower)
            noise_variance=900
            ;;
        imagesample)
            noise_variance=1600
            ;;
    esac

    stats_path="${output_dir}/${stem}_stats.json"
    histogram_path="${output_dir}/${stem}_histogram.png"
    glcm_path="${output_dir}/${stem}_glcm_dr0_dc1.json"
    glcm_image_path="${output_dir}/${stem}_glcm_dr0_dc1.png"
    noisy_path="${output_dir}/${stem}_noise_var${noise_variance}.png"
    psnr_path="${output_dir}/${stem}_noise_var${noise_variance}_psnr.json"

    "${binary}" lab1 stats \
        --input "${image_path}" \
        --output "${stats_path}" \
        --histogram-output "${histogram_path}"

    echo "wrote ${stats_path#${repo_root}/}"
    echo "wrote ${histogram_path#${repo_root}/}"

    "${binary}" lab1 glcm \
        --input "${image_path}" \
        --output "${glcm_path}" \
        --dr 0 \
        --dc 1 \
        --matrix-output "${glcm_image_path}"

    echo "wrote ${glcm_path#${repo_root}/}"
    echo "wrote ${glcm_image_path#${repo_root}/}"

    "${binary}" lab1 noise \
        --input "${image_path}" \
        --output "${noisy_path}" \
        --variance "${noise_variance}" \
        --seed "${noise_seed}"

    echo "wrote ${noisy_path#${repo_root}/}"

    "${binary}" lab1 psnr \
        --original "${image_path}" \
        --distorted "${noisy_path}" \
        --output "${psnr_path}"

    echo "wrote ${psnr_path#${repo_root}/}"
done < <(find "${input_dir}" -maxdepth 1 -type f \
    \( -iname '*.bmp' -o -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.tif' -o -iname '*.tiff' \) \
    -print0 | sort -z)

lab02_output_dir="${repo_root}/images/lab02/output"
lab02_input_dir="${repo_root}/images/lab02/input"
mkdir -p "${lab02_output_dir}"

while IFS= read -r -d '' signal_path; do
    found_any=1
    signal_name="$(basename "${signal_path}")"
    signal_stem="${signal_name%.*}"
    signal_plot="${lab02_output_dir}/${signal_stem}_waveform.png"
    signal_spectrum="${lab02_output_dir}/${signal_stem}_spectrum.png"

    "${binary}" lab2 signal-plot \
        --input "${signal_path}" \
        --output "${signal_plot}"

    echo "wrote ${signal_plot#${repo_root}/}"

    "${binary}" lab2 signal-spectrum \
        --input "${signal_path}" \
        --output "${signal_spectrum}"

    echo "wrote ${signal_spectrum#${repo_root}/}"
done < <(find "${lab02_input_dir}" -maxdepth 1 -type f \
    \( -iname '*.txt' -o -iname '*.csv' -o -iname '*.dat' \) \
    -print0 | sort -z)

while IFS= read -r -d '' image_path; do
    found_any=1
    image_name="$(basename "${image_path}")"
    image_stem="${image_name%.*}"
    image_spectrum="${lab02_output_dir}/${image_stem}_image_spectrum.png"
    if [[ "${image_stem}" == "image" ]]; then
        image_spectrum="${lab02_output_dir}/image_spectrum.png"
    fi

    "${binary}" lab2 image-spectrum \
        --input "${image_path}" \
        --output "${image_spectrum}"

    echo "wrote ${image_spectrum#${repo_root}/}"
done < <(find "${lab02_input_dir}" -maxdepth 1 -type f \
    \( -iname '*.bmp' -o -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.tif' -o -iname '*.tiff' \) \
    -print0 | sort -z)

if [[ "${found_any}" -eq 0 ]]; then
    echo "No example inputs found under images/lab01/input or images/lab02/input."
fi
