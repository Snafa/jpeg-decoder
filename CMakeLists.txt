cmake_minimum_required(VERSION 3.20)
project(jpeg_decoder VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(JPEG_DECODER_BUILD_FUZZERS "Build libFuzzer targets" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET fftw3)
find_package(glog REQUIRED)

add_library(jpeg_decoder
        src/decoder.cpp
        src/jpeg_segments.cpp
        src/jpeg_entropy.cpp
        src/jpeg_postprocess.cpp
        src/fft.cpp
        src/huffman.cpp
        src/reader.cpp
)

target_compile_features(jpeg_decoder PUBLIC cxx_std_20)
set_target_properties(jpeg_decoder PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(jpeg_decoder
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(jpeg_decoder
        PUBLIC
        PkgConfig::FFTW3
        glog::glog
)

add_executable(jpeg_decoder_cli
        app/main.cpp
)

add_library(jpeg_decoder::jpeg_decoder
        ALIAS
        jpeg_decoder
)

target_link_libraries(jpeg_decoder_cli
        PRIVATE
        jpeg_decoder::jpeg_decoder
)

install(TARGETS jpeg_decoder
        EXPORT jpeg_decoderTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS jpeg_decoder_cli
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT jpeg_decoderTargets
        FILE jpeg_decoderTargets.cmake
        NAMESPACE jpeg_decoder::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jpeg_decoder
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/jpeg_decoderConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/jpeg_decoderConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/jpeg_decoderConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jpeg_decoder
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/jpeg_decoderConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/jpeg_decoderConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jpeg_decoder
)

if (BUILD_TESTING)
    find_package(JPEG REQUIRED)
    find_package(PNG REQUIRED)

    include(FetchContent)
    FetchContent_Declare(
            catch2
            URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.10.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(catch2)

    set(CATCH_SINGLE_INCLUDE_DIR ${catch2_SOURCE_DIR}/single_include/catch2)

    function(add_decoder_unit_test target_name source_file)
        add_executable(${target_name}
                ${source_file}
                utils/logger_init.cpp
        )

        target_link_libraries(${target_name}
                PRIVATE
                jpeg_decoder::jpeg_decoder
        )

        target_include_directories(${target_name}
                PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/utils
                ${CATCH_SINGLE_INCLUDE_DIR}
        )

        add_test(NAME ${target_name} COMMAND ${target_name})
    endfunction()

    function(add_decoder_image_test target_name source_file)
        add_executable(${target_name}
                ${source_file}
                utils/logger_init.cpp
                utils/test_commons.cpp
                utils/libjpg_reader.cpp
                utils/png_encoder.cpp
        )

        target_link_libraries(${target_name}
                PRIVATE
                jpeg_decoder::jpeg_decoder
                JPEG::JPEG
                PNG::PNG
        )

        target_include_directories(${target_name}
                PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/utils
                ${CATCH_SINGLE_INCLUDE_DIR}
        )

        target_compile_definitions(${target_name}
                PRIVATE
                HSE_TASK_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/\"
        )

        add_test(NAME ${target_name} COMMAND ${target_name})
    endfunction()

    add_decoder_unit_test(test_huffman tests/test_huffman.cpp)
    add_decoder_unit_test(test_fft tests/test_fft.cpp)

    add_decoder_image_test(test_baseline tests/test_baseline.cpp)
    add_decoder_image_test(test_faster tests/test_faster.cpp)

    set_tests_properties(test_faster PROPERTIES LABELS "perf;slow")

    add_executable(replay_artifacts
            tests/replay_artifacts.cpp
    )

    target_link_libraries(replay_artifacts
            PRIVATE
            jpeg_decoder::jpeg_decoder
    )

    target_include_directories(replay_artifacts
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/utils
    )

    target_compile_definitions(replay_artifacts
            PRIVATE
            HSE_TASK_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/\"
    )

    add_test(NAME replay_artifacts COMMAND replay_artifacts)
    set_tests_properties(replay_artifacts PROPERTIES LABELS "regression;malformed")
endif ()

if (JPEG_DECODER_BUILD_FUZZERS)
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "Fuzzers require Clang/libFuzzer. Configure with clang++.")
    endif ()

    function(add_decoder_fuzzer target_name source_file)
        add_executable(${target_name}
                ${source_file}
        )

        target_link_libraries(${target_name}
                PRIVATE
                jpeg_decoder::jpeg_decoder
        )

        target_include_directories(${target_name}
                PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/utils
        )

        target_compile_options(${target_name}
                PRIVATE
                -fsanitize=fuzzer,address,undefined
                -fno-omit-frame-pointer
        )

        target_link_options(${target_name}
                PRIVATE
                -fsanitize=fuzzer,address,undefined
                -fno-omit-frame-pointer
        )
    endfunction()

    add_decoder_fuzzer(fuzz_huffman tests/fuzz_huffman.cpp)
    add_decoder_fuzzer(fuzz_fft tests/fuzz_fft.cpp)
    add_decoder_fuzzer(fuzz_jpeg tests/fuzz_jpeg.cpp)

    add_custom_target(fuzzers
            DEPENDS fuzz_huffman fuzz_fft fuzz_jpeg
    )
endif ()