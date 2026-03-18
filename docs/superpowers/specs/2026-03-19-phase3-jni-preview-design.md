# Phase 3 — JNI Bridge + Preview Design

## Overview

Phase 3 connects the C++ filter framework to the Android UI, enabling users to load an image, apply brightness filter, and see the result on screen. This is the first end-to-end user-facing feature of Fantasy.

## Architecture & Data Flow

```
User action (pick image / drag slider)
       |
   Compose UI (EditorScreen)
       | event
   EditorViewModel
       | coroutine (Dispatchers.Default.limitedParallelism(1), debounce 100ms)
   NativeBridge.nativeApplyFilters(imageBytes, width, height, filterConfig)
       | JNI
   C++ NativeBridge: parse config -> FilterChain -> apply -> readPixels
       | byte[]
   EditorViewModel: byte[] -> Bitmap -> update State
       |
   Compose Image displays preview
```

Key decisions:
- Bitmap round-trip approach (not GLSurfaceView): C++ does offscreen rendering, returns byte[] to Java. Simple, matches CLAUDE.md's "coarse JNI, pass byte[]" design. Same pipeline reused for Phase 5 export.
- Single Activity + ViewModel + Compose: no premature Activity splitting.
- Debounce 100ms on parameter changes to avoid excessive rendering during slider drag.
- JNI calls serialized via `Dispatchers.Default.limitedParallelism(1)` to prevent concurrent EGL/GL access from multiple coroutines.

## JNI Interface

### New Method

```cpp
JNIEXPORT jbyteArray JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeApplyFilters(
    JNIEnv *env, jobject,
    jbyteArray imageData,    // input RGBA byte[]
    jint width,
    jint height,
    jstring filterConfig     // filter configuration string
);
```

### Filter Config Format

Simple text format, one filter per line, `name:value`. Each filter has one canonical parameter:

```
brightness:0.2
contrast:0.0
saturation:0.0
```

This is a deliberate simplification: each filter in the initial set (brightness, contrast, saturation) has exactly one parameter. The format maps filter name to its single canonical value. If a future filter needs multiple parameters, the format can be extended (e.g., `filter_name.param_key:value`), but that is out of scope for Phase 3-4.

### C++ Processing Flow

1. Copy pixel data from `jbyteArray`
2. Parse filterConfig string, create corresponding Filters and set parameters
3. Create EGLHelper context + `RHIShader::clearRegistry()` + `RHIShader::registerBuiltins()` (same pattern as existing tests — per-call context with fresh shader registry)
4. Build texture -> FilterChain.apply -> readPixels
5. Cleanup: `RHIShader::clearRegistry()`, EGLHelper destructor destroys context
6. Return result byte[] to Java

**EGL context lifecycle**: Per-call create/destroy, same as existing test functions. This is acceptable for Phase 3 because: (a) debounce ensures calls are ≤10/sec at most, (b) EGL init/teardown cost (~2-5ms) is small relative to filter rendering time, (c) it avoids complex context caching and thread-affinity issues. If profiling shows this is a bottleneck, context reuse can be added as an optimization later.

Existing test JNI methods are preserved unchanged.

## Kotlin Side

### NativeBridge.kt — New Declaration

```kotlin
external fun nativeApplyFilters(
    imageData: ByteArray, width: Int, height: Int, filterConfig: String
): ByteArray
```

### EditorViewModel

```kotlin
class EditorViewModel : ViewModel() {
    // Dedicated single-thread dispatcher for JNI calls (thread safety)
    private val renderDispatcher = Dispatchers.Default.limitedParallelism(1)

    // State
    val originalBitmap: State<Bitmap?>       // original image
    val previewBitmap: State<Bitmap?>        // filtered preview
    val brightness: State<Float>             // -1.0 ~ 1.0, default 0
    val contrast: State<Float>               // reserved for Phase 4
    val saturation: State<Float>             // reserved for Phase 4
    val isProcessing: State<Boolean>         // rendering indicator

    fun loadImage(uri: Uri, context: Context)   // load from gallery
    fun loadBuiltinImage(context: Context)      // load built-in sample
    fun updateBrightness(value: Float)          // trigger debounced render
}
```

### Image Loading

