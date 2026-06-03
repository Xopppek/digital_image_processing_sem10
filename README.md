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

Краткое описание визуального сравнения методов интерполяции сохранено в
`images/lab03/output/interpolation_visual_comparison.txt`. Для просмотра эффекта
удобно сравнивать рядом файлы `*_rotate_nearest_*.png`,
`*_rotate_bilinear_*.png` и `*_rotate_bicubic_*.png`.

### Лабораторная 4

Двумерная свертка в пространственной области для заданного ядра. Граничные
ячейки отбрасываются, поэтому результат получается меньше исходного изображения.

Применить ядро свертки:

```sh
./build/dip lab4 convolve --input images/lab04/input/flower.jpg --output images/lab04/output/flower_convolve_sharpen_3x3.png --kernel images/lab04/input/kernels/sharpen_3x3.txt
```

Файл ядра задается как текстовая матрица чисел. В каждой строке должно быть
одинаковое количество значений, размеры ядра должны быть нечетными:

```text
0 -1 0
-1 5 -1
0 -1 0
```

Применить фильтр повышения резкости:

```sh
./build/dip lab4 sharpen --input images/lab04/input/flower.jpg --output images/lab04/output/flower_sharpen_5x5.png --kernel-size 5 --amount 1.0
```

Размер ядра задается параметром `--kernel-size`; значение должно быть нечетным
и не меньше `3`. Если параметр не указан, используется размер `5`.
Коэффициент `--amount` задает силу повышения резкости; значение по умолчанию
равно `1.0`.

Фильтр строится как `I + amount * L`, где `I` - единичное ядро, а `L` -
лапласиан с положительным центральным коэффициентом.

Исследовать работу усредняющего фильтра низких частот на зашумленном
изображении:

```sh
./build/dip lab4 lowpass-denoise --input images/lab04/input/flower.jpg --noise gaussian --variance 900 --kernel-size 3 --seed 1 --noisy-output images/lab04/output/flower_gaussian_var900.png --filtered-output images/lab04/output/flower_gaussian_var900_lowpass_3x3.png --metrics-output images/lab04/output/flower_gaussian_var900_lowpass_3x3_psnr.json
```

Для импульсного шума вместо `--variance` используется вероятность замены
пиксела на черный или белый:

```sh
./build/dip lab4 lowpass-denoise --input images/lab04/input/flower.jpg --noise impulse --probability 0.04 --kernel-size 3 --seed 1 --noisy-output images/lab04/output/flower_impulse_p004.png --filtered-output images/lab04/output/flower_impulse_p004_lowpass_3x3.png --metrics-output images/lab04/output/flower_impulse_p004_lowpass_3x3_psnr.json
```

JSON-файл содержит MSE и PSNR для зашумленного и отфильтрованного изображения
относительно исходного. Так как свертка отбрасывает границы, сравнение
выполняется по центральной области исходного изображения размера результата
фильтрации.

Применить усредняющий фильтр с порогом:

```sh
./build/dip lab4 threshold-lowpass --input images/lab04/output/flower_gaussian_var900.png --output images/lab04/output/flower_gaussian_var900_threshold_lowpass_3x3_t20.png --kernel-size 3 --threshold 20
```

В пороговом варианте сначала вычисляется результат сглаживания `I_c`, а затем
пиксел заменяется на `I_c` только при `|I_c - I| >= T`; иначе остается исходное
значение пиксела. Для команды `lab4 lowpass-denoise` этот же режим включается
опцией `--threshold <value>`, а PSNR сохраняется в JSON-файл.

Применить ФВЧ на основе лапласиана:

```sh
./build/dip lab4 laplacian --input images/lab04/input/flower.jpg --output images/lab04/output/flower_laplacian_four.png --kernel four --kernel-size 3
```

Доступные варианты ядра: `four` и `eight`. Размер задается параметром
`--kernel-size`; значение должно быть нечетным и не меньше `3`. Если параметр не
указан, используется размер `3`. Коэффициенты строятся как численная
аппроксимация второй производной конечными разностями по выбранному числу
точек. Вариант `four` использует производные вдоль осей `x` и `y`, вариант
`eight` дополнительно учитывает диагональные направления. Сумма коэффициентов
ядра равна `0`.

Применить лапласиан гауссиана:

```sh
./build/dip lab4 log-filter --input images/lab04/input/flower.jpg --output images/lab04/output/flower_log_7x7_sigma1p4.png --kernel-size 7 --sigma 1.4
```

Для LoG используется знак ядра с положительным центральным значением. Результаты
команд `laplacian` и `log-filter` сохраняются как нормированные изображения
модуля отклика фильтра.

Построить бинарную карту границ по пересечениям нулевого уровня LoG-отклика:

