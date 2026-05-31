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

for input_dir in "${repo_root}"/images/lab*/input; do
    [[ -d "${input_dir}" ]] || continue

    lab_dir="$(dirname "${input_dir}")"
    output_dir="${lab_dir}/output"
    mkdir -p "${output_dir}"

    while IFS= read -r -d '' image_path; do
        found_any=1
        base_name="$(basename "${image_path}")"
        result_path="${output_dir}/${base_name}.info.txt"
        "${binary}" info --input "${image_path}" > "${result_path}"
        echo "wrote ${result_path#${repo_root}/}"
    done < <(find "${input_dir}" -maxdepth 1 -type f \
        \( -iname '*.bmp' -o -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.tif' -o -iname '*.tiff' \) \
        -print0 | sort -z)
done

if [[ "${found_any}" -eq 0 ]]; then
    echo "No example images found under images/labXX/input."
fi
