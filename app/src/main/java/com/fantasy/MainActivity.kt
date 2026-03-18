package com.fantasy

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.lifecycle.lifecycleScope
import com.fantasy.bridge.NativeBridge
import com.fantasy.ui.theme.FantasyTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class MainActivity : ComponentActivity() {
    private val bridge = NativeBridge()
    private var displayText by mutableStateOf("Running tests...")

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        // 在后台线程运行 GPU 测试，避免阻塞主线程
        lifecycleScope.launch {
            val result = withContext(Dispatchers.Default) {
                val version = bridge.nativeGetVersion()
                val textureResult = bridge.nativeTestTexture()
                val shaderResult = bridge.nativeTestShader()
                val vbResult = bridge.nativeTestVertexBuffer()
                val fbResult = bridge.nativeTestFramebuffer()
                val pipelineResult = bridge.nativeTestPipeline()

                buildString {
                    append(version)
                    append("\nTexture: ${if (textureResult) "PASSED" else "FAILED"}")
                    append("\nShader: ${if (shaderResult) "PASSED" else "FAILED"}")
                    append("\nVertexBuffer: ${if (vbResult) "PASSED" else "FAILED"}")
                    append("\nFramebuffer: ${if (fbResult) "PASSED" else "FAILED"}")
                    append("\nPipeline: ${if (pipelineResult) "PASSED" else "FAILED"}")
                }
            }
            displayText = result
        }

        setContent {
            FantasyTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Greeting(
                        name = displayText,
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
    }
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = name,
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    FantasyTheme {
        Greeting("Fantasy v0.1.0 - RHI ready")
    }
}
