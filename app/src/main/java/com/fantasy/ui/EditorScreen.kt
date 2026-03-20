package com.fantasy.ui

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import com.fantasy.ui.components.FilterPanel
import com.fantasy.ui.components.FilterSliderConfig
import com.fantasy.ui.components.ImagePreview
import com.fantasy.ui.components.TopToolBar
import com.fantasy.viewmodel.EditorViewModel

@Composable
fun EditorScreen(
    viewModel: EditorViewModel,
    onBuiltinClick: () -> Unit,
    onPickFromAlbum: () -> Unit,
    onSaveClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    val previewBitmap by viewModel.previewBitmap
    val isProcessing by viewModel.isProcessing
    val isSaving by viewModel.isSaving
    val brightness by viewModel.brightness
    val contrast by viewModel.contrast
    val saturation by viewModel.saturation
    val hasImage = viewModel.originalBitmap.value != null

    Column(modifier = modifier.fillMaxSize()) {
        TopToolBar(
            onBuiltinClick = onBuiltinClick,
            onAlbumClick = onPickFromAlbum,
            onSaveClick = onSaveClick,
            saveEnabled = hasImage,
            isSaving = isSaving
        )

        ImagePreview(
            bitmap = previewBitmap,
            isProcessing = isProcessing,
            modifier = Modifier.weight(1f)
        )

        FilterPanel(
            sliders = listOf(
                FilterSliderConfig(
                    label = "亮度",
                    value = brightness,
                    onValueChange = { viewModel.updateBrightness(it) },
                    enabled = hasImage
                ),
                FilterSliderConfig(
                    label = "对比度",
                    value = contrast,
                    onValueChange = { viewModel.updateContrast(it) },
                    enabled = hasImage
                ),
                FilterSliderConfig(
                    label = "饱和度",
                    value = saturation,
                    onValueChange = { viewModel.updateSaturation(it) },
                    enabled = hasImage
                )
            )
        )
    }
}
