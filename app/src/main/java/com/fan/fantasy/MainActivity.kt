package com.fan.fantasy

import android.graphics.ImageDecoder
import android.net.Uri
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import com.fan.engine.EngineBridge
import com.fan.engine.RenderSurfaceView
import com.fan.fantasy.ui.theme.FantasyTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        EngineBridge.nativeInit()        // 幂等：每次 Activity 创建都确保引擎在（进程复用后重建也安全）
        enableEdgeToEdge()
        setContent {
            FantasyTheme {
                EditorScreen()
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        if (isFinishing) {               // 仅真正退出才拆引擎；配置变更重建不拆
            EngineBridge.nativeDestroy()
        }
    }
}

@Composable
fun EditorScreen() {
    val context = LocalContext.current
    val launcher = rememberLauncherForActivityResult(
        ActivityResultContracts.PickVisualMedia()   
    ) { uri: Uri? ->                                
        if (uri == null) return@rememberLauncherForActivityResult

        val source = ImageDecoder.createSource(context.contentResolver, uri)
        val bitmap = ImageDecoder.decodeBitmap(source) { decoder, _, _ ->
            decoder.allocator = ImageDecoder.ALLOCATOR_SOFTWARE
        }
        EngineBridge.nativeSetImage(bitmap)
        bitmap.recycle()
    }

    Box(Modifier.fillMaxSize()) {
        AndroidView(
            factory = { RenderSurfaceView(it) },   
            modifier = Modifier.fillMaxSize()     
        )
        Button(
            onClick = {
                launcher.launch(
                    PickVisualMediaRequest(ActivityResultContracts.PickVisualMedia.ImageOnly)
                )
            },
            modifier = Modifier
                .align(Alignment.BottomCenter)     
                .padding(24.dp)                    
        ) {
            Text("选图")                            
        }
    }
}
