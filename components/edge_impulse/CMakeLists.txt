file(GLOB_RECURSE ei_sources
    "${CMAKE_CURRENT_LIST_DIR}/edge-impulse-sdk/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/edge-impulse-sdk/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/edge-impulse-sdk/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/tflite-model/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/tflite-model/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/tflite-model/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/model-parameters/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/model-parameters/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/model-parameters/*.cpp"
)

idf_component_register(
    SRCS ${ei_sources}
    INCLUDE_DIRS
        "."
        "edge-impulse-sdk"
        "model-parameters"
        "tflite-model"
        "edge-impulse-sdk/porting/espressif/esp-dsp/modules/common/include"
        "edge-impulse-sdk/porting/espressif/esp-dsp/modules/fft/include"
    REQUIRES freertos log esp_timer esp_common
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    EI_PORTING_ESPRESSIF=1
    EIDSP_USE_ESP_DSP=0
    EIDSP_USE_CMSIS_DSP=0
    CONFIG_DSP_MAX_FFT_SIZE=256
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-missing-field-initializers
    -Wno-error=missing-field-initializers
    -Wno-deprecated-declarations
    -Wno-error=deprecated-declarations
    -Wno-error=maybe-uninitialized
)
