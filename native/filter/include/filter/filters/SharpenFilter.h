#pragma once

#include "filter/Filter.h"

namespace fantasy {
namespace filter {

class SharpenFilter : public Filter {
public:
    // sharpness: 0.0 ~ 1.0, default 0.0 (no sharpening)
    void setSharpness(float sharpness);

    std::shared_ptr<rhi::RHITexture> apply(
        std::shared_ptr<rhi::RHITexture> input,
        std::shared_ptr<rhi::RHIRenderer> renderer) override;

protected:
    std::string getShaderName() const override;
    void registerShader() const override;
};

} // namespace filter
} // namespace fantasy
