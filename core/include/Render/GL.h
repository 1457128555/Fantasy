#pragma once

#if defined(__ANDROID__)
    #include <GLES3/gl3.h>
#elif defined(__APPLE__)
    #include <OpenGLES/ES3/gl.h>   
#endif