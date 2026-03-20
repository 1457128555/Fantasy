package com.fantasy

import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Modifier
import androidx.lifecycle.viewmodel.compose.viewModel
import com.fantasy.ui.EditorScreen
import com.fantasy.ui.theme.FantasyTheme
import com.fantasy.viewmodel.EditorViewModel

class MainActivity : ComponentActivity() {

    private val pickMedia = registerForActivityResult(
        ActivityResultContracts.PickVisualMedia()
    ) { uri ->
        uri?.let { editorViewModel.loadImage(it, this) }
    }

    private lateinit var editorViewModel: EditorViewModel

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        setContent {
            FantasyTheme {
                editorViewModel = viewModel()

                LaunchedEffect(Unit) {
                    editorViewModel.saveSuccessEvent.collect { msg ->
                        Toast.makeText(this@MainActivity, msg, Toast.LENGTH_SHORT).show()
                    }
                }
                LaunchedEffect(Unit) {
                    editorViewModel.errorEvent.collect { msg ->
                        Toast.makeText(this@MainActivity, msg, Toast.LENGTH_SHORT).show()
                    }
                }

                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    EditorScreen(
                        viewModel = editorViewModel,
                        onBuiltinClick = { editorViewModel.loadBuiltinImage(this@MainActivity) },
                        onPickFromAlbum = {
                            pickMedia.launch(
                                PickVisualMediaRequest(ActivityResultContracts.PickVisualMedia.ImageOnly)
                            )
                        },
                        onSaveClick = { editorViewModel.exportImage(this@MainActivity) },
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
    }
}
