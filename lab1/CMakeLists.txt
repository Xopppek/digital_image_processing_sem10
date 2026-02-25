add_executable(lab1
    src/main.cpp
    src/tasks.cpp
)

target_include_directories(lab1 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(lab1
    lab_utils
    ${OpenCV_LIBS}
)

add_custom_command(TARGET lab1 PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/results
    COMMENT "Creating data and results directories"
)

add_custom_command(TARGET lab1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/
    COMMENT "Copying input data to build directory"
)

if(UNIX)
    add_custom_command(TARGET lab1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data
        COMMENT "Creating symlink to input data directory"
    )
endif()

if(UNIX)
    add_custom_command(TARGET lab1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_SOURCE_DIR}/results
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/results
        COMMENT "Creating symlink to results directory"
    )
endif()
