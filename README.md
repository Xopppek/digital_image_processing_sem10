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

### Лабораторная 1

Анализ изображений, матрица совместной встречаемости, зашумление и PSNR.

Рассчитать статистические характеристики изображения:

```sh
./build/dip lab1 stats --input images/lab01/input/dog.jpg --output images/lab01/output/dog_stats.json --histogram-output images/lab01/output/dog_histogram.png
```

Рассчитать матрицу совместной встречаемости для соседних пикселов с отступом
`dr = 0`, `dc = 1`:

```sh
./build/dip lab1 glcm --input images/lab01/input/dog.jpg --output images/lab01/output/dog_glcm_dr0_dc1.json --dr 0 --dc 1 --matrix-output images/lab01/output/dog_glcm_dr0_dc1.png
```

Добавить аддитивный белый гауссов шум с заданной дисперсией:

```sh
./build/dip lab1 noise --input images/lab01/input/dog.jpg --output images/lab01/output/dog_noise_var225.png --variance 225 --seed 1
```

Рассчитать MSE и PSNR для исходного и зашумленного изображения:

```sh
./build/dip lab1 psnr --original images/lab01/input/dog.jpg --distorted images/lab01/output/dog_noise_var225.png --output images/lab01/output/dog_noise_var225_psnr.json
```

Статистика и значения PSNR сохраняются в JSON-файлы, гистограмма и матрица
совместной встречаемости сохраняются отдельными изображениями.

### Лабораторная 2

Быстрое преобразование Фурье для одномерных сигналов и двумерных изображений.

Построить изображение формы одномерного сигнала:

```sh
./build/dip lab2 signal-plot --input images/lab02/input/synthetic_sine_mix.txt --output images/lab02/output/synthetic_sine_mix_waveform.png
```

Построить логарифм амплитудного спектра одномерного сигнала:

```sh
./build/dip lab2 signal-spectrum --input images/lab02/input/synthetic_sine_mix.txt --output images/lab02/output/synthetic_sine_mix_spectrum.png
```

Построить логарифм амплитудного спектра изображения:

```sh
./build/dip lab2 image-spectrum --input images/lab02/input/synthetic_sine_mix.png --output images/lab02/output/synthetic_sine_mix_image_spectrum.png
```

### Лабораторная 3

Поворот изображения на произвольный угол с интерполяцией.

Повернуть изображение:

```sh
./build/dip lab3 rotate --input images/lab03/input/dog.jpg --output images/lab03/output/dog_rotate_bilinear_30.png --angle 30 --method bilinear
```

Доступные методы интерполяции: `nearest`, `bilinear`, `bicubic`.

Сравнить время работы и качество поворота для методов интерполяции:

```sh
./build/dip lab3 compare --input images/lab03/input/dog.jpg --output images/lab03/output/dog_rotation_comparison_30.json --angle 30
```

PSNR в этой команде считается не для одного повернутого изображения, а для
сценария "поворот на заданный угол, обратный поворот, центральная обрезка до
исходного размера". Это воспроизводимая метрика устойчивости преобразования, но
не прямое отражение визуального качества одного поворота.

## Входные и выходные изображения

Для удобства проверки примеры входных и выходных изображений хранятся прямо в
репозитории:

```text
images/lab01/input   входные изображения для лабораторной 1
images/lab01/output  результаты лабораторной 1
images/lab02/input   входные изображения для лабораторной 2
images/lab02/output  результаты лабораторной 2
images/lab03/input   входные изображения для лабораторной 3
images/lab03/output  результаты лабораторной 3
...
images/lab06/input   входные изображения для лабораторной 6
images/lab06/output  результаты лабораторной 6
```

Скрипт для воспроизводимого запуска примеров на текущем этапе выполняет
реализованные команды для лабораторных 1-3:

- для лабораторной 1 обрабатывает изображения из `images/lab01/input`, сохраняет
  статистику, гистограммы, матрицы совместной встречаемости, зашумленные
  изображения и PSNR в `images/lab01/output`;
- для лабораторной 2 строит графики и спектры для сигналов, а также спектры для
  изображений из `images/lab02/input`;
- для лабораторной 3 поворачивает изображения из `images/lab03/input` с разными
  методами интерполяции и сохраняет изображения, а также JSON-файлы сравнения
  времени и качества в `images/lab03/output`.

Дополнительные примеры и параметры запуска:

- для разных входных изображений лабораторной 1 используются разные
  фиксированные дисперсии шума, чтобы значения PSNR отличались;
- синтетические сигнал и изображение из нескольких синусоид хранятся в
  `images/lab02/input` как обычные входные примеры;
- небольшой пример `pixel_art_invader.png` добавлен в `images/lab03/input` для
  наглядного сравнения сглаживания на резких пиксельных границах.

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
