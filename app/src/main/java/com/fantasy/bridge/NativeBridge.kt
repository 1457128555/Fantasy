package com.fantasy.bridge

class NativeBridge {

    companion object {
        init {
            System.loadLibrary("fantasy_bridge")
        }
    }

    external fun nativeGetVersion(): String
    external fun nativeTestTexture(): Boolean
    external fun nativeTestShader(): Boolean
    external fun nativeTestVertexBuffer(): Boolean
    external fun nativeTestFramebuffer(): Boolean
    external fun nativeTestPipeline(): Boolean
    external fun nativeTestBrightnessFilter(): Boolean
    external fun nativeTestBrightnessIdentity(): Boolean
    external fun nativeTestFilterChain(): Boolean
}
