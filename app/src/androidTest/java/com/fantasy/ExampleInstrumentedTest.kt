package com.fantasy

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.fantasy.bridge.NativeBridge
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ExampleInstrumentedTest {

    private val bridge = NativeBridge()

    @Test
    fun useAppContext() {
        val appContext = InstrumentationRegistry.getInstrumentation().targetContext
        assertEquals("com.fantasy", appContext.packageName)
    }

    // --- RHI Tests (Phase 1) ---

    @Test
    fun testTexture() = runBlocking(Dispatchers.Default) {
        assertTrue("Texture test failed", bridge.nativeTestTexture())
    }

    @Test
    fun testShader() = runBlocking(Dispatchers.Default) {
        assertTrue("Shader test failed", bridge.nativeTestShader())
    }

    @Test
    fun testVertexBuffer() = runBlocking(Dispatchers.Default) {
        assertTrue("VertexBuffer test failed", bridge.nativeTestVertexBuffer())
    }

    @Test
    fun testFramebuffer() = runBlocking(Dispatchers.Default) {
        assertTrue("Framebuffer test failed", bridge.nativeTestFramebuffer())
    }

    @Test
    fun testPipeline() = runBlocking(Dispatchers.Default) {
        assertTrue("Pipeline test failed", bridge.nativeTestPipeline())
    }

    // --- Filter Tests (Phase 2) ---

    @Test
    fun testBrightnessFilter() = runBlocking(Dispatchers.Default) {
        assertTrue("BrightnessFilter test failed", bridge.nativeTestBrightnessFilter())
    }

    @Test
    fun testBrightnessIdentity() = runBlocking(Dispatchers.Default) {
        assertTrue("BrightnessIdentity test failed", bridge.nativeTestBrightnessIdentity())
    }

    @Test
    fun testFilterChain() = runBlocking(Dispatchers.Default) {
        assertTrue("FilterChain test failed", bridge.nativeTestFilterChain())
    }
}
