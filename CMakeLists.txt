cmake_minimum_required(VERSION 3.16)

project(digital_image_processing_labs
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)

add_executable(dip
    src/app/main.cpp
    src/app/cli.cpp
    src/core/gray_image.cpp
    src/core/image_io.cpp
    src/homework/block_classifier.cpp
    src/homework/halftone_pipeline.cpp
    src/homework/tiff_reader.cpp
    src/lab01_analysis/cooccurrence.cpp
    src/lab01_analysis/histogram.cpp
    src/lab01_analysis/noise.cpp
    src/lab01_analysis/quality.cpp
    src/lab01_analysis/statistics.cpp
    src/lab02_fourier/fft.cpp
    src/lab03_geometry/rotation.cpp
    src/lab04_convolution/convolution.cpp
    src/lab04_convolution/high_pass.cpp
    src/lab04_convolution/low_pass.cpp
    src/lab05_nonlinear/morphology.cpp
    src/lab05_nonlinear/rank_filter.cpp
    src/lab06_segmentation/connected_components.cpp
    src/lab06_segmentation/otsu.cpp
)

target_include_directories(dip
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(dip
    PRIVATE
        opencv_core
        opencv_imgcodecs
)

if(MSVC)
    target_compile_options(dip PRIVATE /W4 /permissive-)
else()
    target_compile_options(dip PRIVATE -Wall -Wextra -Wpedantic)
endif()
