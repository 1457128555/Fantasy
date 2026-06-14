package com.fan.engine

import android.view.Surface
import android.graphics.Bitmap 

object EngineBridge {
    init {
        System.loadLibrary("engine")
        nativeInit();
    }

    external fun nativeHello(): String

    external fun nativeInit()   

    external fun nativeSurfaceCreated(surface: Surface)

    external fun nativeSurfaceChanged(width: Int, height: Int)

    external fun nativeSurfaceDestroyed()

    external fun nativeSetImage(bitmap: Bitmap) 
}
