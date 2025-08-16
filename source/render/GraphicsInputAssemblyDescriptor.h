#pragma once
#include "IGraphicsResourceDescriptor.h"
#include "VertexInputLayout.h"
#include <any>

namespace CS
{
class GraphicsBufferDescriptor;

class GraphicsInputAssemblyDescriptor : public IGraphicsResourceDescriptor
{
public:
    struct VertexInput
    {
        GraphicsBufferDescriptor* vertexBuffer{nullptr};
        uint32_t offset{0u};
    };
    GraphicsInputAssemblyDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);
    ~GraphicsInputAssemblyDescriptor() = default;

    virtual bool IsBuild() const override;
    virtual void Destroy() override;
    virtual bool IsDirty() const override;

    void SetVertexInputLayout(std::unique_ptr<VertexInputLayout> vertexInputLayout);
    const VertexInputLayout& GetVertexInputLayout() const { return *m_vertexInputLayout.get(); }

    void SetVertexInput(uint32_t binding, uint32_t bufferResourceID, uint32_t offset);
    const VertexInput& GetVertexInput(uint32_t binding) const;

    void SetIndexBuffer(uint32_t bufferResourceID, IndexFormat format);
    const GraphicsBufferDescriptor* GetIndexBuffer() const { return m_indexBuffer; }
    IndexFormat GetIndexFormat() const { return m_format; }

    template <typename AssemblyDataType>
    void SetAssemblyInputData(AssemblyDataType&& assembe)
    {
        m_assemblyData = std::forward<AssemblyDataType>(assembe);
    }

    template <typename AssemblyDataType>
    AssemblyDataType GetAssemblyInputData() const
    {
        return std::any_cast<AssemblyDataType>(m_assemblyData);
    }

protected:
    virtual bool build() override;

private:
    std::unique_ptr<VertexInputLayout> m_vertexInputLayout;
    std::array<VertexInput, VertexInputLayout::MaxBindings + 1u> m_vertexInputs;
    GraphicsBufferDescriptor* m_indexBuffer{nullptr};
    IndexFormat m_format{IndexFormat::UInt16};
    std::any m_assemblyData;
};

} // namespace CS
