#include "UUID.h"
#include <array>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

static std::string GenerateUUID_V4()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis8(0, 3);

    std::array<char, 36> uuid{'-'};

    for (int i = 0; i < 36; ++i) {
        switch (i) {
        case 8:  // 第 8 位后加 '-'
        case 13: // 第 13 位后加 '-'
        case 18: // 第 18 位后加 '-'
        case 23: // 第 23 位后加 '-'
        case 14: // 第 14 位固定为 '4'（版本 v4）
        case 19: // 第 19 位为 8/9/A/B（变体）
            break;
        default: // 其余位为随机 16 进制数
            uuid[i] = "0123456789abcdef"[dis(gen)];
            break;
        }
    }
    uuid[8] = uuid[13] = uuid[18] = uuid[23] = '-';
    uuid[14] = '4';
    uuid[19] = "89AB"[dis8(gen)];

    return std::string(uuid.begin(), uuid.end());
}

namespace CS
{
UUID::UUID() {}

void UUID::Regenerate()
{
    m_data = GenerateUUID_V4();
}

} // namespace CS
