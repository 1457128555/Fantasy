#pragma once

#include "rhi/RHIUniformSet.h"
#include <string>

namespace fantasy {
namespace filter {

class FilterParam {
public:
    void setFloat(const std::string& key, float value) {
        m_uniforms.setFloat(key, value);
    }

    void setInt(const std::string& key, int value) {
        m_uniforms.setInt(key, value);
    }

    void setFloat2(const std::string& key, float x, float y) {
        m_uniforms.setFloat2(key, x, y);
    }

    void setFloat3(const std::string& key, float x, float y, float z) {
        m_uniforms.setFloat3(key, x, y, z);
    }

    void setFloat4(const std::string& key, float x, float y, float z, float w) {
        m_uniforms.setFloat4(key, x, y, z, w);
    }

    const rhi::RHIUniformSet& toUniformSet() const { return m_uniforms; }

    void clear() { m_uniforms.clear(); }

private:
    rhi::RHIUniformSet m_uniforms;
};

} // namespace filter
} // namespace fantasy
