add_library(filter STATIC
    src/Filter.cpp
    src/FilterChain.cpp
    src/filters/BrightnessFilter.cpp
    src/filters/ContrastFilter.cpp
    src/filters/SaturationFilter.cpp
    src/filters/LUTFilter.cpp
    src/filters/SharpenFilter.cpp
    src/filters/BlurFilter.cpp
    src/filters/VignetteFilter.cpp
    src/filters/CropRotateFilter.cpp
)

target_include_directories(filter PUBLIC include)

target_link_libraries(filter
    rhi
    log
)
