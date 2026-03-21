package com.fantasy.viewmodel

import android.content.ContentValues
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Matrix
import android.graphics.RectF
import android.net.Uri
import android.opengl.GLSurfaceView
import android.os.Build
import android.os.Environment
import android.provider.MediaStore
import androidx.compose.runtime.State
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.exifinterface.media.ExifInterface
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.fantasy.renderer.FantasyRenderer
import com.fantasy.renderer.LUTPreset
import com.fantasy.renderer.LUTPresets
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.InputStream
import java.nio.ByteBuffer
enum class AspectRatioMode(val label: String, val ratio: Float?) {
    FREE("自由", null),
    RATIO_1_1("1:1", 1f),
    RATIO_4_3("4:3", 4f / 3f),
    RATIO_16_9("16:9", 16f / 9f)
}

class EditorViewModel : ViewModel() {

    // --- Public state ---
    private val _originalBitmap = mutableStateOf<Bitmap?>(null)
    val originalBitmap: State<Bitmap?> = _originalBitmap

    private val _brightness = mutableFloatStateOf(0f)
    val brightness: State<Float> = _brightness

    private val _contrast = mutableFloatStateOf(0f)
    val contrast: State<Float> = _contrast

    private val _saturation = mutableFloatStateOf(0f)
    val saturation: State<Float> = _saturation

    private val _sharpness = mutableFloatStateOf(0f)
    val sharpness: State<Float> = _sharpness

    private val _blur = mutableFloatStateOf(0f)
    val blur: State<Float> = _blur

    private val _vignette = mutableFloatStateOf(0f)
    val vignette: State<Float> = _vignette

    private val _selectedPreset = mutableStateOf("None")
    val selectedPreset: State<String> = _selectedPreset

    private val _lutStrength = mutableFloatStateOf(1f)
    val lutStrength: State<Float> = _lutStrength

    // Crop & rotation state
    private val _cropRect = mutableStateOf(RectF(0f, 0f, 1f, 1f))
    val cropRect: State<RectF> = _cropRect

    private val _rotation90 = mutableIntStateOf(0)
    val rotation90: State<Int> = _rotation90

    private val _freeRotation = mutableFloatStateOf(0f)
    val freeRotation: State<Float> = _freeRotation

    private val _isEditingCrop = mutableStateOf(false)
    val isEditingCrop: State<Boolean> = _isEditingCrop

    private val _aspectRatioMode = mutableStateOf(AspectRatioMode.FREE)
    val aspectRatioMode: State<AspectRatioMode> = _aspectRatioMode

    private val _isSaving = mutableStateOf(false)
    val isSaving: State<Boolean> = _isSaving

    // One-shot events
    private val _errorEvent = MutableSharedFlow<String>()
    val errorEvent: SharedFlow<String> = _errorEvent

    private val _saveSuccessEvent = MutableSharedFlow<String>()
    val saveSuccessEvent: SharedFlow<String> = _saveSuccessEvent

    // --- Internal cache ---
    private var cachedRgbaBytes: ByteArray? = null
    private var cachedWidth = 0
    private var cachedHeight = 0

    // --- Renderer binding ---
    private var fantasyRenderer: FantasyRenderer? = null
    private var glSurfaceView: GLSurfaceView? = null

    companion object {
        private const val MAX_LONG_EDGE = 1920
    }

    fun bindRenderer(renderer: FantasyRenderer, view: GLSurfaceView) {
        fantasyRenderer = renderer
        glSurfaceView = view
        // If image was already loaded before binding, push it to renderer
        cachedRgbaBytes?.let { bytes ->
            renderer.setImage(bytes, cachedWidth, cachedHeight)
            requestRender()
        }
    }

    fun loadBuiltinImage(context: Context) {
        viewModelScope.launch {
            val bitmap = withContext(Dispatchers.IO) {
                val raw = BitmapFactory.decodeResource(
                    context.resources,
                    com.fantasy.R.drawable.builtin_sample
                )
                scaleBitmap(raw)
            }
            onBitmapLoaded(bitmap)
        }
    }

    fun loadImage(uri: Uri, context: Context) {
        viewModelScope.launch {
            val bitmap = withContext(Dispatchers.IO) {
                val exifStream = context.contentResolver.openInputStream(uri)
                val rotation = exifStream?.use { getExifRotation(it) } ?: 0

                val options = BitmapFactory.Options().apply { inJustDecodeBounds = true }
                context.contentResolver.openInputStream(uri)?.use {
                    BitmapFactory.decodeStream(it, null, options)
                }
                options.inSampleSize = calculateInSampleSize(options.outWidth, options.outHeight)
                options.inJustDecodeBounds = false

                val raw = context.contentResolver.openInputStream(uri)?.use {
                    BitmapFactory.decodeStream(it, null, options)
                } ?: return@withContext null

                val rotated = applyRotation(raw, rotation)
                scaleBitmap(rotated)
            }
            bitmap?.let { onBitmapLoaded(it) }
        }
    }

    fun updateBrightness(value: Float) {
        _brightness.floatValue = value
        requestRender()
    }

