package com.fan.engine

import android.content.Context
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.graphics.BitmapFactory 

class RenderSurfaceView(context: Context) : SurfaceView(context), SurfaceHolder.Callback {
    init {
        holder.addCallback(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        EngineBridge.nativeSurfaceCreated(holder.surface)

        val bmp = BitmapFactory.decodeResource(resources, R.drawable.test)
        EngineBridge.nativeSetImage(bmp)   
        bmp.recycle()                       
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        EngineBridge.nativeSurfaceChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        EngineBridge.nativeSurfaceDestroyed()
    }

    companion object {
        private const val TAG = "RenderSurfaceView"
    }
}