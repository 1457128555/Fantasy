#pragma once

#include "rhi/RHITexture.h"
#include <memory>

namespace fantasy {
namespace rhi {

class RHIFramebuffer {
public:
    virtual ~RHIFramebuffer() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual std::shared_ptr<RHITexture> getColorAttachment() const = 0;

    static std::shared_ptr<RHIFramebuffer> create(int width, int height, TextureFormat format = TextureFormat::RGBA8);
};

} // namespace rhi
} // namespace fantasy
