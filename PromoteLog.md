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
· 工具栏（折叠/展开，搜索）在同一行，title单独一行

优化 CSSceneHierarchyView
· 增加SelectionModel，关联起Hierarchy 和 Inspector
· 点击空白处后取消选择
· 增加工具栏（折叠/展开，搜索）在同一行，title单独一行

MVVM
· 梳理model和view之间的通信关系，修正错误
· 增加model之间的通知关系


完善ComponentView
· float 或 float vector，左右拖拽可以调整值的大小
· 使 ComponentView 的编辑真正生效


日志系统 ✓
· 日志库：纯 C++ 标准库实现（C++20 std::format），无第三方依赖，header-only 零侵入
· log输出可设置为文件或者控制台，运行期可配置 ✓  (Logger::SetSink / Logger::Sink::Console|File|Both)
· 在engine/base目录下，实现CS::Logger::Log的封装隐藏底层细节 ✓  (Logger.h / Logger.cpp)
· 内容定义，支持format风格 ✓  (CS_LOG_ERROR(channel, "{}", value) 等宏)
· 使用ExtensibleFlagEnum定义日志分级 ✓  (LogLevels: trace/debug/info/warning/error/fatal)
· 支持日志分级运行期可配置 ✓  (Logger::SetMinLevel)
· 配置文件唯一 ✓  (Logger::Initialize 单点调用)
· 输出内容应包含（时间，分级，pid，tid，模块，函数，内容）✓
· 识别现有代码中（如——// TODO: log error），添加log ✓  (全部 TODO: log 注释已替换)