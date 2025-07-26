#pragma once

namespace CS
{

class Color
{
public:
    Color(float red, float green, float blue, float alpha);
    ~Color() = default;

    float Red() const { return m_data[0]; }
    float Green() const { return m_data[1]; }
    float Blue() const { return m_data[2]; }
    float Alpha() const { return m_data[3]; }

private:
    union {
        float m_data[4];
        struct
        {
            float r, g, b, a;
        };
    };
};

} // namespace CS
