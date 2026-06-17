package com.fan.engine

import android.view.Surface
import android.graphics.Bitmap 

object EngineBridge {
    init {
        System.loadLibrary("engine")
    }

    external fun initialize() : Boolean

    external fun destroy()

    external fun onSurfaceCreated(surface: Surface) : Boolean

    external fun onSurfaceChanged(width: Int, height: Int)

    external fun onSurfaceDestroyed()

    external fun setImage(bitmap: Bitmap)
}