```sh
./build/dip lab4 zero-crossing --input images/lab04/input/flower.jpg --output images/lab04/output/flower_zero_crossing_log_7x7_sigma1p4.png --kernel-size 7 --sigma 1.4 --metrics-output images/lab04/output/flower_zero_crossing_log_7x7_sigma1p4.json
```

Порог `T` вычисляется автоматически как `3 * sum(abs(I_e)) / (4 * W * H)`, где
`I_e` - знаковый результат LoG-фильтрации. В JSON-файл сохраняются параметры
LoG, рассчитанный порог и количество пикселов границы.

Текстовое сравнение качества работы фильтров сохранено в
`images/lab04/output/filter_quality_comparison.txt`. В нем приведены наблюдения
по ФНЧ, пороговому ФНЧ, лапласиану, LoG, zero-crossing и повышению резкости.

### Лабораторная 5

Нелинейная ранговая фильтрация для произвольной апертуры.

Применить ранговый фильтр:

```sh
./build/dip lab5 rank --input images/lab05/input/imagesample.jpg --output images/lab05/output/imagesample_rank_full_3x3_median.png --aperture images/lab05/input/apertures/full_3x3.txt --rank median
```

Апертура задается текстовой маской из `0` и `1`: значение `1` включает пиксел в
ранжируемый набор, значение `0` пропускает позицию. По смыслу такая апертура
близка к структурному элементу: она задает форму локального соседства.

Пример полной апертуры `3x3`:

```text
1 1 1
1 1 1
1 1 1
```

Параметр `--rank` задает элемент отсортированного набора значений. Можно указать
целочисленный индекс с нуля или одно из значений: `min`, `median`, `max`.
Граничные ячейки отбрасываются, поэтому результат меньше исходного изображения.

Применить фильтр усеченного среднего:

```sh
./build/dip lab5 trimmed-mean --input images/lab05/input/imagesample.jpg --output images/lab05/output/imagesample_trimmed_mean_full_3x3_d4.png --aperture images/lab05/input/apertures/full_3x3.txt --trimmed-count 4
```

Параметр `--trimmed-count` задает количество значений `d`, исключаемых из
усреднения. Значение должно быть четным и меньше числа активных ячеек апертуры.
После сортировки отбрасываются `d / 2` наименьших и `d / 2` наибольших
значений, а оставшиеся пикселы усредняются. При `d = 0` получается обычный
усредняющий фильтр; для полной нечетной апертуры при `d = K * L - 1` остается
одно среднее значение ранжированного набора, то есть медиана.

Сравнить медианный фильтр и фильтр усеченного среднего по PSNR на зашумленном
изображении:

```sh
./build/dip lab5 compare-denoise --input images/lab05/input/imagesample.jpg --noise impulse --probability 0.06 --aperture images/lab05/input/apertures/full_3x3.txt --trimmed-count 4 --seed 1 --noisy-output images/lab05/output/imagesample_impulse_p006.png --median-output images/lab05/output/imagesample_impulse_p006_median_full_3x3.png --trimmed-output images/lab05/output/imagesample_impulse_p006_trimmed_mean_full_3x3_d4.png --metrics-output images/lab05/output/imagesample_impulse_p006_median_vs_trimmed_mean_full_3x3_psnr.json
```

Для гауссова шума используется `--noise gaussian --variance <value>`, для
импульсного шума - `--noise impulse --probability <value>`. JSON-файл содержит
MSE и PSNR для зашумленного изображения, результата медианного фильтра и
результата усеченного среднего относительно исходного изображения.

Применить морфологическую операцию:

```sh
./build/dip lab5 morphology --input images/lab05/input/morphology/block_with_holes.pgm --output images/lab05/output/block_with_holes_morphology_full_3x3_opening.png --aperture images/lab05/input/apertures/full_3x3.txt --operation opening
```

Доступные операции: `erosion`, `dilation`, `opening`, `closing`. Эрозия
заменяет пиксел минимумом в окрестности, наращивание - максимумом. Открытие
выполняет эрозию, затем наращивание; закрытие выполняет наращивание, затем
эрозию. Граничные ячейки отбрасываются на каждом шаге применения апертуры.
Для визуального сравнения морфологии используются отдельные черно-белые
пиксельные примеры из `images/lab05/input/morphology`. PNG-версии этих входных
примеров сохранены в `images/lab05/output` как `*_input_view.png`, рядом с
результатами морфологических операций.

### Лабораторная 6

Сегментация изображения на два класса.

Выполнить бинаризацию по порогу, найденному алгоритмом Оцу:

```sh
./build/dip lab6 otsu --input images/lab06/input/metal.jpg --output images/lab06/output/metal_otsu_binary.png --metrics-output images/lab06/output/metal_otsu.json
```

