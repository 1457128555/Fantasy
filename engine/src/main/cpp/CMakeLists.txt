# engine native 构建 —— Android 平台胶水（JNI + 将来的 EGL）
# 注意：这里看起来像普通本机 CMake，没有任何交叉编译设置——
# 因为 AGP 调用 CMake 时会自动注入 NDK 工具链、ANDROID_ABI、ANDROID_PLATFORM 等。
# 你只管描述"编什么、链什么"。

cmake_minimum_required(VERSION 3.22.1)
project(engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引入平台无关的 core（顶层 core/，被 Android 和将来 iOS 共享）。
# core 在 cpp 目录之外，add_subdirectory 必须显式指定第二个参数（构建输出目录）。
add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../core   # 源码目录：Fantasy/core
        ${CMAKE_CURRENT_BINARY_DIR}/fantasy_core        # 构建输出目录
)

# 库名 engine → 产物 libengine.so → Kotlin 侧 System.loadLibrary("engine")
# 这三处必须一致。SHARED = 动态库（.so），打进 APK 的 lib/arm64-v8a/。
add_library(engine SHARED
        jni_bridge.cpp
        EglContext.cpp
)

# liblog 是 NDK sysroot 自带的系统库，__android_log_print 用它。
# 声明链接即可，运行时由系统提供。
find_library(log-lib log)
find_library(android-lib android)    
find_library(egl-lib  EGL)
find_library(gles-lib GLESv3)

# 链接：系统 log 库 + 平台无关的 fantasy_core（静态库，被吸收进 .so）。
# 链了 fantasy_core 后，jni_bridge.cpp 里可以 #include <fantasy/...>。
target_link_libraries(engine
        ${log-lib}
        ${android-lib}     
        ${egl-lib}             
        ${gles-lib}             
        fantasy_core
)
