#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace fantasy {
namespace rhi {

class RHIShader {
public:
    virtual ~RHIShader() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual int getUniformLocation(const std::string& name) const = 0;

    // 注册内置 shader
    static void registerBuiltins();

    // 清除所有已注册的 shader（切换 GL 上下文时需要）
    static void clearRegistry();

    // 按名字获取 shader
    static std::shared_ptr<RHIShader> get(const std::string& name);

    // 注册自定义 shader（滤镜层使用）
    static void registerShader(const std::string& name,
                               const std::string& vertexSrc,
                               const std::string& fragmentSrc);

private:
    static std::unordered_map<std::string, std::shared_ptr<RHIShader>> s_shaderRegistry;
};

} // namespace rhi
} // namespace fantasy
