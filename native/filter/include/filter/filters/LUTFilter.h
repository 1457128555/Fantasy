#pragma once

#include "filter/Filter.h"
#include "rhi/RHITexture.h"
#include <memory>

namespace fantasy {
namespace filter {

class LUTFilter : public Filter {
public:
    void setLUT(std::shared_ptr<rhi::RHITexture> lutTexture);
    void setStrength(float strength);

    std::shared_ptr<rhi::RHITexture> apply(
        std::shared_ptr<rhi::RHITexture> input,
        std::shared_ptr<rhi::RHIRenderer> renderer) override;

protected:
    std::string getShaderName() const override;
    void registerShader() const override;

private:
    std::shared_ptr<rhi::RHITexture> m_lutTexture;
};

} // namespace filter
} // namespace fantasy
