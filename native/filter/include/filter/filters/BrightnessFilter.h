#pragma once

#include "filter/Filter.h"

namespace fantasy {
namespace filter {

class BrightnessFilter : public Filter {
public:
    // brightness: -1.0 ~ 1.0, default 0.0
    void setBrightness(float brightness);

protected:
    std::string getShaderName() const override;
    void registerShader() const override;
};

} // namespace filter
} // namespace fantasy
