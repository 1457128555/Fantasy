#pragma once

#include "filter/Filter.h"

namespace fantasy {
namespace filter {

class SaturationFilter : public Filter {
public:
    // saturation: -1.0 ~ 1.0, default 0.0
    void setSaturation(float saturation);

protected:
    std::string getShaderName() const override;
    void registerShader() const override;
};

} // namespace filter
} // namespace fantasy
