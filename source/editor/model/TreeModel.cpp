#include "TreeModel.h"

namespace CSEditor
{
TreeNode* TreeModel::CreateTreeNode(TreeNode* parent)
{
    return m_impl->CreateTreeNode(parent);
}
const TreeNode* TreeModel::GetRoot() const
{
    return m_impl->GetRoot();
}
TreeNode* TreeModel::GetRoot()
{
    return m_impl->GetRoot();
}
} // namespace CSEditor
