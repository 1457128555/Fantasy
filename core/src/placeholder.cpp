// 占位文件：让 fantasy_core 现在就能编出一个（空的）静态库，
// 用于验证跨平台构建链路（core 静态库 → 链进 Android 的 libengine.so）。
//
// 从 M2 起，真正的 core 代码（RHI 接口、IGLContext、Renderer、
// EditSession、Effect 等）由你写进 core/src/ 和 core/include/fantasy/，
// 届时删掉本文件。
//
// 记住铁律：这里的任何文件都不许 #include 平台头（jni / android / EGL / OpenGLES）。
