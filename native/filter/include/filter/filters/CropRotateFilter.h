#pragma once

#include "filter/Filter.h"

namespace fantasy {
namespace filter {

class CropRotateFilter : public Filter {
public:
    // crop rect: normalized [0,1], default (0,0,1,1) = full image
    void setCropRect(float x, float y, float w, float h);

    // 90-degree rotation: 0=0°, 1=90°CW, 2=180°, 3=270°CW
    void setRotation90(int steps);

    // free rotation in radians, range [-pi/4, pi/4]
    void setFreeRotation(float radians);

    std::shared_ptr<rhi::RHITexture> apply(
        std::shared_ptr<rhi::RHITexture> input,
        std::shared_ptr<rhi::RHIRenderer> renderer) override;

protected:
    std::string getShaderName() const override;
    void registerShader() const override;

private:
    float m_cropX = 0.0f;
    float m_cropY = 0.0f;
    float m_cropW = 1.0f;
    float m_cropH = 1.0f;
    int m_rotation90 = 0;
    float m_freeRotation = 0.0f;
};

} // namespace filter
} // namespace fantasy
