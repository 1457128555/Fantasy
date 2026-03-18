#pragma once

#include "filter/Filter.h"
#include "rhi/RHITexture.h"
#include "rhi/RHIRenderer.h"
#include <vector>
#include <memory>

namespace fantasy {
namespace filter {

class FilterChain {
public:
    void addFilter(std::shared_ptr<Filter> filter);
    void clearFilters();
    int getFilterCount() const;

    // 依次执行所有滤镜，返回最终输出纹理
    // 如果链为空，返回原始输入
    std::shared_ptr<rhi::RHITexture> apply(
        std::shared_ptr<rhi::RHITexture> input,
        std::shared_ptr<rhi::RHIRenderer> renderer);

private:
    std::vector<std::shared_ptr<Filter>> m_filters;
};

} // namespace filter
} // namespace fantasy
