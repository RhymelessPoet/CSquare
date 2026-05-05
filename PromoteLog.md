基于C++23 和 qt6， 在editor模块中，实现sceneobject的inspector，包含
· CSInspectorView.qml
· CSComponentView.qml
· SceneObjectModel.cpp/.h
· CSComponentModel.h/.cpp
. CSInspectorView.qml 根据 SceneObjectModel.cpp/.h 自动添加 CSComponentView.qml
· SceneObjectModel.cpp/.h 根据SceneObject包含的components，创建 CSComponentModel.h/.cpp
· CSComponentModel.h/.cpp 根据不同component的property，创建可被CSComponentView.qml编辑的item
· CSComponentView.qml 与 CSComponentModel.h/.cpp 关联，根据CSComponentModel.h/.cpp 从component中识别到的property组合出编辑界面

补全CSSceneHierarchyView.qml，支持选中和取消选中
· 可以将选中的SceneObject传递给SceneObjectModel.cpp/.h