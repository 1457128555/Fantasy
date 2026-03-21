package com.fantasy.renderer

data class LUTPreset(
    val name: String,
    val displayName: String,
    val generator: (() -> ByteArray)?
)

object LUTPresets {

    val presets = listOf(
        LUTPreset("None", "原图", null),
        LUTPreset("Warm", "暖色", ::generateWarm),
        LUTPreset("Cool", "冷色", ::generateCool),
        LUTPreset("Vintage", "复古", ::generateVintage)
    )

    private const val LUT_SIZE = 64
    private const val GRID = 8
    private const val ATLAS = 512 // GRID * LUT_SIZE

    private fun generateIdentity(): ByteArray {
        val data = ByteArray(ATLAS * ATLAS * 4)
        for (py in 0 until ATLAS) {
            for (px in 0 until ATLAS) {
                val tileX = px / LUT_SIZE
                val tileY = py / LUT_SIZE
                val localX = px % LUT_SIZE
                val localY = py % LUT_SIZE
                val blueIndex = tileY * GRID + tileX

                val r = localX * 255 / (LUT_SIZE - 1)
                val g = localY * 255 / (LUT_SIZE - 1)
                val b = blueIndex * 255 / (LUT_SIZE - 1)

                val offset = (py * ATLAS + px) * 4
                data[offset] = r.toByte()
                data[offset + 1] = g.toByte()
                data[offset + 2] = b.toByte()
                data[offset + 3] = 0xFF.toByte()
            }
        }
        return data
    }

    private fun generateWithTransform(transform: (Float, Float, Float) -> Triple<Float, Float, Float>): ByteArray {
        val data = ByteArray(ATLAS * ATLAS * 4)
        for (py in 0 until ATLAS) {
            for (px in 0 until ATLAS) {
                val tileX = px / LUT_SIZE
                val tileY = py / LUT_SIZE
                val localX = px % LUT_SIZE
                val localY = py % LUT_SIZE
                val blueIndex = tileY * GRID + tileX

                val rIn = localX.toFloat() / (LUT_SIZE - 1)
                val gIn = localY.toFloat() / (LUT_SIZE - 1)
                val bIn = blueIndex.toFloat() / (LUT_SIZE - 1)

                val (rOut, gOut, bOut) = transform(rIn, gIn, bIn)

                val offset = (py * ATLAS + px) * 4
                data[offset] = (rOut.coerceIn(0f, 1f) * 255).toInt().toByte()
                data[offset + 1] = (gOut.coerceIn(0f, 1f) * 255).toInt().toByte()
                data[offset + 2] = (bOut.coerceIn(0f, 1f) * 255).toInt().toByte()
                data[offset + 3] = 0xFF.toByte()
            }
        }
        return data
    }

    private fun generateWarm(): ByteArray = generateWithTransform { r, g, b ->
        Triple(
            r * 1.08f + 0.02f,
            g * 1.02f,
            b * 0.85f
        )
    }

    private fun generateCool(): ByteArray = generateWithTransform { r, g, b ->
        Triple(
            r * 0.88f,
            g * 0.98f,
            b * 1.10f + 0.03f
        )
    }

    private fun generateVintage(): ByteArray = generateWithTransform { r, g, b ->
        val luma = 0.299f * r + 0.587f * g + 0.114f * b
        val desatR = r * 0.7f + luma * 0.3f
        val desatG = g * 0.7f + luma * 0.3f
        val desatB = b * 0.7f + luma * 0.3f
        Triple(
            desatR * 1.05f + 0.04f,
            desatG * 0.97f + 0.02f,
            desatB * 0.82f + 0.01f
        )
    }
}
