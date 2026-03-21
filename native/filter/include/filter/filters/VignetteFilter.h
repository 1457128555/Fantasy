#pragma once

#include "filter/Filter.h"

namespace fantasy {
namespace filter {

class VignetteFilter : public Filter {
public:
    // vignette: 0.0 ~ 1.0, default 0.0 (no vignette)
    void setVignette(float vignette);

protected:
    std::string getShaderName() const override;
    void registerShader() const override;
};

} // namespace filter
} // namespace fantasy