1. Uri/Resource -> `BitmapFactory.decode` with `inSampleSize` for initial downscale -> ensure `Bitmap.Config.ARGB_8888`
2. Read EXIF orientation via `ExifInterface`, apply rotation if needed via `Matrix` transform
3. If long edge > 1920px, scale down with `Bitmap.createScaledBitmap(src, w, h, true)` (bilinear filter, preserve aspect ratio, never upscale)
4. Bitmap -> `copyPixelsToBuffer` -> byte[] (ARGB_8888 layout). In C++ side, handle as RGBA since Android ARGB_8888 ByteBuffer output is actually RGBA byte order when read sequentially.
5. Cache RGBA byte[], width, height in ViewModel. On parameter change, reuse cached data for JNI call (no re-decode).

## UI Layout

```
+-------------------------------+
|  Toolbar:  [Built-in] [Album] |
+-------------------------------+
|                               |
|                               |
|      Image Preview Area       |
|    (centered, aspect fit)     |
|                               |
|                               |
+-------------------------------+
|  Brightness  --*-------- 0.2  |
|  Contrast    ----*------ 0.0  |  <- disabled (gray)
|  Saturation  ----*------ 0.0  |  <- disabled (gray)
+-------------------------------+
```

### Compose Components

- **EditorScreen** — top-level composable, assembles toolbar + preview + filter panel
- **TopToolBar** — "Built-in image" and "Pick from album" buttons
- **ImagePreview** — centered Bitmap display with loading indicator during rendering
- **FilterPanel** — slider rows: label + Slider + current value. Each slider accepts `enabled` parameter.

Phase 3: only Brightness `enabled = true`. Contrast and Saturation `enabled = false` (grayed out). Phase 4 flips enabled after filters are implemented.

## Image Sources

1. **Built-in sample image**: bundled as `res/drawable/builtin_sample.jpg`, loaded via `BitmapFactory.decodeResource`
2. **Photo Picker**: Android system Photo Picker via `ActivityResultLauncher`, no storage permission needed

## Error Handling & Edge Cases

- **JNI exception**: C++ catches all exceptions, returns null on error. Java side shows toast "Processing failed".
- **Large images**: scaled to long edge <= 1920px on load to prevent OOM and slow rendering. Never upscale small images.
- **No image loaded**: slider area disabled until an image is loaded.
- **Redundant rendering**: debounce 100ms + coroutine cancellation + `limitedParallelism(1)`. New request auto-cancels previous unfinished render, and serialization prevents concurrent GL access.
- **Permissions**: Photo Picker needs no storage permission. No save feature in Phase 3.
- **EXIF orientation**: handled at load time via `ExifInterface` + `Matrix` rotation, so downstream code always works with correctly oriented pixels.

## File Changes

### New Files

- `app/src/main/java/com/fantasy/viewmodel/EditorViewModel.kt`
- `app/src/main/java/com/fantasy/ui/EditorScreen.kt`
- `app/src/main/java/com/fantasy/ui/components/TopToolBar.kt`
- `app/src/main/java/com/fantasy/ui/components/ImagePreview.kt`
- `app/src/main/java/com/fantasy/ui/components/FilterPanel.kt`
- `app/src/main/res/drawable/builtin_sample.jpg`

### Modified Files

- `app/src/main/java/com/fantasy/bridge/NativeBridge.kt` — add `nativeApplyFilters`
- `app/src/main/java/com/fantasy/MainActivity.kt` — replace test UI with EditorScreen
- `native/bridge/src/NativeBridge.cpp` — add `nativeApplyFilters` + config parsing
- `app/build.gradle.kts` — add Compose/ViewModel dependencies if not already present

### Deleted Files

- `app/src/main/java/com/fantasy/ui/PreviewView.kt` — replaced by Compose-based `ImagePreview.kt` (if this file exists; it was planned in CLAUDE.md but may not have been created yet)

### Unchanged

- All RHI layer files
- All Filter layer files
- Existing test JNI methods (preserved)

## Test Criteria (Phase 3 Exit)

### Manual Tests
- Pick a built-in image, apply brightness filter, see the effect on screen
- Pick an image from album, apply brightness filter, see the effect on screen
- Drag brightness slider, preview updates with debounce
- Contrast/Saturation sliders visible but disabled

### Automated Test
- Add `nativeTestApplyFilters()` JNI test: pass known RGBA pixel data + brightness config, verify output pixel values match expected result (reuses existing test pattern from Phase 1/2 tests)