    fun updateContrast(value: Float) {
        _contrast.floatValue = value
        requestRender()
    }

    fun updateSaturation(value: Float) {
        _saturation.floatValue = value
        requestRender()
    }

    fun updateSharpness(value: Float) {
        _sharpness.floatValue = value
        requestRender()
    }

    fun updateBlur(value: Float) {
        _blur.floatValue = value
        requestRender()
    }

    fun updateVignette(value: Float) {
        _vignette.floatValue = value
        requestRender()
    }

    fun selectPreset(preset: LUTPreset) {
        _selectedPreset.value = preset.name
        if (preset.generator != null) {
            _lutStrength.floatValue = 1f
            viewModelScope.launch {
                val lutData = withContext(Dispatchers.Default) { preset.generator.invoke() }
                fantasyRenderer?.setLUT(lutData, 512, 512)
                requestRender()
            }
        } else {
            requestRender()
        }
    }

    fun updateLutStrength(value: Float) {
        _lutStrength.floatValue = value
        requestRender()
    }

    // --- Crop & Rotation ---

    fun enterCropMode() {
        _isEditingCrop.value = true
    }

    fun exitCropMode(apply: Boolean) {
        if (!apply) {
            // Cancel: reset to previous committed crop
            // (crop rect stays as-is since we edit in place)
        }
        _isEditingCrop.value = false
        requestRender()
    }

    fun updateCropRect(rect: RectF) {
        _cropRect.value = rect
        requestRender()
    }

    fun rotate90CW() {
        _rotation90.intValue = (_rotation90.intValue + 1) % 4
        // Reset crop to full image after rotation
        _cropRect.value = RectF(0f, 0f, 1f, 1f)
        requestRender()
    }

    fun updateFreeRotation(degrees: Float) {
        _freeRotation.floatValue = degrees
        requestRender()
    }

    fun setAspectRatioMode(mode: AspectRatioMode) {
        _aspectRatioMode.value = mode
        if (mode.ratio == null) {
            // FREE mode: reset to full image
            _cropRect.value = RectF(0f, 0f, 1f, 1f)
            requestRender()
            return
        }
        // Compute the largest crop rect with target ratio that fits the full image
        if (mode.ratio != null) {
            // Effective image aspect after 90° rotation
            val imgW = if (_rotation90.intValue % 2 == 0) cachedWidth else cachedHeight
            val imgH = if (_rotation90.intValue % 2 == 0) cachedHeight else cachedWidth
            val imgAspect = imgW.toFloat() / imgH

            // Target aspect in normalized crop space (1.0 x 1.0 = full image)
            val targetAspect = mode.ratio / imgAspect

            // Fit the largest rect with targetAspect inside [0,1] x [0,1]
            val newW: Float
            val newH: Float
            if (targetAspect > 1f) {
                // Target is wider than image: full width, reduce height
                newW = 1f
                newH = 1f / targetAspect
            } else {
                // Target is taller than image: full height, reduce width
                newH = 1f
                newW = targetAspect
            }

            // Center in the image
            val left = (1f - newW) / 2f
            val top = (1f - newH) / 2f
            _cropRect.value = RectF(left, top, left + newW, top + newH)
        }
        requestRender()
    }

    /** Compute output dimensions after crop+rotation for export */
    private fun computeExportDimensions(): Pair<Int, Int> {
        val r90 = _rotation90.intValue
        val effectiveW = if (r90 % 2 == 0) cachedWidth else cachedHeight
        val effectiveH = if (r90 % 2 == 0) cachedHeight else cachedWidth
        val crop = _cropRect.value
        val outW = (effectiveW * crop.width()).toInt().coerceAtLeast(1)
        val outH = (effectiveH * crop.height()).toInt().coerceAtLeast(1)
        return Pair(outW, outH)
    }

    fun exportImage(context: Context) {
        val w = cachedWidth
        val h = cachedHeight
        if (cachedRgbaBytes == null || w == 0 || h == 0) return

        val (exportW, exportH) = computeExportDimensions()

        viewModelScope.launch {
            _isSaving.value = true

            val result = withContext(Dispatchers.IO) {
                fantasyRenderer?.exportImage(w, h)
            }

            if (result != null) {
                val bmp = Bitmap.createBitmap(exportW, exportH, Bitmap.Config.ARGB_8888)
                bmp.copyPixelsFromBuffer(ByteBuffer.wrap(result))

                val saved = withContext(Dispatchers.IO) {
                    saveToGallery(context, bmp)
                }

                if (saved) {
                    _saveSuccessEvent.emit("已保存到相册")
                } else {
                    _errorEvent.emit("保存失败")
                }
            } else {
                _errorEvent.emit("处理失败")
            }

            _isSaving.value = false
        }
    }

    // --- Internal ---

