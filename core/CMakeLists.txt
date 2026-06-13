# fantasy_core —— 平台无关的纯 C++ 渲染/编辑核心
#
# 铁律：本库不依赖任何平台头文件或库
#   ✗ 不许 #include <jni.h> / <android/*> / <EGL/*> / <OpenGLES/*>
#   ✗ 不许链接 log / EGL 等平台库
#   ✓ 平台服务（日志、GL context/surface、文件 IO）通过 include/fantasy/ 下的
#     接口（如 IGLContext）由各平台胶水注入
#
# 这样 Android（engine）和将来 iOS（platform/ios）能复用同一份 core。
# GL ES 的 draw 调用两端通用，可放这里；context 创建（EGL/EAGL）属平台胶水。

cmake_minimum_required(VERSION 3.22.1)
project(fantasy_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(fantasy_core STATIC
        src/placeholder.cpp
)

# include/ 设为 PUBLIC：链接 fantasy_core 的目标（engine 胶水、iOS 胶水）
# 自动获得 <fantasy/...> 头文件搜索路径。
target_include_directories(fantasy_core PUBLIC include)
