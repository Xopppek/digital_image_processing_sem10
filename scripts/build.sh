#!/bin/bash

mkdir -p build
cd build

# Конфигурация
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_LAB1=ON \
         -DBUILD_LAB2=OFF \
         -DBUILD_LAB3=OFF \
         -DBUILD_LAB4=OFF \
         -DBUILD_LAB5=OFF \
         -DBUILD_LAB6=OFF

# Сборка
make -j$(nproc)

echo "Build complete. Binaries are in build/bin/"
