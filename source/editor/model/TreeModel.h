#pragma once
#include "TreeNode.h"
#include <memory>

namespace CSEditor
{

template <typename TreeType>
struct tree_node_type
{
    using Type = TreeType::NodeType;
};
template <typename TreeType>
using tree_node_type_t = tree_node_type<TreeType>::Type;

class TreeModel
{
public:
    template <typename TreeType, typename... ExtendedNodeProperties>
    TreeModel(TreeType&& tree, ExtendedNodeProperties&&... properties)
    {}

    TreeNode* CreateTreeNode(TreeNode* parent);
    const TreeNode* GetRoot() const;
    TreeNode* GetRoot();

    class TreeModelConcept
    {
    public:
        virtual ~TreeModelConcept() = default;
        virtual TreeNode* CreateTreeNode(TreeNode* parent) = 0;
        virtual const TreeNode* GetRoot() const = 0;
        virtual TreeNode* GetRoot() = 0;
    };

    template <typename TreeType, typename ExtendStrategy, typename... ExtendedNodeProperties>
    class TreeModelConceptImpl
    {
    public:
        using UnderlyingNodeType = tree_node_type_t<TreeType>;
        using ExtendedPropertiesTuple = std::tuple<ExtendedNodeProperties...>;

        class TreeNodeExternal final : public TreeNode
        {
        public:
            TreeNodeExternal(ExtendStrategy strategy, UnderlyingNodeType&& node)
                : m_strategy(strategy), m_node(std::forward<UnderlyingNodeType>(node))
            {
                strategy.Initialize(m_extendProperties);
                m_strategy.Insert(m_node, this);
            }

            virtual TreeNode* GetParent() const override { return m_strategy.Get(GetParent(m_node)); }
            virtual uint32_t GetChildrenCount() const override { return GetChildrenCount(m_node); }
            virtual TreeNode* GetChild(uint32_t index) const override
            {
                return m_strategy.Get(GetChild(m_node, index));
            }
            virtual bool IsValid() const override { return IsValid(m_node) && m_strategy.Get(m_node) != nullptr; }

            virtual std::any GetProperty(std::string_view property) const override
            {
                return m_strategy.GetProperty(m_node, m_extendProperties, property);
            }

            virtual std::any GetUnderlyingNode() const override { return m_node; }

        private:
            [[no_unique_address]] ExtendStrategy m_strategy;
            UnderlyingNodeType m_node;
            ExtendedPropertiesTuple m_extendProperties;
        };

        TreeModelConceptImpl(TreeType&& tree) : m_underlyingTree(std::move(tree)) {}

        virtual TreeNode* CreateTreeNode(TreeNode* parent) override
        {
            if (parent == nullptr) {
                return nullptr;
            }
            auto parentNodeAny = parent->GetUnderlyingNode();
            auto parentNode = std::any_cast<UnderlyingNodeType>(parentNodeAny);
            UnderlyingNodeType node = CreateNode(m_underlyingTree, parentNode);
            auto newNode = new TreeNodeExternal(m_strategy, node);
            return newNode;
        }
        virtual const TreeNode* GetRoot() const override
        {
            auto rootNode = GetRoot(m_underlyingTree);
            return reinterpret_cast<TreeNode*>(m_strategy.Get(rootNode));
        }
        virtual TreeNode* GetRoot() override
        {
            auto rootNode = GetRoot(m_underlyingTree);
            return reinterpret_cast<TreeNode*>(m_strategy.Get(rootNode));
        }

    private:
        TreeNodeExternal* getParent(const UnderlyingNodeType& node) { m_strategy.GetParent(node); }

    private:
        [[no_unique_address]] ExtendStrategy m_strategy;
        TreeType m_underlyingTree;
    };

private:
    std::unique_ptr<TreeModelConcept> m_impl;
};

} // namespace CSEditor
