#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
binary="${repo_root}/build/dip"

print_usage() {
    cat <<EOF
Usage:
  scripts/run_all_examples.sh [lab...]
  scripts/run_all_examples.sh --lab <lab> [--lab <lab> ...]

Labs:
  lab1, lab01, 1
  lab2, lab02, 2
  lab3, lab03, 3
  lab4, lab04, 4
  lab5, lab05, 5
  all

With no lab argument, all implemented labs are run.
EOF
}

normalize_lab() {
    case "$1" in
        all)
            echo "all"
            ;;
        1|lab1|lab01)
            echo "lab01"
            ;;
        2|lab2|lab02)
            echo "lab02"
            ;;
        3|lab3|lab03)
            echo "lab03"
            ;;
        4|lab4|lab04)
            echo "lab04"
            ;;
        5|lab5|lab05)
            echo "lab05"
            ;;
        *)
            echo "Unknown lab selector: $1" >&2
            print_usage >&2
            exit 2
            ;;
    esac
}

declare -A selected_labs=()
selected_lab_args=()

while [[ "$#" -gt 0 ]]; do
    case "$1" in
        -h|--help)
            print_usage
            exit 0
            ;;
        --lab)
            if [[ "$#" -lt 2 ]]; then
                echo "Missing value for --lab." >&2
                print_usage >&2
                exit 2
            fi
            selected_lab_args+=("$2")
            shift 2
            ;;
        --lab=*)
            selected_lab_args+=("${1#--lab=}")
            shift
            ;;
        *)
            selected_lab_args+=("$1")
            shift
            ;;
    esac
done

select_all_labs() {
    selected_labs["lab01"]=1
    selected_labs["lab02"]=1
    selected_labs["lab03"]=1
    selected_labs["lab04"]=1
    selected_labs["lab05"]=1
}

if [[ "${#selected_lab_args[@]}" -eq 0 ]]; then
    select_all_labs
else
    for lab_arg in "${selected_lab_args[@]}"; do
        normalized_lab="$(normalize_lab "${lab_arg}")"
        if [[ "${normalized_lab}" == "all" ]]; then
            select_all_labs
        else
            selected_labs["${normalized_lab}"]=1
        fi
    done
fi

should_run() {
    [[ "${selected_labs[$1]:-0}" -eq 1 ]]
}

if [[ ! -x "${binary}" ]]; then
    echo "Executable not found: ${binary}" >&2
    echo "Build the project with: cmake -S . -B build && cmake --build build" >&2
    exit 1
fi

found_any=0

if should_run "lab01"; then
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
fi

if should_run "lab02"; then
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
fi

if should_run "lab03"; then
lab03_output_dir="${repo_root}/images/lab03/output"
lab03_input_dir="${repo_root}/images/lab03/input"
mkdir -p "${lab03_output_dir}"

while IFS= read -r -d '' image_path; do
    found_any=1
    image_name="$(basename "${image_path}")"
    image_stem="${image_name%.*}"

    for angle in 30 -45; do
        angle_label="${angle/-/m}"
        angle_label="${angle_label/./p}"

        for method in nearest bilinear bicubic; do
            rotated_path="${lab03_output_dir}/${image_stem}_rotate_${method}_${angle_label}.png"

            "${binary}" lab3 rotate \
                --input "${image_path}" \
                --output "${rotated_path}" \
                --angle "${angle}" \
                --method "${method}"

            echo "wrote ${rotated_path#${repo_root}/}"
        done

        comparison_path="${lab03_output_dir}/${image_stem}_rotation_comparison_${angle_label}.json"
        "${binary}" lab3 compare \
            --input "${image_path}" \
            --output "${comparison_path}" \
            --angle "${angle}"

        echo "wrote ${comparison_path#${repo_root}/}"
    done
