package com.fan.fantasy

import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource

@Composable
fun HomeScreen(onPicked: (Uri) -> Unit) {
    val launcher = rememberLauncherForActivityResult(
        ActivityResultContracts.PickVisualMedia()
    ) { uri: Uri? ->
        if (uri != null) onPicked(uri)           // 选到了才往下走
    }

    Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        // 主页背景:直接用参考图(art/ref-bg.png),Crop 填充+裁切,不变形
        Image(
            painter = painterResource(R.drawable.home_bg),
            contentDescription = null,
            modifier = Modifier.fillMaxSize(),
            contentScale = ContentScale.Crop,
        )
        Button(onClick = {
            launcher.launch(
                PickVisualMediaRequest(ActivityResultContracts.PickVisualMedia.ImageOnly)
            )
        }) {
            Text("选图")
        }
    }
}
