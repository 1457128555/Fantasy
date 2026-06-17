package com.fan.fantasy

import android.graphics.ImageDecoder
import android.net.Uri
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.lifecycle.viewmodel.compose.viewModel
import com.fan.engine.EngineBridge
import com.fan.engine.RenderSurfaceView

@Composable
fun EditorScreen(uriString: String, onBack: () -> Unit) {
    val vm: EditorViewModel = viewModel()           // 生命周期锚点（仍只靠存在起作用）
    val context = LocalContext.current

    LaunchedEffect(uriString) {                      // 进页一次性：按 uri 解码并喂给引擎
        val uri = Uri.parse(uriString)
        val source = ImageDecoder.createSource(context.contentResolver, uri)
        val bitmap = ImageDecoder.decodeBitmap(source) { decoder, _, _ ->
            decoder.allocator = ImageDecoder.ALLOCATOR_SOFTWARE   // 必须软件位图，否则 JNI 锁不住像素
        }
        EngineBridge.setImage(bitmap)          // 现在它只「标脏」，不抢 surface 的跑了
        bitmap.recycle()
    }

    Box(Modifier.fillMaxSize()) {
        AndroidView(factory = { RenderSurfaceView(it) }, modifier = Modifier.fillMaxSize())
        Button(onClick = onBack, modifier = Modifier.align(Alignment.TopStart).padding(16.dp)) {
            Text("返回")
        }
    }
}
