# Лабораторные работы по цифровой обработке изображений

Репозиторий содержит лабораторные работы в рамках курса "Цифровая обработка
изображений".

Проект написан на C++17. Для сборки используется CMake, итоговый исполняемый
файл называется `dip`. OpenCV используется только для чтения и записи
изображений.

## Сборка

```sh
cmake -S . -B build
cmake --build build
```

## Запуск

Вывести сведения о полутоновом изображении:

```sh
./build/dip info --input images/lab01/input/example.png
```

Посмотреть справку по лабораторным работам:

```sh
./build/dip lab1 --help
./build/dip lab2 --help
./build/dip lab3 --help
./build/dip lab4 --help
./build/dip lab5 --help
./build/dip lab6 --help
```

Рассчитать статистические характеристики изображения для лабораторной 1:

```sh
./build/dip lab1 stats --input images/lab01/input/dog.jpg --output images/lab01/output/dog_stats.json --histogram-output images/lab01/output/dog_histogram.png
```

Статистика сохраняется в JSON-файл, гистограмма сохраняется отдельным
изображением.

Рассчитать матрицу совместной встречаемости для соседних пикселов с отступом
`dr = 0`, `dc = 1`:

```sh
./build/dip lab1 glcm --input images/lab01/input/dog.jpg --output images/lab01/output/dog_glcm_dr0_dc1.json --dr 0 --dc 1 --matrix-output images/lab01/output/dog_glcm_dr0_dc1.png
```

Добавить аддитивный белый гауссов шум с заданной дисперсией:

```sh
./build/dip lab1 noise --input images/lab01/input/dog.jpg --output images/lab01/output/dog_noise_var400.png --variance 400 --seed 1
```

## Входные и выходные изображения

Для удобства проверки примеры входных и выходных изображений хранятся прямо в
репозитории:

```text
images/lab01/input   входные изображения для лабораторной 1
images/lab01/output  результаты лабораторной 1
images/lab02/input   входные изображения для лабораторной 2
images/lab02/output  результаты лабораторной 2
...
images/lab06/input   входные изображения для лабораторной 6
images/lab06/output  результаты лабораторной 6
```

Скрипт для воспроизводимого запуска примеров на текущем этапе обрабатывает
изображения из `images/lab01/input` и записывает статистику, гистограммы и
матрицы совместной встречаемости, а также зашумленные изображения в
`images/lab01/output`:

```sh
scripts/run_all_examples.sh
```

## Структура проекта

```text
src/core  контейнер изображения и ввод-вывод
src/app   интерфейс командной строки
scripts   скрипты проверок и воспроизводимых запусков
images    входные и выходные изображения лабораторных работ
```
