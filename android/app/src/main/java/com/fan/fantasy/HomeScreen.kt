package com.fan.fantasy

import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.ui.draw.shadow
import androidx.compose.material.icons.filled.Add
import androidx.compose.material3.ButtonDefaults
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource

@Composable
fun HomeScreen(onPicked: (Uri) -> Unit) {
    val brand = Color(0xFF803D57)

    val launcher = rememberLauncherForActivityResult(
        ActivityResultContracts.PickVisualMedia()
    ) { uri: Uri? ->
        if (uri != null) onPicked(uri)           // 选到了才往下走
    }

    // 一个 Box = ZStack:背景图垫底,Column 浮在上面
    Box(Modifier.fillMaxSize()) {
        // 背景:参考图,先写=最底层。Crop 填充+裁切,不变形
        Image(
            painter = painterResource(R.drawable.home_bg),
            contentDescription = null,
            modifier = Modifier.fillMaxSize(),
            contentScale = ContentScale.Crop,
        )

        // 前景:顶栏 + 下方按钮
        Column(
            Modifier
                .fillMaxSize()
                .systemBarsPadding(),          // 避开状态栏 + 导航栏
            horizontalAlignment = Alignment.CenterHorizontally,   // 让按钮水平居中
        ) {
            // 顶栏:名字靠左 + 齿轮靠右
            Row(
                Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 36.dp, vertical = 8.dp)   // 离左右边 36,和 iOS 一致
            ) {
                Text(
                    "Fantasy",
                    color = brand,
                    fontSize = 32.sp,
                    fontWeight = FontWeight.Bold,
                    letterSpacing = 0.5.sp,        // 字距,≈ iOS 的 .tracking(0.5)
                )
                Spacer(Modifier.weight(1f))       // 把齿轮挤到最右
                Icon(
                    Icons.Filled.Settings,
                    contentDescription = "设置",
                    tint = brand,                  // 给齿轮上色 ≈ iOS 的 .foregroundStyle(brand)
                    modifier = Modifier.size(26.dp),
                )
            }

            Spacer(Modifier.weight(1f))           // 把按钮挤到下方

            Button(
                onClick = {
                    launcher.launch(
                        PickVisualMediaRequest(ActivityResultContracts.PickVisualMedia.ImageOnly)
                    )
                },
                shape = CircleShape,                                  // 宽按钮 + 圆形 = 胶囊 ≈ iOS Capsule
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color.White.copy(alpha = 0.92f),// 底色 ≈ iOS .white.opacity(0.92)
                    contentColor = brand,                            // 文字+图标统一品牌色
                ),
                contentPadding = PaddingValues(vertical = 18.dp),    // 只管高度;宽度交给 fillMaxWidth
                elevation = null,                                    // 关掉 Material 灰色硬阴影
                modifier = Modifier
                    .fillMaxWidth(0.7f)                              // 宽 = 屏宽 70%
                    .padding(bottom = 48.dp)                         // 离底边留空隙
                    .shadow(                                         // 自己画柔和+染色阴影 ≈ iOS
                        elevation = 12.dp,
                        shape = CircleShape,
                        spotColor = brand,
                        ambientColor = brand,
                    ),
            ) {
                Icon(Icons.Filled.Add, contentDescription = null, modifier = Modifier.size(20.dp))
                Spacer(Modifier.width(8.dp))
                Text("选图", fontSize = 19.sp, fontWeight = FontWeight.SemiBold)
            }
        }
    }
}
