package com.fantasy.ui.components

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
fun TopToolBar(
    onBuiltinClick: () -> Unit,
    onAlbumClick: () -> Unit,
    onSaveClick: () -> Unit,
    onCropClick: () -> Unit,
    saveEnabled: Boolean,
    cropEnabled: Boolean,
    isSaving: Boolean,
    modifier: Modifier = Modifier
) {
    Row(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 8.dp),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Button(onClick = onBuiltinClick) {
            Text("内置图片")
        }
        Button(onClick = onAlbumClick) {
            Text("从相册选")
        }
        Spacer(modifier = Modifier.weight(1f))
        OutlinedButton(
            onClick = onCropClick,
            enabled = cropEnabled
        ) {
            Text("裁剪")
        }
        Button(
            onClick = onSaveClick,
            enabled = saveEnabled && !isSaving
        ) {
            Text(if (isSaving) "保存中..." else "保存")
        }
    }
}
