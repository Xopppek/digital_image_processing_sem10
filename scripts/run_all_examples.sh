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

    stats_path="${output_dir}/${stem}_stats.json"
    histogram_path="${output_dir}/${stem}_histogram.png"
    glcm_path="${output_dir}/${stem}_glcm_dr0_dc1.json"
    glcm_image_path="${output_dir}/${stem}_glcm_dr0_dc1.png"
    noisy_path="${output_dir}/${stem}_noise_var400.png"

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
        --variance 400 \
        --seed 1

    echo "wrote ${noisy_path#${repo_root}/}"
done < <(find "${input_dir}" -maxdepth 1 -type f \
    \( -iname '*.bmp' -o -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.tif' -o -iname '*.tiff' \) \
    -print0 | sort -z)

if [[ "${found_any}" -eq 0 ]]; then
    echo "No Lab 1 input images found under images/lab01/input."
fi
