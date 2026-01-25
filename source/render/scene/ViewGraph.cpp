#include "ViewGraph.h"
#include "View.h"
#include "base/Graph.h"

namespace CS
{

template <>
struct ImplData<ViewGraph>
{
    ImplData() {}

    void sort();

    DAG<std::shared_ptr<View>, GenerationID<ViewID>> viewGraph;
    std::shared_ptr<View> mainView;
    std::vector<std::shared_ptr<View>> views;
};

ViewGraph::ViewGraph() : PImpl<ViewGraph>()
{
    impl().mainView = CreateView();
}

std::shared_ptr<View> ViewGraph::GetMainView() const
{
    return impl().mainView;
}

void ViewGraph::Recompute()
{
    impl().sort();
}

std::shared_ptr<View> ViewGraph::CreateView()
{
    auto id = impl().viewGraph.AddVertex();
    auto view = View::create((ViewID)id);
    impl().viewGraph.SetVertexData(id, view);
    return view;
}

std::shared_ptr<View> ViewGraph::CreateView(RenderTarget target)
{
    auto id = impl().viewGraph.AddVertex();
    auto view = View::create((ViewID)id, target);
    impl().viewGraph.SetVertexData(id, view);
    return view;
}

void ImplData<ViewGraph>::sort()
{
    auto vertices = viewGraph.GetTopologicallySortedVertices();
    views.clear();
    for (const auto& vertexID : vertices) {
        views.push_back(viewGraph.GetVertex(vertexID).data);
    }
}

} // namespace CS
