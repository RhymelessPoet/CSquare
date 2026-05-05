#include "QVariantsAny.h"
#include <QJSValue>
#include <QString>
#include <QVariantList>
#include <array>

namespace CSEditor
{
QVariant AnyToQVariant(const std::any& anyVal)
{
    QVariant var;

    if (anyVal.type() == typeid(int)) {
        var = std::any_cast<int>(anyVal);
    } else if (anyVal.type() == typeid(float)) {
        var = static_cast<double>(std::any_cast<float>(anyVal));
    } else if (anyVal.type() == typeid(double)) {
        var = std::any_cast<double>(anyVal);
    } else if (anyVal.type() == typeid(std::string)) {
        var = QString::fromStdString(std::any_cast<std::string>(anyVal));
    } else if (anyVal.type() == typeid(bool)) {
        var = std::any_cast<bool>(anyVal);
    } else if (anyVal.type() == typeid(std::array<float, 3>)) {
        const auto& arr = std::any_cast<const std::array<float, 3>&>(anyVal);
        QVariantList list;
        list.reserve(3);
        list.append(static_cast<double>(arr[0]));
        list.append(static_cast<double>(arr[1]));
        list.append(static_cast<double>(arr[2]));
        var = list;
    } else {
        // TODO: log error
    }

    return var;
}

std::any QVariantToAny(const QVariant& variant)
{
    // QML passes JS values (numbers, arrays, objects) wrapped in QJSValue.
    // Unwrap them into a native QVariant so our switch below matches.
    if (variant.canConvert<QJSValue>() && variant.userType() == qMetaTypeId<QJSValue>()) {
        const QJSValue jsVal = variant.value<QJSValue>();
        return QVariantToAny(jsVal.toVariant());
    }

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
    case QVariant::List: {
        const QVariantList list = variant.toList();
        if (list.size() == 3) {
            std::array<float, 3> arr{};
            for (int i = 0; i < 3; ++i) {
                arr[i] = static_cast<float>(list[i].toDouble());
            }
            anyVal = arr;
        }
        break;
    }
    default: {
        // TODO: log error
    }
    }

    return anyVal;
}

} // namespace CSEditor