done < <(find "${lab03_input_dir}" -maxdepth 1 -type f \
    \( -iname '*.bmp' -o -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.tif' -o -iname '*.tiff' \) \
    -print0 | sort -z)
fi

if should_run "lab04"; then
lab04_output_dir="${repo_root}/images/lab04/output"
lab04_input_dir="${repo_root}/images/lab04/input"
lab04_kernel_dir="${lab04_input_dir}/kernels"
mkdir -p "${lab04_output_dir}"

while IFS= read -r -d '' image_path; do
    found_any=1
    image_name="$(basename "${image_path}")"
    image_stem="${image_name%.*}"

    while IFS= read -r -d '' kernel_path; do
        kernel_name="$(basename "${kernel_path}")"
        kernel_stem="${kernel_name%.*}"
        convolved_path="${lab04_output_dir}/${image_stem}_convolve_${kernel_stem}.png"

        "${binary}" lab4 convolve \
            --input "${image_path}" \
            --output "${convolved_path}" \
            --kernel "${kernel_path}"

        echo "wrote ${convolved_path#${repo_root}/}"
    done < <(find "${lab04_kernel_dir}" -maxdepth 1 -type f -iname '*.txt' -print0 | sort -z)

    for sharpen_kernel_size in 3 5 7; do
        sharpen_path="${lab04_output_dir}/${image_stem}_sharpen_${sharpen_kernel_size}x${sharpen_kernel_size}.png"

        "${binary}" lab4 sharpen \
            --input "${image_path}" \
            --output "${sharpen_path}" \
            --kernel-size "${sharpen_kernel_size}" \
            --amount 1.0

        echo "wrote ${sharpen_path#${repo_root}/}"
    done

    for laplacian_kernel_size in 3 5; do
        for laplacian_kernel in four eight; do
            laplacian_path="${lab04_output_dir}/${image_stem}_laplacian_${laplacian_kernel}.png"
            if [[ "${laplacian_kernel_size}" -ne 3 ]]; then
                laplacian_path="${lab04_output_dir}/${image_stem}_laplacian_${laplacian_kernel}_${laplacian_kernel_size}x${laplacian_kernel_size}.png"
            fi

            "${binary}" lab4 laplacian \
                --input "${image_path}" \
                --output "${laplacian_path}" \
                --kernel "${laplacian_kernel}" \
                --kernel-size "${laplacian_kernel_size}"

            echo "wrote ${laplacian_path#${repo_root}/}"
        done
    done

    log_kernel_size=7
    log_sigma=1.4
    log_sigma_label="1p4"
    log_path="${lab04_output_dir}/${image_stem}_log_${log_kernel_size}x${log_kernel_size}_sigma${log_sigma_label}.png"

    "${binary}" lab4 log-filter \
        --input "${image_path}" \
        --output "${log_path}" \
        --kernel-size "${log_kernel_size}" \
        --sigma "${log_sigma}"

    echo "wrote ${log_path#${repo_root}/}"

    zero_crossing_path="${lab04_output_dir}/${image_stem}_zero_crossing_log_${log_kernel_size}x${log_kernel_size}_sigma${log_sigma_label}.png"
    zero_crossing_metrics_path="${lab04_output_dir}/${image_stem}_zero_crossing_log_${log_kernel_size}x${log_kernel_size}_sigma${log_sigma_label}.json"

    "${binary}" lab4 zero-crossing \
        --input "${image_path}" \
        --output "${zero_crossing_path}" \
        --kernel-size "${log_kernel_size}" \
        --sigma "${log_sigma}" \
        --metrics-output "${zero_crossing_metrics_path}"

    echo "wrote ${zero_crossing_path#${repo_root}/}"
    echo "wrote ${zero_crossing_metrics_path#${repo_root}/}"

    strong_log_kernel_size=11
    strong_log_sigma=2.4
    strong_log_sigma_label="2p4"
    strong_zero_crossing_path="${lab04_output_dir}/${image_stem}_zero_crossing_log_${strong_log_kernel_size}x${strong_log_kernel_size}_sigma${strong_log_sigma_label}.png"
    strong_zero_crossing_metrics_path="${lab04_output_dir}/${image_stem}_zero_crossing_log_${strong_log_kernel_size}x${strong_log_kernel_size}_sigma${strong_log_sigma_label}.json"

    "${binary}" lab4 zero-crossing \
        --input "${image_path}" \
        --output "${strong_zero_crossing_path}" \
        --kernel-size "${strong_log_kernel_size}" \
        --sigma "${strong_log_sigma}" \
        --metrics-output "${strong_zero_crossing_metrics_path}"

    echo "wrote ${strong_zero_crossing_path#${repo_root}/}"
    echo "wrote ${strong_zero_crossing_metrics_path#${repo_root}/}"

    lowpass_kernel_size=3
    threshold_value=20
    threshold_label="t20"
    gaussian_variance=400
    impulse_probability=0.05
    impulse_label="p005"

    case "${image_stem}" in
        flower)
            gaussian_variance=900
            impulse_probability=0.04
            impulse_label="p004"
            ;;
        imagesample)
            gaussian_variance=625
            impulse_probability=0.06
            impulse_label="p006"
            ;;
        pixel_art_invader)
            gaussian_variance=225
            impulse_probability=0.10
            impulse_label="p010"
            ;;
    esac

    gaussian_noisy_path="${lab04_output_dir}/${image_stem}_gaussian_var${gaussian_variance}.png"
    gaussian_filtered_path="${lab04_output_dir}/${image_stem}_gaussian_var${gaussian_variance}_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}.png"
    gaussian_metrics_path="${lab04_output_dir}/${image_stem}_gaussian_var${gaussian_variance}_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}_psnr.json"

    "${binary}" lab4 lowpass-denoise \
        --input "${image_path}" \
        --noise gaussian \
        --variance "${gaussian_variance}" \
        --kernel-size "${lowpass_kernel_size}" \
        --seed 1 \
        --noisy-output "${gaussian_noisy_path}" \
        --filtered-output "${gaussian_filtered_path}" \
        --metrics-output "${gaussian_metrics_path}"

    echo "wrote ${gaussian_noisy_path#${repo_root}/}"
    echo "wrote ${gaussian_filtered_path#${repo_root}/}"
    echo "wrote ${gaussian_metrics_path#${repo_root}/}"

    gaussian_threshold_filtered_path="${lab04_output_dir}/${image_stem}_gaussian_var${gaussian_variance}_threshold_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}_${threshold_label}.png"
    gaussian_threshold_metrics_path="${lab04_output_dir}/${image_stem}_gaussian_var${gaussian_variance}_threshold_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}_${threshold_label}_psnr.json"

    "${binary}" lab4 lowpass-denoise \
        --input "${image_path}" \
        --noise gaussian \
        --variance "${gaussian_variance}" \
        --kernel-size "${lowpass_kernel_size}" \
        --threshold "${threshold_value}" \
        --seed 1 \
        --noisy-output "${gaussian_noisy_path}" \
        --filtered-output "${gaussian_threshold_filtered_path}" \
        --metrics-output "${gaussian_threshold_metrics_path}"

    echo "wrote ${gaussian_threshold_filtered_path#${repo_root}/}"
    echo "wrote ${gaussian_threshold_metrics_path#${repo_root}/}"

    impulse_noisy_path="${lab04_output_dir}/${image_stem}_impulse_${impulse_label}.png"
    impulse_filtered_path="${lab04_output_dir}/${image_stem}_impulse_${impulse_label}_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}.png"
    impulse_metrics_path="${lab04_output_dir}/${image_stem}_impulse_${impulse_label}_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}_psnr.json"

    "${binary}" lab4 lowpass-denoise \
        --input "${image_path}" \
        --noise impulse \
        --probability "${impulse_probability}" \
        --kernel-size "${lowpass_kernel_size}" \
        --seed 1 \
        --noisy-output "${impulse_noisy_path}" \
        --filtered-output "${impulse_filtered_path}" \
        --metrics-output "${impulse_metrics_path}"

    echo "wrote ${impulse_noisy_path#${repo_root}/}"
    echo "wrote ${impulse_filtered_path#${repo_root}/}"
    echo "wrote ${impulse_metrics_path#${repo_root}/}"

    impulse_threshold_filtered_path="${lab04_output_dir}/${image_stem}_impulse_${impulse_label}_threshold_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}_${threshold_label}.png"
    impulse_threshold_metrics_path="${lab04_output_dir}/${image_stem}_impulse_${impulse_label}_threshold_lowpass_${lowpass_kernel_size}x${lowpass_kernel_size}_${threshold_label}_psnr.json"

    "${binary}" lab4 lowpass-denoise \
        --input "${image_path}" \
        --noise impulse \
        --probability "${impulse_probability}" \
        --kernel-size "${lowpass_kernel_size}" \
        --threshold "${threshold_value}" \
        --seed 1 \
        --noisy-output "${impulse_noisy_path}" \
        --filtered-output "${impulse_threshold_filtered_path}" \
        --metrics-output "${impulse_threshold_metrics_path}"

    echo "wrote ${impulse_threshold_filtered_path#${repo_root}/}"
    echo "wrote ${impulse_threshold_metrics_path#${repo_root}/}"
