#include "QVariantsAny.h"
#include <QString>

namespace CSEditor
{
QVariant AnyToQVariant(const std::any& anyVal)
{
    QVariant var;

    if (anyVal.type() == typeid(int)) {
        var = std::any_cast<int>(anyVal);
    } else if (anyVal.type() == typeid(double)) {
        var = std::any_cast<double>(anyVal);
    } else if (anyVal.type() == typeid(std::string)) {
        var = QString::fromStdString(std::any_cast<std::string>(anyVal));
    } else if (anyVal.type() == typeid(bool)) {
        var = std::any_cast<bool>(anyVal);
    } else {
        // TODO: log error
    }

    return var;
}

std::any QVariantToAny(const QVariant& variant)
{
    std::any anyVal;

    switch (variant.type()) {
    case QVariant::Int:
        anyVal = variant.toInt();
        break;
    case QVariant::Double:
        anyVal = variant.toDouble();
        break;
    case QVariant::String:
        anyVal = variant.toString().toStdString();
        break;
    case QVariant::Bool:
        anyVal = variant.toBool();
        break;
    default: {
        // TODO: log error
    }
    }

    return anyVal;
}

} // namespace CSEditor
