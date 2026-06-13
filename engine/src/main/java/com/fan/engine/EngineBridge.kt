package com.fan.engine

object EngineBridge {
    init {
        System.loadLibrary("engine")
    }

    external fun nativeHello(): String
}
