add_library(filter STATIC
    src/Filter.cpp
    src/FilterChain.cpp
    src/filters/BrightnessFilter.cpp
)

target_include_directories(filter PUBLIC include)

target_link_libraries(filter
    rhi
    log
)
