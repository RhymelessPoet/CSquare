#pragma once
#include "TreeNode.h"
#include <memory>

namespace CSEditor
{

template <typename TreeType, typename TreeNodeType>
struct TreeOperationStrategy
{
    static TreeNodeType GetParent(const TreeNodeType& node) { return TreeNodeType{}; }
    static TreeNodeType GetChild(const TreeNodeType& node, size_t index) { return TreeNodeType{}; }
    static size_t GetChildCount(const TreeNodeType& node) { return 0; }
    static bool IsValid(const TreeNodeType& node) { return false; }
    static TreeNodeType GetRoot(const TreeType& tree) { return TreeNodeType{}; }
    static TreeNodeType CreateNode(const TreeType& tree, const TreeNodeType& parent) { return TreeNodeType{}; }
};

template <typename TreeNodeType, typename... ExtendedProperties>
struct TreeExtendStrategy
{
    using ExtendedPropertiesTuple = std::tuple<ExtendedProperties...>;
    std::vector<std::string_view> GetPropertyNames() const { return {}; }
    void Initialize(ExtendedPropertiesTuple& props) {}
    void Insert(const TreeNodeType& node, const TreeNode* external_node) {}
    void Remove(const TreeNodeType& node) {}
    const TreeNode* Get(const TreeNodeType& node) const { return nullptr; }
    std::any
    GetProperty(const TreeNodeType& node, const ExtendedPropertiesTuple& props, std::string_view property) const
    {
        return std::any{};
    }
};

class TreeModel
{
public:
    TreeModel() = default;

    template <typename TreeType,
              typename TreeNodeType,
              typename OpStrategy = TreeOperationStrategy<TreeType, TreeNodeType>,
              typename ExtStrategy = TreeExtendStrategy<TreeNodeType>,
              typename... ExtendedProperties>
    explicit TreeModel(TreeType tree, TreeNodeType, ExtStrategy extStrategy, ExtendedProperties&&... props)
    {
        // 类型约束：确保 TreeNodeType 是可被操作策略处理的类型
        static_assert(std::is_same_v<decltype(OpStrategy::GetRoot(tree)), TreeNodeType>,
                      "TreeNodeType mismatch with TreeOperationStrategy");

        using ImplType = TreeModelImpl<TreeType, TreeNodeType, OpStrategy, ExtStrategy, ExtendedProperties...>;
        m_impl = std::make_unique<ImplType>(tree, std::move(extStrategy), std::forward<ExtendedProperties>(props)...);
    }

    bool IsValid() const { return m_impl != nullptr; }

    const TreeNode* CreateTreeNode(const TreeNode* parent) { return m_impl ? m_impl->CreateTreeNode(parent) : nullptr; }

    const TreeNode* GetRoot() const { return m_impl ? m_impl->GetRoot() : nullptr; }
    const TreeNode* GetRoot() { return m_impl ? m_impl->GetRoot() : nullptr; }

    std::vector<std::string_view> GetPropertyNames() const
    {
        return m_impl ? m_impl->GetPropertyNames() : std::vector<std::string_view>{};
    }

private:
    class TreeModelConcept
    {
    public:
        virtual ~TreeModelConcept() = default;
        virtual const TreeNode* CreateTreeNode(const TreeNode* parent) = 0;
        virtual const TreeNode* GetRoot() const = 0;
        virtual std::vector<std::string_view> GetPropertyNames() const = 0;
    };
    template <typename TreeType,
              typename TreeNodeType,
              typename OpStrategy,
              typename ExtStrategy,
              typename... ExtendedProperties>
    class TreeModelImpl : public TreeModelConcept
    {
    public:
        using ExtendedPropertiesTuple = std::tuple<ExtendedProperties...>;
        using UnderlyingNodeType = TreeNodeType;

        class TreeNodeExternal final : public TreeNode
        {
        public:
            TreeNodeExternal(ExtStrategy extStrategy, UnderlyingNodeType node)
                : m_extStrategy(std::move(extStrategy)), m_node(std::move(node))
            {
                m_extStrategy.Initialize(m_extProps);
                m_extStrategy.Insert(m_node, this);
            }

            ~TreeNodeExternal() override
            {
                m_extStrategy.Remove(m_node);
                for (size_t i = 0; i < GetChildCount(); ++i) {
                    auto child = GetChild(i);
                    if (child) {
                        delete child;
                    }
                }
            }

            const TreeNode* GetParent() const override
            {
                auto parentNode = OpStrategy::GetParent(m_node);
                auto parent = m_extStrategy.Get(parentNode);
                if (parent == nullptr) {
                    return new TreeNodeExternal(m_extStrategy, parentNode);
                }
                return parent;
            }

            uint32_t GetChildCount() const override { return static_cast<uint32_t>(OpStrategy::GetChildCount(m_node)); }

            const TreeNode* GetChild(uint32_t index) const override
            {
                if (index < GetChildCount()) {
                    auto childNode = OpStrategy::GetChild(m_node, index);
                    auto node = m_extStrategy.Get(childNode);
                    if (node == nullptr) {
                        return new TreeNodeExternal(m_extStrategy, childNode);
                    } else {
                        return node;
                    }
                }
                return nullptr;
            }

            bool IsValid() const override
            {
                return OpStrategy::IsValid(m_node) && (m_extStrategy.Get(m_node) != nullptr);
            }

            std::any GetProperty(std::string_view property) const override
            {
                return m_extStrategy.GetProperty(m_node, m_extProps, property);
            }

            std::any GetUnderlyingNode() const override { return m_node; }

            virtual std::vector<std::string_view> GetPropertyNames() const override
            {
                return m_extStrategy.GetPropertyNames();
            }

        private:
            [[no_unique_address]] ExtStrategy m_extStrategy;
            UnderlyingNodeType m_node;
            ExtendedPropertiesTuple m_extProps;
        };

        TreeModelImpl(TreeType tree, ExtStrategy extStrategy, ExtendedProperties... props)
            : m_tree(std::move(tree)), m_extStrategy(std::move(extStrategy))
        {
            auto rootNode = OpStrategy::GetRoot(m_tree);
            m_root = std::make_unique<TreeNodeExternal>(m_extStrategy, rootNode);
        }

        const TreeNode* CreateTreeNode(const TreeNode* parent) override
        {
            if (!parent)
                return nullptr;

            auto parentAny = parent->GetUnderlyingNode();
            auto parentNode = std::any_cast<const UnderlyingNodeType&>(parentAny);

            auto newNode = OpStrategy::CreateNode(m_tree, parentNode);
            auto newExternal = new TreeNodeExternal(m_extStrategy, newNode);
            return newExternal;
        }

        const TreeNode* GetRoot() const override { return m_root.get(); }

        virtual std::vector<std::string_view> GetPropertyNames() const override
        {
            return m_extStrategy.GetPropertyNames();
        }

    private:
        TreeType m_tree;
        ExtStrategy m_extStrategy;
        std::unique_ptr<TreeNodeExternal> m_root;
    };

    std::unique_ptr<TreeModelConcept> m_impl;
};
} // namespace CSEditor
