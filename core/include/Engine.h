#pragma once
#include "Common/Singleton.h"
#include <vector>

namespace Fantasy
{
    class Engine : public Common::Singleton<Engine>
    {
    public:
        Engine();
        
        ~Engine();
        
        bool initialize();
        
        void destroy();

        void renderOneFrame(float dt);

        bool onSurfaceCreated(void* win);

        void onSurfaceChanged(int w, int h);

        void* onSurfaceDestroyed();

        void setImage(void* data, int w, int h);

        int getImgW()const {return mImgW;};
        int getImgH()const {return mImgH;};
        const std::vector<uint8_t>& getImage() const { return mImage; };

        int getWidth()  const { return mWidth; }
        int getHeight() const { return mHeight; }
    private:
        int mWidth = 0;
        int mHeight = 0;
        void* mWinHandle = nullptr;

        int mImgW = 0;
        int mImgH = 0;
        std::vector<uint8_t> mImage;
    };
} // namespace Fantasy
