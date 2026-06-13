plugins {
    alias(libs.plugins.android.library)
}

android {
    namespace = "com.fan.engine"
    compileSdk {
        version = release(36) {
            minorApiLevel = 1
        }
    }

    // 钉死 NDK 版本：用磁盘上已装好的那个，避免 AGP 默认拉 28.2（下载中断、目录损坏）。
    // 这也让构建可复现（团队/换机器都用同一 NDK）。
    ndkVersion = "30.0.14904198"

    defaultConfig {
        minSdk = 24

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")

        // 只编真机要的 arm64-v8a，省去其余 3 个 ABI 的构建时间
        ndk {
            abiFilters += "arm64-v8a"
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    // 把 CMake 接进构建：没有这块，src/main/cpp 下的 CMakeLists 会被完全无视
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }
}

dependencies {
    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.core.ktx)
    implementation(libs.material)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(libs.androidx.junit)
}