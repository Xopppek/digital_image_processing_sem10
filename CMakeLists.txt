cmake_minimum_required(VERSION 3.16)
project(DigitalImageProcessing LANGUAGES CXX)

# Настройки
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

# Опции сборки
option(BUILD_LAB1 "Build Lab 1" ON)
option(BUILD_LAB2 "Build Lab 2" ON)
option(BUILD_LAB3 "Build Lab 3" ON)
option(BUILD_LAB4 "Build Lab 4" ON)
option(BUILD_LAB5 "Build Lab 5" ON)
option(BUILD_LAB6 "Build Lab 6" ON)

# Пути
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Поиск OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

# Добавляем общую библиотеку
add_subdirectory(common)

# Добавляем лабораторные работы
if(BUILD_LAB1)
    add_subdirectory(lab1)
endif()

if(BUILD_LAB2)
    add_subdirectory(lab2)
endif()

if(BUILD_LAB3)
    add_subdirectory(lab3)
endif()

if(BUILD_LAB4)
    add_subdirectory(lab4)
endif()

if(BUILD_LAB5)
    add_subdirectory(lab5)
endif()

if(BUILD_LAB6)
    add_subdirectory(lab6)
endif()
