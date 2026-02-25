# Общая библиотека
add_library(lab_utils STATIC
    src/image_utils.cpp 
    src/visualization.cpp
)

target_include_directories(lab_utils PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lab_utils
    ${OpenCV_LIBS}
)

set_target_properties(lab_utils PROPERTIES
    OUTPUT_NAME "lab_utils"
    CLEAN_DIRECT_OUTPUT 1 
)
