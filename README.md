# JPEG Decoder (Baseline JPEG, C++)

Учебный проект: реализация декодера baseline JPEG на C++.

Проект разбирает JPEG-поток, декодирует энтропийно-сжатые данные, выполняет dequantization, inverse DCT, upsampling компонент и преобразование `YCbCr -> RGB`.

В репозитории есть:
- standalone CMake-сборка
- unit / integration / performance tests
- replay regression проход по malformed inputs
- fuzz targets для ключевых частей декодера
- CLI-утилита для декодирования JPEG в `PPM`

---

## Что реализовано

- parsing JPEG markers:
  - `SOI`
  - `COM`
  - `APPn` (skip)
  - `DQT`
  - `SOF0`
  - `DHT`
  - `SOS`
  - `EOI`
- baseline sequential JPEG decoding
- Huffman entropy decoding
- dequantization of 8x8 blocks
- inverse DCT via FFTW
- upsampling of color components
- conversion from `YCbCr` to `RGB`
- extraction of JPEG comment (`COM`)
- CLI-утилита для декодирования JPEG в `PPM`

---

## Ограничения

Сейчас проект поддерживает не весь стандарт JPEG.

Не реализовано:
- progressive JPEG
- arithmetic coding
- restart markers
- CMYK / YCCK
- редко используемые расширения стандарта JPEG

Проект ориентирован на baseline JPEG и предназначен в первую очередь как учебная реализация.

---

## Архитектура

Пайплайн декодирования:

1. Чтение JPEG markers и служебных сегментов
2. Загрузка quantization tables (`DQT`)
3. Загрузка Huffman tables (`DHT`)
4. Чтение параметров кадра (`SOF0`) и скана (`SOS`)
5. Entropy decoding MCU-блоков
6. Восстановление DC/AC коэффициентов
7. Dequantization
8. Inverse DCT
9. Upsampling компонент
10. Преобразование `YCbCr -> RGB`

### Структура кода

- `decoder.cpp` — верхнеуровневый orchestration-код и `Decode(...)`
- `jpeg_segments.cpp` — parsing JPEG segments (`DQT`, `SOF0`, `DHT`, `SOS`, `COM`)
- `jpeg_entropy.cpp` — entropy decoding и чтение MCU
- `jpeg_postprocess.cpp` — inverse DCT, upsampling, conversion to RGB
- `reader.cpp`, `reader.h` — побитовое и побайтовое чтение JPEG-потока
- `huffman.cpp`, `huffman.h` — Huffman tree и декодирование кодов
- `fft.cpp`, `fft.h` — inverse DCT через FFTW
- `include/jpeg_types.h` — внутренние структуры декодера
- `main.cpp` — CLI-утилита

---

## Структура репозитория

- `include/` — публичные и внутренние заголовки
- `tests/` — unit, integration, performance, fuzz и replay regression tests
- `utils/` — вспомогательные утилиты для тестов и работы с изображениями
- `fuzz/corpus/` — стартовые corpus-наборы для fuzzing
- `fuzz/artifacts/` — malformed inputs, найденные и сохранённые как regression artifacts

---

## Зависимости

Основные:
- CMake
- C++ compiler с поддержкой C++20
- FFTW
- glog

Для тестов:
- libjpeg
- libpng

Для fuzzing:
- Clang / libFuzzer
- AddressSanitizer
- UndefinedBehaviorSanitizer

Пример для Ubuntu:

```bash
sudo apt update
sudo apt install cmake pkg-config libfftw3-dev libgoogle-glog-dev libjpeg-dev libpng-dev clang
```

---

## Сборка

### Release / CLI

```bash
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --target jpeg_decoder_cli
```

### Debug / tests

```bash
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

### Fuzz build

```bash
cmake -S . -B cmake-build-fuzz-rel \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++-20 \
  -DJPEG_DECODER_BUILD_FUZZERS=ON \
  -DJPEG_DECODER_BUILD_TESTS=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build cmake-build-fuzz-rel --target fuzzers
```

---

## Usage

CLI принимает:
- входной JPEG
- выходной файл в формате binary PPM (`P6`)

Пример:

```bash
./cmake-build-release/jpeg_decoder_cli tests/data/small.jpg out.ppm
```

Пример вывода:

```text
Decoded successfully
Size: 32x32
Comment: :)
Saved to: out.ppm
```

Открыть результат можно любым просмотрщиком, поддерживающим `PPM`, либо конвертировать в PNG через ImageMagick:

```bash
convert out.ppm out.png
```

---

## Testing

### Все тесты

```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

### Набор тестов

- `test_huffman` — unit tests for Huffman decoding
- `test_fft` — unit tests for inverse DCT
- `test_baseline` — integration tests on baseline JPEG files
- `test_faster` — performance / stress-style test
- `replay_artifacts` — replay проход по malformed inputs из `fuzz/artifacts/jpeg`

### Только performance test

```bash
ctest --test-dir cmake-build-debug -R test_faster --output-on-failure
```

### Только replay malformed artifacts

```bash
ctest --test-dir cmake-build-debug -R replay_artifacts --output-on-failure
```

---

## Fuzzing

Проект содержит fuzz targets:
- `fuzz_huffman`
- `fuzz_fft`
- `fuzz_jpeg`

### Короткий smoke-run

```bash
./cmake-build-fuzz/fuzz_huffman fuzz/corpus/huffman -max_total_time=30
./cmake-build-fuzz/fuzz_fft fuzz/corpus/fft -max_total_time=30
./cmake-build-fuzz/fuzz_jpeg fuzz/corpus/jpeg -max_total_time=30
```

### Что уже проверялось

В процессе fuzzing были найдены malformed inputs, приводившие к:
- crash при обработке entropy-coded data
- неконтролируемой аллокации памяти на некорректных размерах

После исправлений эти входы используются как replay/regression artifacts.

---

## Что было улучшено по ходу проекта

Кроме базовой реализации декодера, в проекте были добавлены:
- standalone CMake configuration
- unit / integration / performance tests
- replay regression проход по найденным malformed inputs
- проверки корректности входных данных
- защита от некорректных Huffman tables и невалидных scan/table references
- защита от чрезмерных аллокаций на malformed input
- CLI-утилита для ручного запуска

---

## Status

Текущая версия:
- умеет декодировать baseline JPEG
- имеет standalone build
- проходит unit / integration / performance tests
- содержит fuzz targets и replay regression artifacts
- имеет CLI для декодирования в `PPM`

Возможные дальнейшие улучшения:
- CI
- более строгие фиксированные regression tests с отдельным перечислением ключевых reproducer-файлов
- оптимизация производительности и памяти
- поддержка дополнительных вариантов JPEG