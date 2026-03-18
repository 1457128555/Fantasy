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
       | coroutine (Dispatchers.Default, debounce 100ms)
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

Simple text format, one filter per line, `name:value`:

```
brightness:0.2
contrast:0.0
saturation:0.0
```

### C++ Processing Flow

1. Copy pixel data from `jbyteArray`
2. Parse filterConfig string, create corresponding Filters and set parameters
3. Create EGLHelper context (per-call, destroyed after)
4. Build texture -> FilterChain.apply -> readPixels
5. Return result byte[] to Java

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

1. Uri/Resource -> `BitmapFactory.decode` -> scale to max 1920px on long edge
2. Bitmap -> `copyPixelsToBuffer` -> RGBA byte[] cached in ViewModel
3. On parameter change, use cached byte[] for JNI call (no re-decode)

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
- **Large images**: scaled to long edge <= 1920px on load to prevent OOM and slow rendering.
- **No image loaded**: slider area disabled until an image is loaded.
- **Redundant rendering**: debounce 100ms + coroutine cancellation. New request auto-cancels previous unfinished render.
- **Permissions**: Photo Picker needs no storage permission. No save feature in Phase 3.

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

### Unchanged

- All RHI layer files
- All Filter layer files
- Existing test JNI methods (preserved)

## Test Criteria (Phase 3 Exit)

- Pick a built-in image, apply brightness filter, see the effect on screen
- Pick an image from album, apply brightness filter, see the effect on screen
- Drag brightness slider, preview updates with debounce
- Contrast/Saturation sliders visible but disabled
