add_library(fantasy_bridge SHARED
    src/NativeBridge.cpp
)

target_include_directories(fantasy_bridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../filter/include
)

target_link_libraries(fantasy_bridge
    filter
    rhi
    android
    log
)
