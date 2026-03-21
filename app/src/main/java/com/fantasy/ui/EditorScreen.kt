package com.fantasy.ui

import android.opengl.GLSurfaceView
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import com.fantasy.renderer.FantasyRenderer
import com.fantasy.renderer.LUTPresets
import com.fantasy.ui.components.FilterPanel
import com.fantasy.ui.components.FilterSliderConfig
import com.fantasy.ui.components.GLPreview
import com.fantasy.ui.components.PresetPanel
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
    val isSaving by viewModel.isSaving
    val brightness by viewModel.brightness
    val contrast by viewModel.contrast
    val saturation by viewModel.saturation
    val sharpness by viewModel.sharpness
    val blur by viewModel.blur
    val vignette by viewModel.vignette
    val selectedPreset by viewModel.selectedPreset
    val lutStrength by viewModel.lutStrength
    val hasImage = viewModel.originalBitmap.value != null

    val renderer = remember { FantasyRenderer() }
    val glSurfaceViewState = remember { mutableStateOf<GLSurfaceView?>(null) }

    LaunchedEffect(glSurfaceViewState.value) {
        val view = glSurfaceViewState.value ?: return@LaunchedEffect
        viewModel.bindRenderer(renderer, view)
    }

    Column(modifier = modifier.fillMaxSize()) {
        TopToolBar(
            onBuiltinClick = onBuiltinClick,
            onAlbumClick = onPickFromAlbum,
            onSaveClick = onSaveClick,
            saveEnabled = hasImage,
            isSaving = isSaving
        )

        Box(
            modifier = Modifier
                .weight(1f)
                .fillMaxWidth()
                .background(MaterialTheme.colorScheme.surfaceVariant),
            contentAlignment = Alignment.Center
        ) {
            if (hasImage) {
                GLPreview(
                    renderer = renderer,
                    onSurfaceViewReady = { glSurfaceViewState.value = it },
                    modifier = Modifier.fillMaxSize()
                )
            } else {
                Text(
                    text = "请选择一张图片",
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }

        PresetPanel(
            presets = LUTPresets.presets,
            selectedPreset = selectedPreset,
            enabled = hasImage,
            onPresetSelected = { viewModel.selectPreset(it) }
        )

        FilterPanel(
            sliders = listOfNotNull(
                if (selectedPreset != "None") FilterSliderConfig(
                    label = "LUT 强度",
                    value = lutStrength,
                    onValueChange = { viewModel.updateLutStrength(it) },
                    enabled = hasImage,
                    valueRange = 0f..1f
                ) else null,
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
                ),
                FilterSliderConfig(
                    label = "锐化",
                    value = sharpness,
                    onValueChange = { viewModel.updateSharpness(it) },
                    enabled = hasImage,
                    valueRange = 0f..1f
                ),
                FilterSliderConfig(
                    label = "模糊",
                    value = blur,
                    onValueChange = { viewModel.updateBlur(it) },
                    enabled = hasImage,
                    valueRange = 0f..1f
                ),
                FilterSliderConfig(
                    label = "暗角",
                    value = vignette,
                    onValueChange = { viewModel.updateVignette(it) },
                    enabled = hasImage,
                    valueRange = 0f..1f
                )
            )
        )
    }
}
