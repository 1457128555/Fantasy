# fantasy_core —— 平台无关的纯 C++ 渲染/编辑核心
#
# 铁律：本库不依赖平台「窗口 / context / 系统」头文件或库
#   ✗ 不许 #include <jni.h> / <android/*> / <EGL/*> / <OpenGLES/EAGL*>（窗口/context/JNI）
#   ✗ 不许链接 log / EGL 等平台库
#   ✓ 但 GLES 绘制 API 头允许（<GLES3/gl3.h> / <OpenGLES/ES3/gl.h>）——core 直接做 draw，
#     统一收口在 Render/GL.h（唯一平台 #ifdef 处）
#   ✓ 平台服务（日志、GL context/surface、文件 IO）通过 include 下的接口
#     （如 IGLContext）由各平台胶水注入
#
# 这样 Android（engine/）和 iOS（ios/）复用同一份 core。
# GLES draw 两端通用，放这里；context 创建（EGL/EAGL）属平台胶水。

cmake_minimum_required(VERSION 3.22.1)
project(fantasy_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动收集 src/ 下所有 .cpp（含子目录）。CONFIGURE_DEPENDS 让增删文件时
# CMake 自动重新收集，不用每次手动改这里。
file(GLOB_RECURSE FANTASY_CORE_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(fantasy_core STATIC ${FANTASY_CORE_SOURCES})

# include/ 设为 PUBLIC：链接 fantasy_core 的目标（engine 胶水、iOS 胶水）
# 自动获得 <Common/...> <Render/...> 等头文件搜索路径。
target_include_directories(fantasy_core PUBLIC include)
