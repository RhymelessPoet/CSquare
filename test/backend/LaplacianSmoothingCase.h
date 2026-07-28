#pragma once
#include <QImage>
#include <memory>

namespace CS
{
class Renderer;
class GraphicsResourceCache;
} // namespace CS

class LaplacianSmoothingCase
{
public:
    LaplacianSmoothingCase();
    ~LaplacianSmoothingCase();
    QImage run(QString& error);

private:
    bool initialize(QString& error);
    std::shared_ptr<CS::GraphicsResourceCache> m_cache;
    std::unique_ptr<CS::Renderer> m_renderer;
};
