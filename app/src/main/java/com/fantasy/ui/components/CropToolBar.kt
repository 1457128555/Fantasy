package com.fantasy.ui.components

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.Button
import androidx.compose.material3.FilterChip
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.fantasy.viewmodel.AspectRatioMode

@Composable
fun CropToolBar(
    aspectRatioMode: AspectRatioMode,
    freeRotation: Float,
    onAspectRatioChanged: (AspectRatioMode) -> Unit,
    onRotate90: () -> Unit,
    onFreeRotationChanged: (Float) -> Unit,
    onApply: () -> Unit,
    onCancel: () -> Unit,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 8.dp)
    ) {
        // Aspect ratio chips + rotate button
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            AspectRatioMode.entries.forEach { mode ->
                FilterChip(
                    selected = aspectRatioMode == mode,
                    onClick = { onAspectRatioChanged(mode) },
                    label = { Text(mode.label, style = MaterialTheme.typography.bodySmall) }
                )
            }
            Spacer(modifier = Modifier.weight(1f))
            OutlinedButton(onClick = onRotate90) {
                Text("90°")
            }
        }

        // Free rotation slider
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "旋转",
                modifier = Modifier.width(48.dp),
                style = MaterialTheme.typography.bodyMedium
            )
            Slider(
                value = freeRotation,
                onValueChange = onFreeRotationChanged,
                valueRange = -45f..45f,
                modifier = Modifier.weight(1f)
            )
            Text(
                text = String.format("%.1f°", freeRotation),
                modifier = Modifier.width(48.dp),
                style = MaterialTheme.typography.bodySmall
            )
        }

        // Apply / Cancel buttons
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.End
        ) {
            OutlinedButton(onClick = onCancel) {
                Text("取消")
            }
            Spacer(modifier = Modifier.width(12.dp))
            Button(onClick = onApply) {
                Text("确定")
            }
        }
    }
}
