package com.fan.renderengine

object RenderEngineBridge {
    init {
        System.loadLibrary("renderengine")
    }

    external fun nativeHello(): String
}
