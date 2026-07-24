#pragma once
#include "TypeTraits.h"
#include <any>
#include <string>

namespace CSEditor
{

class TreeNode
{
public:
    TreeNode() = default;
    virtual ~TreeNode() = default;

    virtual const TreeNode* GetParent() const = 0;
    virtual uint32_t GetChildCount() const = 0;
    virtual const TreeNode* GetChild(uint32_t index) const = 0;
    virtual bool IsValid() const = 0;
    virtual std::any GetProperty(std::string_view property) const = 0;
    virtual bool SetProperty(std::string_view property, const std::any& value) = 0;
    virtual std::any GetUnderlyingNode() const = 0;
    virtual std::vector<std::string_view> GetPropertyNames() const = 0;
};

static int32_t IndexOfChildInParent(const TreeNode* child, const TreeNode* parent)
{
    if (parent == nullptr) {
        return -1;
    }

    for (int32_t index = 0; index < parent->GetChildCount(); ++index) {
        if (parent->GetChild(index) == child) {
            return index;
        }
    }
    return -1;
}

static int32_t IndexOfChildInParent(const TreeNode* node)
{
    if (node == nullptr) {
        return -1;
    }
    return IndexOfChildInParent(node, node->GetParent());
}

} // namespace CSEditor
