#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace fantasy {
namespace rhi {

enum class VertexAttribType {
    Float,
    Float2,
    Float3,
    Float4,
};

struct VertexAttrib {
    std::string name;
    VertexAttribType type;
    int location;
    size_t offset;

    static int sizeOf(VertexAttribType type) {
        switch (type) {
            case VertexAttribType::Float:  return 4;
            case VertexAttribType::Float2: return 8;
            case VertexAttribType::Float3: return 12;
            case VertexAttribType::Float4: return 16;
            default: return 0;
        }
    }

    static int componentCount(VertexAttribType type) {
        switch (type) {
            case VertexAttribType::Float:  return 1;
            case VertexAttribType::Float2: return 2;
            case VertexAttribType::Float3: return 3;
            case VertexAttribType::Float4: return 4;
            default: return 0;
        }
    }
};

class RHIVertexLayout {
public:
    RHIVertexLayout& add(const std::string& name, VertexAttribType type, int location) {
        for (const auto& attr : m_attribs) {
            if (attr.location == location) {
                throw std::runtime_error("Duplicate vertex attribute location: " + std::to_string(location));
            }
        }
        size_t offset = m_stride;
        m_attribs.push_back({name, type, location, offset});
        m_stride += VertexAttrib::sizeOf(type);
        return *this;
    }

    const std::vector<VertexAttrib>& getAttribs() const { return m_attribs; }
    size_t getStride() const { return m_stride; }

private:
    std::vector<VertexAttrib> m_attribs;
    size_t m_stride = 0;
};

} // namespace rhi
} // namespace fantasy
