package com.fantasy.ui

import android.opengl.GLSurfaceView
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Tab
import androidx.compose.material3.TabRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.fantasy.renderer.FantasyRenderer
import com.fantasy.renderer.LUTPresets
import com.fantasy.ui.components.CropOverlay
import com.fantasy.ui.components.CropToolBar
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
    val canUndo by viewModel.canUndo
    val canRedo by viewModel.canRedo
    val hasImage = viewModel.originalBitmap.value != null
    val isEditingCrop by viewModel.isEditingCrop
    val cropRect by viewModel.cropRect
    val rotation90 by viewModel.rotation90
    val freeRotation by viewModel.freeRotation
    val aspectRatioMode by viewModel.aspectRatioMode

    val renderer = remember { FantasyRenderer() }
    val glSurfaceViewState = remember { mutableStateOf<GLSurfaceView?>(null) }
    var selectedTab by remember { mutableIntStateOf(0) }
    val tabTitles = listOf("预设", "调色", "效果")

    LaunchedEffect(glSurfaceViewState.value) {
        val view = glSurfaceViewState.value ?: return@LaunchedEffect
        viewModel.bindRenderer(renderer, view)
    }

    Column(modifier = modifier.fillMaxSize()) {
        TopToolBar(
            onBuiltinClick = onBuiltinClick,
            onAlbumClick = onPickFromAlbum,
            onSaveClick = onSaveClick,
            onCropClick = { viewModel.enterCropMode() },
            saveEnabled = hasImage && !isEditingCrop,
            cropEnabled = hasImage && !isEditingCrop,
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
                if (isEditingCrop) {
                    CropOverlay(
                        cropRect = cropRect,
                        imageWidth = viewModel.originalBitmap.value?.width ?: 1,
                        imageHeight = viewModel.originalBitmap.value?.height ?: 1,
                        rotation90 = rotation90,
                        aspectRatioMode = aspectRatioMode,
                        onCropRectChanged = { viewModel.updateCropRect(it) },
                        modifier = Modifier.fillMaxSize()
                    )
                }
                // Floating undo/redo buttons at bottom-start
                if (!isEditingCrop && (canUndo || canRedo)) {
                    Row(
                        modifier = Modifier
                            .align(Alignment.BottomStart)
                            .padding(12.dp)
                    ) {
                        Text(
                            text = "↩",
                            fontSize = 22.sp,
                            color = androidx.compose.ui.graphics.Color.White.copy(
                                alpha = if (canUndo) 0.9f else 0.3f
                            ),
                            modifier = Modifier
                                .clickable(enabled = canUndo) { viewModel.undo() }
                                .padding(8.dp)
                        )
                        Text(
                            text = "↪",
                            fontSize = 22.sp,
                            color = androidx.compose.ui.graphics.Color.White.copy(
                                alpha = if (canRedo) 0.9f else 0.3f
                            ),
                            modifier = Modifier
                                .clickable(enabled = canRedo) { viewModel.redo() }
                                .padding(8.dp)
                        )
                    }
                }
            } else {
                Text(
                    text = "请选择一张图片",
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }

        if (isEditingCrop) {
            Box(modifier = Modifier.height(160.dp).fillMaxWidth()) {
                CropToolBar(
                    aspectRatioMode = aspectRatioMode,
                    freeRotation = freeRotation,
                    onAspectRatioChanged = { viewModel.setAspectRatioMode(it) },
                    onRotate90 = { viewModel.rotate90CW() },
                    onFreeRotationChanged = { viewModel.updateFreeRotation(it) },
                    onApply = { viewModel.exitCropMode(true) },
                    onCancel = { viewModel.exitCropMode(false) }
                )
            }
        } else {
            TabRow(selectedTabIndex = selectedTab) {
                tabTitles.forEachIndexed { index, title ->
                    Tab(
                        selected = selectedTab == index,
                        onClick = { selectedTab = index },
                        text = { Text(title) }
                    )
                }
            }

            Box(modifier = Modifier.height(160.dp).fillMaxWidth()) {
                when (selectedTab) {
                    0 -> {
                        Column {
                            PresetPanel(
                                presets = LUTPresets.presets,
                                selectedPreset = selectedPreset,
                                enabled = hasImage,
                                onPresetSelected = { viewModel.selectPreset(it) }
                            )
                            if (selectedPreset != "None") {
                                FilterPanel(
                                    sliders = listOf(
                                        FilterSliderConfig(
                                            label = "LUT 强度",
                                            value = lutStrength,
                                            onValueChange = { viewModel.updateLutStrength(it) },
                                            onValueChangeStarted = { viewModel.beginParameterChange() },
                                            onValueChangeFinished = { viewModel.commitParameterChange() },
                                            enabled = hasImage,
                                            valueRange = 0f..1f
                                        )
                                    )
                                )
                            }
                        }
                    }
                    1 -> {
                        FilterPanel(
                            sliders = listOf(
                                FilterSliderConfig(
                                    label = "亮度",
                                    value = brightness,
                                    onValueChange = { viewModel.updateBrightness(it) },
                                    onValueChangeStarted = { viewModel.beginParameterChange() },
                                    onValueChangeFinished = { viewModel.commitParameterChange() },
                                    enabled = hasImage
                                ),
                                FilterSliderConfig(
                                    label = "对比度",
                                    value = contrast,
                                    onValueChange = { viewModel.updateContrast(it) },
                                    onValueChangeStarted = { viewModel.beginParameterChange() },
                                    onValueChangeFinished = { viewModel.commitParameterChange() },
                                    enabled = hasImage
                                ),
                                FilterSliderConfig(
                                    label = "饱和度",
                                    value = saturation,
                                    onValueChange = { viewModel.updateSaturation(it) },
                                    onValueChangeStarted = { viewModel.beginParameterChange() },
                                    onValueChangeFinished = { viewModel.commitParameterChange() },
                                    enabled = hasImage
                                )
                            )
                        )
                    }
                    2 -> {
                        FilterPanel(
                            sliders = listOf(
                                FilterSliderConfig(
                                    label = "锐化",
                                    value = sharpness,
                                    onValueChange = { viewModel.updateSharpness(it) },
                                    onValueChangeStarted = { viewModel.beginParameterChange() },
                                    onValueChangeFinished = { viewModel.commitParameterChange() },
                                    enabled = hasImage,
                                    valueRange = 0f..1f
                                ),
                                FilterSliderConfig(
                                    label = "模糊",
                                    value = blur,
                                    onValueChange = { viewModel.updateBlur(it) },
                                    onValueChangeStarted = { viewModel.beginParameterChange() },
                                    onValueChangeFinished = { viewModel.commitParameterChange() },
                                    enabled = hasImage,
                                    valueRange = 0f..1f
                                ),
                                FilterSliderConfig(
                                    label = "暗角",
                                    value = vignette,
                                    onValueChange = { viewModel.updateVignette(it) },
                                    onValueChangeStarted = { viewModel.beginParameterChange() },
                                    onValueChangeFinished = { viewModel.commitParameterChange() },
                                    enabled = hasImage,
                                    valueRange = 0f..1f
                                )
                            )
                        )
                    }
                }
            }
        }
    }
}
