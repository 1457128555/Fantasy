package com.fan.engine

import android.content.Context
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView

class RenderSurfaceView(context: Context) : SurfaceView(context), SurfaceHolder.Callback {
    init {
        holder.addCallback(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        EngineBridge.onSurfaceCreated(holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        EngineBridge.onSurfaceChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        EngineBridge.onSurfaceDestroyed()
    }

    companion object {
        private const val TAG = "RenderSurfaceView"
    }
}