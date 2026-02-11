#pragma once
#include <QVariant>
#include <any>

namespace CSEditor
{

QVariant AnyToQVariant(const std::any& anyVal);
std::any QVariantToAny(const QVariant& variant);

} // namespace CSEditor
