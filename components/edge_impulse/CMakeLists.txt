set(EI_SOURCES

)
file(GLOB_RECURSE EI_CPP_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/edge-impulse-sdk/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/tflite-model/*.cpp"
)
file(GLOB_RECURSE EI_C_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/edge-impulse-sdk/*.c"
)
list(FILTER EI_C_SOURCES EXCLUDE REGEX ".*/esp-dsp/.*")
list(APPEND EI_SOURCES ${EI_CPP_SOURCES} ${EI_C_SOURCES})

set(EI_INCLUDE_DIRS
    "."
    "edge-impulse-sdk"
    "model-parameters"
    "tflite-model"
    "edge-impulse-sdk/porting/espressif/esp-dsp/modules/common/include"
    "edge-impulse-sdk/porting/espressif/esp-dsp/modules/fft/include"
)

idf_component_register(
    SRCS ${EI_SOURCES}
    INCLUDE_DIRS ${EI_INCLUDE_DIRS}
    REQUIRES freertos log esp_timer esp_common
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC EI_PORTING_ESPRESSIF=1 EIDSP_USE_ESP_DSP=0 EIDSP_USE_CMSIS_DSP=0 CONFIG_DSP_MAX_FFT_SIZE=256)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers -Wno-error=missing-field-initializers)

set_source_files_properties(
    "${CMAKE_CURRENT_LIST_DIR}/edge-impulse-sdk/porting/espressif/esp-dsp/modules/fft/float/dsps_fft2r_fc32_ansi.c"
    PROPERTIES LANGUAGE C
)