    private fun saveToGallery(context: Context, bitmap: Bitmap): Boolean {
        val filename = "Fantasy_${System.currentTimeMillis()}.jpg"
        val contentValues = ContentValues().apply {
            put(MediaStore.Images.Media.DISPLAY_NAME, filename)
            put(MediaStore.Images.Media.MIME_TYPE, "image/jpeg")
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                put(MediaStore.Images.Media.RELATIVE_PATH, Environment.DIRECTORY_PICTURES + "/Fantasy")
                put(MediaStore.Images.Media.IS_PENDING, 1)
            }
        }

        val resolver = context.contentResolver
        val uri = resolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, contentValues)
            ?: return false

        return try {
            resolver.openOutputStream(uri)?.use { out ->
                bitmap.compress(Bitmap.CompressFormat.JPEG, 95, out)
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                contentValues.clear()
                contentValues.put(MediaStore.Images.Media.IS_PENDING, 0)
                resolver.update(uri, contentValues, null, null)
            }
            true
        } catch (_: Exception) {
            resolver.delete(uri, null, null)
            false
        }
    }

    private fun onBitmapLoaded(bitmap: Bitmap) {
        _originalBitmap.value = bitmap
        val ensured = bitmap.copy(Bitmap.Config.ARGB_8888, false)
        val buffer = ByteBuffer.allocate(ensured.byteCount)
        ensured.copyPixelsToBuffer(buffer)
        cachedRgbaBytes = buffer.array()
        cachedWidth = ensured.width
        cachedHeight = ensured.height
        _brightness.floatValue = 0f
        _contrast.floatValue = 0f
        _saturation.floatValue = 0f
        _sharpness.floatValue = 0f
        _blur.floatValue = 0f
        _vignette.floatValue = 0f
        _selectedPreset.value = "None"
        _lutStrength.floatValue = 1f
        _cropRect.value = RectF(0f, 0f, 1f, 1f)
        _rotation90.intValue = 0
        _freeRotation.floatValue = 0f
        _isEditingCrop.value = false
        _aspectRatioMode.value = AspectRatioMode.FREE
        fantasyRenderer?.setImage(buffer.array(), ensured.width, ensured.height)
        requestRender()
    }

    private fun requestRender() {
        val config = buildString {
            // Crop & rotation params (always included when non-default)
            val crop = _cropRect.value
            val r90 = _rotation90.intValue
            val fr = _freeRotation.floatValue
            val isEditing = _isEditingCrop.value

            if (!isEditing) {
                // Only apply crop when not in editing mode
                val hasCrop = crop.left != 0f || crop.top != 0f ||
                        crop.width() != 1f || crop.height() != 1f
                if (hasCrop) {
                    appendLine("crop_x:${crop.left}")
                    appendLine("crop_y:${crop.top}")
                    appendLine("crop_w:${crop.width()}")
                    appendLine("crop_h:${crop.height()}")
                }
            }
            if (r90 != 0) appendLine("rotation90:$r90")
            if (fr != 0f) appendLine("free_rotation:${Math.toRadians(fr.toDouble()).toFloat()}")

            // Filter params
            if (_selectedPreset.value != "None") {
                appendLine("lut_strength:${_lutStrength.floatValue}")
            }
            val b = _brightness.floatValue
            val c = _contrast.floatValue
            val s = _saturation.floatValue
            val sh = _sharpness.floatValue
            val bl = _blur.floatValue
            val v = _vignette.floatValue
            if (b != 0f) appendLine("brightness:$b")
            if (c != 0f) appendLine("contrast:$c")
            if (s != 0f) appendLine("saturation:$s")
            if (sh != 0f) appendLine("sharpness:$sh")
            if (bl != 0f) appendLine("blur:$bl")
            if (v != 0f) appendLine("vignette:$v")
        }
        fantasyRenderer?.setFilterConfig(config)
    }

    private fun scaleBitmap(bitmap: Bitmap): Bitmap {
        val longEdge = maxOf(bitmap.width, bitmap.height)
        if (longEdge <= MAX_LONG_EDGE) return bitmap
        val scale = MAX_LONG_EDGE.toFloat() / longEdge
        val newW = (bitmap.width * scale).toInt()
        val newH = (bitmap.height * scale).toInt()
        return Bitmap.createScaledBitmap(bitmap, newW, newH, true)
    }

    private fun getExifRotation(inputStream: InputStream): Int {
        return try {
            val exif = ExifInterface(inputStream)
            when (exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL)) {
                ExifInterface.ORIENTATION_ROTATE_90 -> 90
                ExifInterface.ORIENTATION_ROTATE_180 -> 180
                ExifInterface.ORIENTATION_ROTATE_270 -> 270
                else -> 0
            }
        } catch (_: Exception) {
            0
        }
    }

    private fun applyRotation(bitmap: Bitmap, degrees: Int): Bitmap {
        if (degrees == 0) return bitmap
        val matrix = Matrix().apply { postRotate(degrees.toFloat()) }
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.width, bitmap.height, matrix, true)
    }

    private fun calculateInSampleSize(width: Int, height: Int): Int {
        var inSampleSize = 1
        val longEdge = maxOf(width, height)
        val target = MAX_LONG_EDGE * 2
        while (longEdge / inSampleSize > target) {
            inSampleSize *= 2
        }
        return inSampleSize
    }
}
