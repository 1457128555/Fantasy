#pragma once

#include <memory>
#include <cstdint>

namespace fantasy {
namespace rhi {

enum class TextureFormat {
    RGBA8,
    RGB8,
};

class RHITexture {
public:
    virtual ~RHITexture() = default;

    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual TextureFormat getFormat() const = 0;

    virtual void upload(const uint8_t* data, int width, int height, TextureFormat format) = 0;
    virtual void readPixels(uint8_t* outData) const = 0;
    virtual void bind(int unit = 0) const = 0;
    virtual void unbind() const = 0;

    static std::shared_ptr<RHITexture> create(int width, int height, TextureFormat format);
    static std::shared_ptr<RHITexture> create(const uint8_t* data, int width, int height, TextureFormat format);
};

} // namespace rhi
} // namespace fantasy
