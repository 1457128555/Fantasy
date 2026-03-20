package com.fantasy.ui.components

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

data class FilterSliderConfig(
    val label: String,
    val value: Float,
    val onValueChange: (Float) -> Unit,
    val enabled: Boolean = true,
    val valueRange: ClosedFloatingPointRange<Float> = -1f..1f
)

@Composable
fun FilterPanel(
    sliders: List<FilterSliderConfig>,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 8.dp)
    ) {
        sliders.forEach { config ->
            FilterSliderRow(config)
        }
    }
}

@Composable
private fun FilterSliderRow(config: FilterSliderConfig) {
    val alpha = if (config.enabled) 1f else 0.4f
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = config.label,
            modifier = Modifier.width(72.dp),
            color = MaterialTheme.colorScheme.onSurface.copy(alpha = alpha),
            style = MaterialTheme.typography.bodyMedium
        )
        Slider(
            value = config.value,
            onValueChange = config.onValueChange,
            valueRange = config.valueRange,
            enabled = config.enabled,
            modifier = Modifier.weight(1f)
        )
        Text(
            text = String.format("%.1f", config.value),
            modifier = Modifier.width(40.dp),
            color = MaterialTheme.colorScheme.onSurface.copy(alpha = alpha),
            style = MaterialTheme.typography.bodySmall
        )
    }
}
