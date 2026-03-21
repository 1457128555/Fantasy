package com.fantasy.renderer

import android.opengl.GLSurfaceView
import com.fantasy.bridge.NativeBridge
import java.util.concurrent.CountDownLatch
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class FantasyRenderer : GLSurfaceView.Renderer {

    private val bridge = NativeBridge()

    private var viewportWidth = 0
    private var viewportHeight = 0
    private var surfaceView: GLSurfaceView? = null

    private var pendingImage: Triple<ByteArray, Int, Int>? = null

    fun bindSurfaceView(view: GLSurfaceView) {
        surfaceView = view
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        bridge.nativeRendererInit()
        pendingImage?.let { (bytes, w, h) ->
            bridge.nativeSetImage(bytes, w, h)
            pendingImage = null
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        viewportWidth = width
        viewportHeight = height
    }

    override fun onDrawFrame(gl: GL10?) {
        bridge.nativeDrawFrame(viewportWidth, viewportHeight)
    }

    fun setImage(bytes: ByteArray, w: Int, h: Int) {
        pendingImage = Triple(bytes, w, h)
        surfaceView?.queueEvent {
            bridge.nativeSetImage(bytes, w, h)
            pendingImage = null
            surfaceView?.requestRender()
        }
    }

    fun setLUT(data: ByteArray, w: Int, h: Int) {
        surfaceView?.queueEvent {
            bridge.nativeSetLUT(data, w, h)
            surfaceView?.requestRender()
        }
    }

    fun setFilterConfig(config: String) {
        bridge.nativeSetFilterConfig(config)
        surfaceView?.requestRender()
    }

    fun exportImage(w: Int, h: Int): ByteArray? {
        var result: ByteArray? = null
        val latch = CountDownLatch(1)
        surfaceView?.queueEvent {
            result = bridge.nativeExportImage(w, h)
            latch.countDown()
        }
        latch.await()
        return result
    }

    fun destroy() {
        surfaceView?.queueEvent {
            bridge.nativeRendererDestroy()
        }
    }
}
