#pragma once
#include "base/TypeTraits.h"
#include <any>
#include <string>

namespace CSEditor
{

class TreeNode
{
public:
    TreeNode() = default;
    virtual ~TreeNode() = default;

    virtual TreeNode* GetParent() const = 0;
    virtual uint32_t GetChildrenCount() const = 0;
    virtual TreeNode* GetChild(uint32_t index) const = 0;
    virtual bool IsValid() const = 0;

    virtual std::any GetProperty(std::string_view property) const = 0;

    virtual std::any GetUnderlyingNode() const = 0;
};

} // namespace CSEditor
