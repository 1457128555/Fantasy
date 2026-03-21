package com.fantasy.ui.components

import android.graphics.RectF
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Rect
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import com.fantasy.viewmodel.AspectRatioMode

private const val HANDLE_RADIUS = 20f
private const val HANDLE_TOUCH_RADIUS = 48f
private const val MIN_CROP_SIZE = 0.05f

// Which part of the crop rect is being dragged
private const val DRAG_NONE = 0
private const val DRAG_MOVE = 1
private const val DRAG_TOP_LEFT = 2
private const val DRAG_TOP_RIGHT = 3
private const val DRAG_BOTTOM_LEFT = 4
private const val DRAG_BOTTOM_RIGHT = 5
private const val DRAG_TOP = 6
private const val DRAG_BOTTOM = 7
private const val DRAG_LEFT = 8
private const val DRAG_RIGHT = 9

@Composable
fun CropOverlay(
    cropRect: RectF,
    imageWidth: Int,
    imageHeight: Int,
    rotation90: Int,
    aspectRatioMode: AspectRatioMode,
    onCropRectChanged: (RectF) -> Unit,
    modifier: Modifier = Modifier
) {
    // Effective image dimensions after 90° rotation
    val effectiveW = if (rotation90 % 2 == 0) imageWidth else imageHeight
    val effectiveH = if (rotation90 % 2 == 0) imageHeight else imageWidth

    // rememberUpdatedState so pointerInput lambda always reads latest values
    val currentCropRect by rememberUpdatedState(cropRect)
    val currentCallback by rememberUpdatedState(onCropRectChanged)
    val currentAspectRatio by rememberUpdatedState(aspectRatioMode.ratio)
    val currentImageAspect by rememberUpdatedState(effectiveW.toFloat() / effectiveH)

    var dragTarget by remember { mutableIntStateOf(DRAG_NONE) }
    var imageRect by remember { mutableStateOf(Rect.Zero) }

    Canvas(
        modifier = modifier
            .fillMaxSize()
            .pointerInput(Unit) {
                detectDragGestures(
                    onDragStart = { offset ->
                        dragTarget = hitTest(offset, currentCropRect, imageRect)
                    },
                    onDrag = { change, dragAmount ->
                        change.consume()
                        if (dragTarget != DRAG_NONE && imageRect.width > 0) {
                            val dx = dragAmount.x / imageRect.width
                            val dy = dragAmount.y / imageRect.height
                            val newRect = applyDrag(
                                currentCropRect, dragTarget, dx, dy,
                                currentAspectRatio,
                                currentImageAspect
                            )
                            currentCallback(newRect)
                        }
                    },
                    onDragEnd = { dragTarget = DRAG_NONE }
                )
            }
    ) {
        // Compute where the image is displayed (letterbox)
        val imgAspect = effectiveW.toFloat() / effectiveH
        val viewAspect = size.width / size.height
        val imgRect: Rect
        if (imgAspect > viewAspect) {
            val w = size.width
            val h = w / imgAspect
            val y = (size.height - h) / 2f
            imgRect = Rect(0f, y, w, y + h)
        } else {
            val h = size.height
            val w = h * imgAspect
            val x = (size.width - w) / 2f
            imgRect = Rect(x, 0f, x + w, h)
        }
        imageRect = imgRect

        // Convert normalized crop to pixel coords
        val cropPixelRect = Rect(
            left = imgRect.left + cropRect.left * imgRect.width,
            top = imgRect.top + cropRect.top * imgRect.height,
            right = imgRect.left + cropRect.right * imgRect.width,
            bottom = imgRect.top + cropRect.bottom * imgRect.height
        )

        // Draw dimmed overlay outside crop
        drawDimOverlay(cropPixelRect)

        // Draw crop border
        drawRect(
            color = Color.White,
            topLeft = cropPixelRect.topLeft,
            size = cropPixelRect.size,
            style = Stroke(width = 2f)
        )

        // Draw grid (rule of thirds)
        drawGrid(cropPixelRect)

        // Draw corner handles
        drawHandles(cropPixelRect)
    }
}

private fun DrawScope.drawDimOverlay(crop: Rect) {
    val dim = Color.Black.copy(alpha = 0.5f)
    // Top
    drawRect(dim, Offset.Zero, Size(size.width, crop.top))
    // Bottom
    drawRect(dim, Offset(0f, crop.bottom), Size(size.width, size.height - crop.bottom))
    // Left
    drawRect(dim, Offset(0f, crop.top), Size(crop.left, crop.height))
    // Right
    drawRect(dim, Offset(crop.right, crop.top), Size(size.width - crop.right, crop.height))
}

private fun DrawScope.drawGrid(crop: Rect) {
    val color = Color.White.copy(alpha = 0.4f)
    val stroke = Stroke(width = 1f, pathEffect = PathEffect.dashPathEffect(floatArrayOf(8f, 8f)))
    // Vertical lines
    for (i in 1..2) {
        val x = crop.left + crop.width * i / 3f
        drawLine(color, Offset(x, crop.top), Offset(x, crop.bottom), strokeWidth = 1f)
    }
    // Horizontal lines
    for (i in 1..2) {
        val y = crop.top + crop.height * i / 3f
        drawLine(color, Offset(crop.left, y), Offset(crop.right, y), strokeWidth = 1f)
    }
}

