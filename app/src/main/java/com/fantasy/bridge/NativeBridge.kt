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
    external fun nativeApplyFilters(
        imageData: ByteArray, width: Int, height: Int, filterConfig: String
    ): ByteArray?
    external fun nativeTestApplyFilters(): Boolean

    // GLSurfaceView lifecycle
    external fun nativeRendererInit()
    external fun nativeSetImage(imageData: ByteArray, width: Int, height: Int)
    external fun nativeSetLUT(lutData: ByteArray, width: Int, height: Int)
    external fun nativeSetFilterConfig(filterConfig: String)
    external fun nativeDrawFrame(width: Int, height: Int)
    external fun nativeExportImage(width: Int, height: Int): ByteArray?
    external fun nativeRendererDestroy()
}
