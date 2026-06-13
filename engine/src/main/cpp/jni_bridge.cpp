#include <jni.h>
#include <android/log.h>

// ============================================================================
// 这是占位文件：现在是空的，但能编译出一个（空的）libengine.so，
// 用来先验证 CMake/Gradle 构建链路是否打通（第一步小验证）。
//
// 第二步（你来写）：在下面实现 nativeHello。
//
// JNI 函数名必须严格遵守公约：
//   Java_<包名,点换下划线>_<类名>_<方法名>
//   = Java_com_fan_engine_EngineBridge_nativeHello
//
// 参考 M1 课的模板，要点：
//   - extern "C"  ：关闭 name mangling，否则 dlsym 按公约名找不到符号
//   - JNIEXPORT / JNICALL ：可见性 + 调用约定宏
//   - 第一个参数 JNIEnv* env ：跟 VM 说话的接口（线程绑定，类比 GL context）
//   - 第二个参数 jobject ：调用方对象（object 实例）
//   - 返回 env->NewStringUTF("...") 构造一个交给 VM 托管的 jstring
//   - 顺手用 __android_log_print(ANDROID_LOG_INFO, "engine", "...") 打日志
// ============================================================================

extern "C"
JNIEXPORT jstring JNICALL
Java_com_fan_engine_EngineBridge_nativeHello(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "engine", "nativeHello called");
    return env->NewStringUTF("test");
}