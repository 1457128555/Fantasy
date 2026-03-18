#pragma once

#include "rhi/RHIVertexLayout.h"
#include <memory>
#include <cstddef>

namespace fantasy {
namespace rhi {

class RHIVertexBuffer {
public:
    virtual ~RHIVertexBuffer() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual const RHIVertexLayout& getLayout() const = 0;

    static std::shared_ptr<RHIVertexBuffer> create(const float* data, size_t size, const RHIVertexLayout& layout);
};

} // namespace rhi
} // namespace fantasy
