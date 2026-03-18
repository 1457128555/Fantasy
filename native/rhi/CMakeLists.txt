add_library(rhi STATIC
    src/gles/GLTexture.cpp
    src/gles/GLShader.cpp
    src/gles/GLVertexBuffer.cpp
    src/gles/GLFramebuffer.cpp
    src/gles/GLRenderer.cpp
)

target_include_directories(rhi PUBLIC include)

# GLES 实现内部头文件
target_include_directories(rhi PRIVATE src)

target_link_libraries(rhi
    GLESv3
    EGL
    android
    log
)
