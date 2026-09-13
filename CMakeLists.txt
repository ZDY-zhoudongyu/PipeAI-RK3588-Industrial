cmake_minimum_required(VERSION 3.16)
project(PipeAI_RK3588 VERSION 0.2.0 LANGUAGES CXX)

option(PIPEAI_BUILD_TESTS "Build host-side unit tests" ON)
option(PIPEAI_BUILD_TOOLS "Build commissioning/validation tools" ON)
option(PIPEAI_WITH_RKNN "Build with Rockchip RKNN runtime" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pipeai_core STATIC
    runtime/logger.cpp
    runtime/watchdog.cpp
    runtime/metrics.cpp
    runtime/config.cpp
    runtime/cpu_affinity.cpp
    runtime/worker_manager.cpp
    runtime/application.cpp
    sensor/csv_sensor.cpp
    sensor/sensor_manager.cpp
    preprocess/csv_reader.cpp
    preprocess/missing_repair.cpp
    preprocess/savgol.cpp
    preprocess/resample.cpp
    preprocess/preprocessor.cpp
    model/model_loader.cpp
    model/rknn_engine.cpp
    tensor/tensor.cpp
    tensor/tensor_converter.cpp
    tensor/quantizer.cpp
    tensor/buffer_pool.cpp
    optimization/quant/int8_quantizer.cpp
    optimization/neon/neon_normalize.cpp
    profiling/stage_timer.cpp
    model/rknn_runtime/rknn_engine_runtime.cpp
    pipeline/worker.cpp
    pipeline/pipeline.cpp
    pipeline/inference_pipeline_bridge.cpp
    postprocess/softmax.cpp
    postprocess/classifier.cpp
)

target_include_directories(pipeai_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(pipeai_core PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(pipeai_core PUBLIC pthread)

if(PIPEAI_WITH_RKNN)
    if(NOT DEFINED RKNN_SDK_ROOT)
        message(FATAL_ERROR "PIPEAI_WITH_RKNN=ON requires -DRKNN_SDK_ROOT=/path/to/rknpu2/runtime/Linux/librknn_api")
    endif()

    find_path(RKNN_INCLUDE_DIR rknn_api.h
        PATHS ${RKNN_SDK_ROOT}/include
        NO_DEFAULT_PATH)
    find_library(RKNN_LIBRARY rknnrt
        PATHS ${RKNN_SDK_ROOT}/aarch64
        NO_DEFAULT_PATH)

    if(NOT RKNN_INCLUDE_DIR OR NOT RKNN_LIBRARY)
        message(FATAL_ERROR "Could not find rknn_api.h/librknnrt.so under RKNN_SDK_ROOT=${RKNN_SDK_ROOT}")
    endif()

    target_compile_definitions(pipeai_core PUBLIC PIPEAI_WITH_RKNN=1)
    target_include_directories(pipeai_core PUBLIC ${RKNN_INCLUDE_DIR})
    target_link_libraries(pipeai_core PUBLIC ${RKNN_LIBRARY})
endif()

add_executable(pipeai_app app/main.cpp)
target_link_libraries(pipeai_app PRIVATE pipeai_core)

if(PIPEAI_BUILD_TOOLS)
    add_executable(pipeai_preprocess_csv tools/preprocess_csv.cpp)
    target_link_libraries(pipeai_preprocess_csv PRIVATE pipeai_core)

    add_executable(pipeai_benchmark_timer benchmarks/benchmark_timer.cpp)
    add_executable(pipeai_latency_benchmark benchmarks/latency_benchmark.cpp)
endif()

install(TARGETS pipeai_app RUNTIME DESTINATION bin)
if(PIPEAI_BUILD_TOOLS)
    install(TARGETS pipeai_preprocess_csv RUNTIME DESTINATION bin)
endif()
install(FILES config/pipeai.ini DESTINATION etc/pipeai)

if(PIPEAI_BUILD_TESTS)
    enable_testing()

    function(pipeai_add_test name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE pipeai_core)
        add_test(NAME ${name} COMMAND ${name})
    endfunction()

    pipeai_add_test(test_blocking_queue tests/test_blocking_queue.cpp)
    pipeai_add_test(test_config tests/test_config.cpp)
    pipeai_add_test(test_missing_repair tests/test_missing_repair.cpp)
    pipeai_add_test(test_savgol tests/test_savgol.cpp)
    pipeai_add_test(test_resample tests/test_resample.cpp)
    pipeai_add_test(test_preprocess_golden tests/test_preprocess_golden.cpp)
    target_compile_definitions(test_preprocess_golden PRIVATE
        PIPEAI_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
endif()
