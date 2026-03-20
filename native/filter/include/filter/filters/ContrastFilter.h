#pragma once

#include "filter/Filter.h"

namespace fantasy {
namespace filter {

class ContrastFilter : public Filter {
public:
    // contrast: -1.0 ~ 1.0, default 0.0
    void setContrast(float contrast);

protected:
    std::string getShaderName() const override;
    void registerShader() const override;
};

} // namespace filter
} // namespace fantasy
