package com.fantasy.viewmodel

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Matrix
import android.net.Uri
import androidx.compose.runtime.State
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.exifinterface.media.ExifInterface
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.fantasy.bridge.NativeBridge
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.InputStream
import java.nio.ByteBuffer

class EditorViewModel : ViewModel() {

    private val bridge = NativeBridge()

    @OptIn(ExperimentalCoroutinesApi::class)
    private val renderDispatcher = Dispatchers.Default.limitedParallelism(1)

    // --- Public state ---
    private val _originalBitmap = mutableStateOf<Bitmap?>(null)
    val originalBitmap: State<Bitmap?> = _originalBitmap

    private val _previewBitmap = mutableStateOf<Bitmap?>(null)
    val previewBitmap: State<Bitmap?> = _previewBitmap

    private val _brightness = mutableFloatStateOf(0f)
    val brightness: State<Float> = _brightness

    private val _contrast = mutableFloatStateOf(0f)
    val contrast: State<Float> = _contrast

    private val _saturation = mutableFloatStateOf(0f)
    val saturation: State<Float> = _saturation

    private val _isProcessing = mutableStateOf(false)
    val isProcessing: State<Boolean> = _isProcessing

    // Error events (consumed by UI as one-shot)
    private val _errorEvent = MutableSharedFlow<String>()
    val errorEvent: SharedFlow<String> = _errorEvent

    // --- Internal cache ---
    private var cachedRgbaBytes: ByteArray? = null
    private var cachedWidth = 0
    private var cachedHeight = 0
    private var renderJob: Job? = null

    companion object {
        private const val MAX_LONG_EDGE = 1920
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

    // --- Internal ---

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
        _previewBitmap.value = bitmap
    }

    private fun requestRender() {
        renderJob?.cancel()
        renderJob = viewModelScope.launch {
            delay(100)
            applyFilters()
        }
    }

    private suspend fun applyFilters() {
        val bytes = cachedRgbaBytes ?: return
        val w = cachedWidth
        val h = cachedHeight

        _isProcessing.value = true

        val config = buildString {
            appendLine("brightness:${_brightness.floatValue}")
            appendLine("contrast:${_contrast.floatValue}")
            appendLine("saturation:${_saturation.floatValue}")
        }

        val result = withContext(renderDispatcher) {
            bridge.nativeApplyFilters(bytes, w, h, config)
        }

        if (result != null) {
            val bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
            bmp.copyPixelsFromBuffer(ByteBuffer.wrap(result))
            _previewBitmap.value = bmp
        } else {
            _errorEvent.emit("处理失败")
        }

        _isProcessing.value = false
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
