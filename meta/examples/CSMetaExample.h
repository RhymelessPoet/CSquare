#pragma once
#include "MetaDefine.h"

namespace CXMBaseExample
{
    class [[Meta()]] CXMetaBaseExample
    {

    };
} // namespace 


namespace CSMExample
{
class CSXXXX;

class [[Meta(QML)]] CSMetaExample : public CXMBaseExample::CXMetaBaseExample
{
public:
    CSMetaExample(/* args */);
    ~CSMetaExample();

    [[Method()]]
    void setBool(bool value) { m_bool = value; }

private:
    [[Property(Name=(999, (9,0)), Setter = setBool, Getter, SerializeName =Boolean)]]
    bool m_bool{true};

    int m_int{0};
    float m_float{0.0f};
    void* m_ptr{nullptr};
};
}