done < <(find "${lab04_input_dir}" -maxdepth 1 -type f \
    \( -iname '*.bmp' -o -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.tif' -o -iname '*.tiff' \) \
    -print0 | sort -z)
fi

if should_run "lab05"; then
lab05_output_dir="${repo_root}/images/lab05/output"
lab05_input_dir="${repo_root}/images/lab05/input"
lab05_aperture_dir="${lab05_input_dir}/apertures"
mkdir -p "${lab05_output_dir}"

while IFS= read -r -d '' image_path; do
    found_any=1
    image_name="$(basename "${image_path}")"
    image_stem="${image_name%.*}"

    for rank_name in min median max; do
        rank_path="${lab05_output_dir}/${image_stem}_rank_full_3x3_${rank_name}.png"

        "${binary}" lab5 rank \
            --input "${image_path}" \
            --output "${rank_path}" \
            --aperture "${lab05_aperture_dir}/full_3x3.txt" \
            --rank "${rank_name}"

        echo "wrote ${rank_path#${repo_root}/}"
    done

    for aperture_name in cross_5x5 diamond_5x5; do
        rank_path="${lab05_output_dir}/${image_stem}_rank_${aperture_name}_median.png"

        "${binary}" lab5 rank \
            --input "${image_path}" \
            --output "${rank_path}" \
            --aperture "${lab05_aperture_dir}/${aperture_name}.txt" \
            --rank median

        echo "wrote ${rank_path#${repo_root}/}"
    done

    for trimmed_count in 0 4 8; do
        trimmed_path="${lab05_output_dir}/${image_stem}_trimmed_mean_full_3x3_d${trimmed_count}.png"

        "${binary}" lab5 trimmed-mean \
            --input "${image_path}" \
            --output "${trimmed_path}" \
            --aperture "${lab05_aperture_dir}/full_3x3.txt" \
            --trimmed-count "${trimmed_count}"

        echo "wrote ${trimmed_path#${repo_root}/}"
    done
done < <(find "${lab05_input_dir}" -maxdepth 1 -type f \
    \( -iname '*.bmp' -o -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.tif' -o -iname '*.tiff' \) \
    -print0 | sort -z)
fi

if [[ "${found_any}" -eq 0 ]]; then
    echo "No example inputs found for the selected labs."
fi
