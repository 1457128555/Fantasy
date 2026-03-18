#include "filter/FilterChain.h"
#include "rhi/Log.h"

#define TAG "FilterChain"

namespace fantasy {
namespace filter {

void FilterChain::addFilter(std::shared_ptr<Filter> filter) {
    m_filters.push_back(std::move(filter));
}

void FilterChain::clearFilters() {
    m_filters.clear();
}

int FilterChain::getFilterCount() const {
    return static_cast<int>(m_filters.size());
}

std::shared_ptr<rhi::RHITexture> FilterChain::apply(
    std::shared_ptr<rhi::RHITexture> input,
    std::shared_ptr<rhi::RHIRenderer> renderer) {

    auto current = input;
    for (auto& filter : m_filters) {
        current = filter->apply(current, renderer);
    }
    return current;
}

} // namespace filter
} // namespace fantasy
