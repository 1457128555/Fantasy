#pragma once

#include "filter/FilterParam.h"
#include "rhi/RHITexture.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIRenderer.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include <memory>
#include <string>

namespace fantasy {
namespace filter {

class Filter {
public:
    virtual ~Filter() = default;

    // 设置滤镜参数
    void setParam(const std::string& key, float value);

    // 获取参数表
    const FilterParam& getParam() const { return m_param; }

    // 应用滤镜：输入纹理 → 输出纹理
    // renderer 由调用方（FilterChain）提供，保证复用
    std::shared_ptr<rhi::RHITexture> apply(
        std::shared_ptr<rhi::RHITexture> input,
        std::shared_ptr<rhi::RHIRenderer> renderer);

protected:
    // 子类实现：返回使用的 shader 名字
    virtual std::string getShaderName() const = 0;

    // 子类实现：注册自己的 shader（仅首次调用时执行）
    virtual void registerShader() const = 0;

    // 子类实现：把 m_param 转为渲染用的 uniform
    // 默认实现直接返回 m_param.toUniformSet()，子类可覆盖
    virtual rhi::RHIUniformSet buildUniforms() const;

    FilterParam m_param;
};

} // namespace filter
} // namespace fantasy
