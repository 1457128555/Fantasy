#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <array>

namespace fantasy {
namespace rhi {

using UniformValue = std::variant<
    int,
    float,
    std::array<float, 2>,
    std::array<float, 3>,
    std::array<float, 4>
>;

class RHIUniformSet {
public:
    void setInt(const std::string& name, int value) {
        m_uniforms[name] = value;
    }

    void setFloat(const std::string& name, float value) {
        m_uniforms[name] = value;
    }

    void setFloat2(const std::string& name, float x, float y) {
        m_uniforms[name] = std::array<float, 2>{x, y};
    }

    void setFloat3(const std::string& name, float x, float y, float z) {
        m_uniforms[name] = std::array<float, 3>{x, y, z};
    }

    void setFloat4(const std::string& name, float x, float y, float z, float w) {
        m_uniforms[name] = std::array<float, 4>{x, y, z, w};
    }

    const std::unordered_map<std::string, UniformValue>& getAll() const {
        return m_uniforms;
    }

    void clear() { m_uniforms.clear(); }

private:
    std::unordered_map<std::string, UniformValue> m_uniforms;
};

} // namespace rhi
} // namespace fantasy
