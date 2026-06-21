package com.fan.fantasy

import android.util.Log
import androidx.lifecycle.ViewModel
import com.fan.engine.EngineBridge

class EditorViewModel : ViewModel() {
    init {
        Log.d("EditorVM", "init → nativeInit")
        EngineBridge.initialize()
    }

    override fun onCleared() {
        Log.d("EditorVM", "onCleared → nativeDestroy")
        EngineBridge.destroy()        
    }
}