private fun DrawScope.drawHandles(crop: Rect) {
    val corners = listOf(
        crop.topLeft, crop.topRight, crop.bottomLeft, crop.bottomRight
    )
    val handleColor = Color.White
    val handleLength = 24f
    val handleWidth = 4f

    // Draw L-shaped handles at corners
    for ((i, corner) in corners.withIndex()) {
        val hDir = if (i % 2 == 0) 1f else -1f  // horizontal direction
        val vDir = if (i < 2) 1f else -1f        // vertical direction

        // Horizontal bar
        drawLine(
            handleColor,
            corner,
            Offset(corner.x + handleLength * hDir, corner.y),
            strokeWidth = handleWidth
        )
        // Vertical bar
        drawLine(
            handleColor,
            corner,
            Offset(corner.x, corner.y + handleLength * vDir),
            strokeWidth = handleWidth
        )
    }
}

private fun hitTest(
    pos: Offset,
    cropRect: RectF,
    imageRect: Rect
): Int {
    if (imageRect.width <= 0) return DRAG_NONE

    // Convert to pixel crop rect
    val left = imageRect.left + cropRect.left * imageRect.width
    val top = imageRect.top + cropRect.top * imageRect.height
    val right = imageRect.left + cropRect.right * imageRect.width
    val bottom = imageRect.top + cropRect.bottom * imageRect.height

    val r = HANDLE_TOUCH_RADIUS

    // Check corners first
    if (dist(pos, Offset(left, top)) < r) return DRAG_TOP_LEFT
    if (dist(pos, Offset(right, top)) < r) return DRAG_TOP_RIGHT
    if (dist(pos, Offset(left, bottom)) < r) return DRAG_BOTTOM_LEFT
    if (dist(pos, Offset(right, bottom)) < r) return DRAG_BOTTOM_RIGHT

    // Check edges
    if (pos.x in left..right && kotlin.math.abs(pos.y - top) < r) return DRAG_TOP
    if (pos.x in left..right && kotlin.math.abs(pos.y - bottom) < r) return DRAG_BOTTOM
    if (pos.y in top..bottom && kotlin.math.abs(pos.x - left) < r) return DRAG_LEFT
    if (pos.y in top..bottom && kotlin.math.abs(pos.x - right) < r) return DRAG_RIGHT

    // Check inside for move
    if (pos.x in left..right && pos.y in top..bottom) return DRAG_MOVE

    return DRAG_NONE
}

private fun dist(a: Offset, b: Offset): Float {
    val dx = a.x - b.x
    val dy = a.y - b.y
    return kotlin.math.sqrt(dx * dx + dy * dy)
}

private fun applyDrag(
    crop: RectF,
    target: Int,
    dx: Float,
    dy: Float,
    aspectRatio: Float?,
    imageAspect: Float
): RectF {
    val r = RectF(crop)

    when (target) {
        DRAG_MOVE -> {
            val w = r.width()
            val h = r.height()
            r.left = (r.left + dx).coerceIn(0f, 1f - w)
            r.top = (r.top + dy).coerceIn(0f, 1f - h)
            r.right = r.left + w
            r.bottom = r.top + h
        }
        else -> {
            // Resize based on drag target
            when (target) {
                DRAG_LEFT, DRAG_TOP_LEFT, DRAG_BOTTOM_LEFT ->
                    r.left = (r.left + dx).coerceIn(0f, r.right - MIN_CROP_SIZE)
                DRAG_RIGHT, DRAG_TOP_RIGHT, DRAG_BOTTOM_RIGHT ->
                    r.right = (r.right + dx).coerceIn(r.left + MIN_CROP_SIZE, 1f)
            }
            when (target) {
                DRAG_TOP, DRAG_TOP_LEFT, DRAG_TOP_RIGHT ->
                    r.top = (r.top + dy).coerceIn(0f, r.bottom - MIN_CROP_SIZE)
                DRAG_BOTTOM, DRAG_BOTTOM_LEFT, DRAG_BOTTOM_RIGHT ->
                    r.bottom = (r.bottom + dy).coerceIn(r.top + MIN_CROP_SIZE, 1f)
            }

            // Enforce aspect ratio if set
            if (aspectRatio != null) {
                val targetNormAspect = aspectRatio / imageAspect
                val currentW = r.width()
                val currentH = r.height()
                val currentAspect = currentW / currentH

                if (currentAspect > targetNormAspect) {
                    // Too wide, adjust width
                    val newW = currentH * targetNormAspect
                    when (target) {
                        DRAG_LEFT, DRAG_TOP_LEFT, DRAG_BOTTOM_LEFT ->
                            r.left = r.right - newW
                        else ->
                            r.right = r.left + newW
                    }
                } else {
                    // Too tall, adjust height
                    val newH = currentW / targetNormAspect
                    when (target) {
                        DRAG_TOP, DRAG_TOP_LEFT, DRAG_TOP_RIGHT ->
                            r.top = r.bottom - newH
                        else ->
                            r.bottom = r.top + newH
                    }
                }

                // Clamp to bounds
                if (r.left < 0f) { r.right -= r.left; r.left = 0f }
                if (r.top < 0f) { r.bottom -= r.top; r.top = 0f }
                if (r.right > 1f) { r.left -= (r.right - 1f); r.right = 1f }
                if (r.bottom > 1f) { r.top -= (r.bottom - 1f); r.bottom = 1f }
            }
        }
    }

    return r
}
