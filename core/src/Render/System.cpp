#include "Render/System.h"
#include "Render/IGLContext.h"   // 注入后 unique_ptr<IGLContext> 在本 TU 析构，需完整类型

#include "Render/GL.h"
#include "Render/Renderer.h"
#include "Engine.h"

#include <utility>   // std::move

namespace Fantasy::Render
{
    System::System()
    {
    }

    void System::setContext(std::unique_ptr<IGLContext> ctx)
    {
        mContext = std::move(ctx);   // 平台胶水注入，System 接管所有权
    }

    System::~System()
    {

    }

    bool System::initialize() 
    {
        if(!mContext)              // 必须先 setContext 注入平台 context
            return false;
        if(!mContext->initialize())
            return false;
        if(!mThread.initialize())
            return false;
        return true;
    }

    void System::renderOneFrame(float dt)
    {
        int winW = Engine::Instance()->getWidth();
        int winH = Engine::Instance()->getHeight();
        int imgW = Engine::Instance()->getImgW();
        int imgH = Engine::Instance()->getImgH();

        auto imgPtr = std::make_shared<std::vector<uint8_t>>(Engine::Instance()->getImage());
        mThread.post([=](){
            Renderer r;
            r.initGL();
            r.setImage(imgW, imgH, imgPtr->data());
            r.render(winW, winH);
            mContext->swapBuffers();
        });
    }

    void System::destroy()
    {
        mThread.destroy();
        if (mContext)              // 未注入/初始化失败的兜底，防空指针
            mContext->destroy();
    }

    bool System::onSurfaceCreated(void* win)
    {
        if(!mContext->onSurfaceCreated(win))
            return false;

        mThread.post([&](){
            mContext->makeCurrent();
        });

        renderOneFrame(0.f);

        return true;
    }

    void System::onSurfaceChanged(int w, int h)
    {       
        mThread.post([=](){
            glViewport(0, 0, w, h);
        });
    }

    void System::onSurfaceDestroyed()
    {
        mThread.postAndWait([&](){
            mContext->onSurfaceDestroyed();
        });
    }
}