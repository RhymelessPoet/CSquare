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


优化SceneObject Inspector
· Component显示的name，去掉命名空间
· 解决ComponentView 不能编辑的问题
. CSInspectorView.qml 增加title和工具栏（折叠/展开，搜索）
. 修改Transform，将rotation属性改为欧拉角
· 去掉ComponentView 和 ComponentModel 对matrix的支持
· 实现 CSInspectorView 可以从侧边展开和收起 

优化 CSSceneHierarchyView
· 增加SelectionModel，关联起Hierarchy 和 Inspector
· 点击空白处后取消选择

MVVM
· 梳理model和view之间的通信关系，修正错误
· 增加model之间的通知关系