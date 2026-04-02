# JPEG Decoder (Baseline JPEG, C++)

Учебный проект по реализации декодера **baseline sequential JPEG** на C++.

Декодер разбирает JPEG-поток, читает служебные сегменты, декодирует entropy-coded data, выполняет dequantization, inverse DCT, upsampling цветовых компонент и преобразование `YCbCr -> RGB`.

Проект включает:
- standalone CMake build
- CLI для декодирования JPEG в `PPM`
- unit / integration / regression / performance tests
- fuzz targets для ключевых частей декодера
- replay regression checks для найденных malformed inputs

---

## Supported functionality

Реализовано:

- parsing JPEG markers and segments:
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
- dequantization of `8x8` blocks
- inverse DCT via FFTW
- chroma upsampling
- conversion from `YCbCr` to `RGB`
- extraction of JPEG comment (`COM`)
- CLI decoding to binary `PPM` (`P6`)

---

## Limitations

Проект не покрывает весь стандарт JPEG.

Не поддерживаются:

- progressive JPEG
- arithmetic coding
- restart markers
- CMYK / YCCK
- редкие и нестандартные расширения JPEG

Проект ориентирован именно на **baseline JPEG** и предназначен в первую очередь как учебная реализация.

---

## Repository layout

- `app/` — CLI entry point
- `include/` — публичные заголовки
- `src/` — реализация декодера
- `tests/` — unit, integration, regression, performance and fuzz tests
- `utils/` — вспомогательный код для тестов и работы с изображениями
- `fuzz/corpus_seed/` — стартовые seed corpora
- `fuzz/artifacts/` — сохранённые reproducer-файлы и regression artifacts

---

## Internal structure

Основные части декодера:

- `src/decoder.cpp` — верхнеуровневый orchestration-код и `Decode(...)`
- `src/jpeg_segments.cpp` — parsing JPEG segments (`DQT`, `SOF0`, `DHT`, `SOS`, `COM`)
- `src/jpeg_entropy.cpp` — entropy decoding и чтение MCU
- `src/jpeg_postprocess.cpp` — dequantization, inverse DCT, upsampling, RGB conversion
- `src/reader.cpp` — побитовое и побайтовое чтение входного JPEG-потока
- `src/huffman.cpp` — Huffman tree и декодирование кодов
- `src/fft.cpp` — inverse DCT через FFTW
- `src/jpeg_types.h` — внутренние структуры декодера
- `app/main.cpp` — CLI-утилита

---

## Dependencies

Основные зависимости:

- CMake
- C++20 compiler
- FFTW
- glog

Для тестов:

- libjpeg
- libpng

Для fuzzing:

- Clang / libFuzzer
- AddressSanitizer
- UndefinedBehaviorSanitizer

### Ubuntu

```bash
sudo apt update
sudo apt install cmake pkg-config libfftw3-dev libgoogle-glog-dev libjpeg-dev libpng-dev clang
```

---

## Build

### Release build / CLI

```bash
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --target jpeg_decoder_cli
```

### Debug build / tests

```bash
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

### Fuzzing build

```bash
cmake -S . -B cmake-build-fuzz \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++-20 \
  -DJPEG_DECODER_BUILD_FUZZERS=ON \
  -DJPEG_DECODER_BUILD_TESTS=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build cmake-build-fuzz --target fuzzers
```

---

## Usage

CLI принимает:
- входной JPEG-файл
- выходной файл в формате binary `PPM` (`P6`)

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

При необходимости `PPM` можно конвертировать в `PNG`, например через ImageMagick:

```bash
convert out.ppm out.png
```

---

## Testing

### Run all tests

```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

### Main test targets

- `test_huffman` — unit tests for Huffman decoding
- `test_fft` — unit tests for inverse DCT
- `test_baseline` — integration tests on baseline JPEG inputs
- `test_faster` — performance / stress-style test
- `replay_artifacts` — replay regression pass for malformed inputs from `fuzz/artifacts/jpeg`

### Run only performance test

```bash
ctest --test-dir cmake-build-debug -R test_faster --output-on-failure
```

### Run only replay regression artifacts

```bash
ctest --test-dir cmake-build-debug -R replay_artifacts --output-on-failure
```

---

## Fuzzing

Проект содержит три fuzz targets:

- `fuzz_jpeg`
- `fuzz_huffman`
- `fuzz_fft`

### Smoke run

```bash
./cmake-build-fuzz/fuzz_jpeg    -max_total_time=300 fuzz/corpus_work/jpeg    fuzz/corpus_seed/jpeg
./cmake-build-fuzz/fuzz_huffman -max_total_time=300 fuzz/corpus_work/huffman fuzz/corpus_seed/huffman
./cmake-build-fuzz/fuzz_fft     -max_total_time=300 fuzz/corpus_work/fft     fuzz/corpus_seed/fft
```

Первый каталог используется как рабочий corpus directory, второй — как seed corpus.

### Regression artifacts

Во время fuzzing были найдены malformed inputs, которые теперь используются как replay/regression cases. Среди исправленных проблем были:

- ошибки обработки entropy-coded data
- некорректные ссылки на таблицы и scan metadata
---

## CI

Проект содержит GitHub Actions workflow для проверки сборки и тестов.

Сюда можно добавить badge:

```md
![CI](https://github.com/Snafa/jpeg-decoder/actions/workflows/ci.yml/badge.svg)
```

---

## Future work

Возможные улучшения:

- поддержка progressive JPEG
- более строгий набор фиксированных regression tests
- дополнительная оптимизация производительности и памяти
- расширение CLI и форматов вывода
