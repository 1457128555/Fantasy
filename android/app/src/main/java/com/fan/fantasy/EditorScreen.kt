package com.fan.fantasy

import android.graphics.BitmapFactory  
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

        val resolver = context.contentResolver
        val bitmap = resolver.openInputStream(uri)?.use { stream ->
            BitmapFactory.decodeStream(stream)
        }
        bitmap?.let {
            EngineBridge.setImage(it)
            EngineBridge.renderOneFrame();
            it.recycle()
        }
    }

    Box(Modifier.fillMaxSize()) {
        AndroidView(factory = { RenderSurfaceView(it) }, modifier = Modifier.fillMaxSize())
        Button(onClick = onBack, modifier = Modifier.align(Alignment.TopStart).padding(16.dp)) {
            Text("返回")
        }
    }
}