Пикселы со значением меньше или равным найденному порогу относятся к 0-му
классу и записываются как `0`. Пикселы больше порога относятся к 1-му классу и
записываются как `255`. В JSON-файл сохраняются найденный порог, численность
классов, вероятности классов, средние значения и дисперсии.

Разметить 4-связные области на бинарном изображении:

```sh
./build/dip lab6 components --input images/lab06/input/components/diagonal_chains_4conn.pgm --output images/lab06/output/diagonal_chains_4conn_components_color.png --metrics-output images/lab06/output/diagonal_chains_4conn_components.json
```

Фоном считается значение `0`, объектами считаются все ненулевые пикселы.
В PNG-файл сохраняется цветное изображение меток для визуального сравнения, а в
JSON-файл сохраняются точные номера областей, площади, ограничивающие
прямоугольники, геометрические моменты `m00`, `m10`, `m01` и центральные
геометрические моменты второго порядка `mu20`, `mu02`, `mu11`.
Для визуального сравнения связности используются отдельные бинарные пиксельные
примеры из `images/lab06/input/components`.

Выделить области площадью больше 30 пикселов, похожие по форме на круг:

```sh
./build/dip lab6 circles --input images/lab06/input/components/geometric_shapes.pgm --output images/lab06/output/geometric_shapes_large_circles.png --metrics-output images/lab06/output/geometric_shapes_large_circles.json
```

В этом примере в `images/lab06/input/components/geometric_shapes.pgm` есть
круги, прямоугольники, эллипс, маленькая область и тонкая линия. В JSON-файл
сохраняется количество найденных областей и значения признаков: площадь,
отношение сторон ограничивающего прямоугольника, заполненность прямоугольника
и близость центральных моментов `mu20` и `mu02`.

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
images/lab04/input   входные изображения и ядра для лабораторной 4
images/lab04/output  результаты лабораторной 4
images/lab05/input   входные изображения и апертуры для лабораторной 5
images/lab05/output  результаты лабораторной 5
...
images/lab06/input   входные изображения для лабораторной 6
images/lab06/output  результаты лабораторной 6
```

Скрипт для воспроизводимого запуска примеров на текущем этапе выполняет
реализованные команды для лабораторных 1-6:

- для лабораторной 1 обрабатывает изображения из `images/lab01/input`, сохраняет
  статистику, гистограммы, матрицы совместной встречаемости, зашумленные
  изображения и PSNR в `images/lab01/output`;
- для лабораторной 2 строит графики и спектры для сигналов, а также спектры для
  изображений из `images/lab02/input`;
- для лабораторной 3 поворачивает изображения из `images/lab03/input` с разными
  методами интерполяции и сохраняет изображения, а также JSON-файлы сравнения
  времени и качества в `images/lab03/output`;
- для лабораторной 4 применяет ядра из `images/lab04/input/kernels` к
  изображениям из `images/lab04/input`, а также исследует обычный и пороговый
  усредняющий ФНЧ на изображениях с гауссовым и импульсным шумом, лапласиан и
  лапласиан гауссиана, фильтр повышения резкости, а также строит бинарные карты
  пересечений нулевого уровня. Результаты сохраняются в `images/lab04/output`.
- для лабораторной 5 применяет ранговую фильтрацию к изображениям из
  `images/lab05/input` с апертурами из `images/lab05/input/apertures` и
  сохраняет результаты в `images/lab05/output`.
- для лабораторной 6 выполняет бинаризацию по Оцу для изображений из
  `images/lab06/input`, а разметку 4-связных областей выполняет на отдельных
  бинарных примерах из `images/lab06/input/components`. Для примера с
  геометрическими фигурами также выделяются крупные кругоподобные области.
  Результаты сохраняются в `images/lab06/output`.

Дополнительные примеры и параметры запуска:

- для разных входных изображений лабораторной 1 используются разные
  фиксированные дисперсии шума, чтобы значения PSNR отличались;
- синтетические сигнал и изображение из нескольких синусоид хранятся в
  `images/lab02/input` как обычные входные примеры;
- небольшой пример `pixel_art_invader.png` добавлен в `images/lab03/input` для
  наглядного сравнения сглаживания на резких пиксельных границах;
- примеры ядер свертки хранятся в `images/lab04/input/kernels`.
- примеры апертур для ранговой фильтрации хранятся в
  `images/lab05/input/apertures`.

```sh
scripts/run_all_examples.sh
```

Чтобы запустить примеры только для одной лабораторной:

```sh
scripts/run_all_examples.sh lab4
scripts/run_all_examples.sh --lab lab4
```

Можно указать несколько лабораторных:

```sh
scripts/run_all_examples.sh lab2 lab4
```

## Структура проекта

```text
src/core  контейнер изображения и ввод-вывод
src/app   интерфейс командной строки
scripts   скрипты проверок и воспроизводимых запусков
images    входные и выходные изображения лабораторных работ
```
