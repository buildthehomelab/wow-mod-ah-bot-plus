#ifndef AH_BOT_MATERIAL_STOCK_H
#define AH_BOT_MATERIAL_STOCK_H

#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

namespace AhBotStock
{
struct Request
{
    std::uint32_t item;
    std::uint32_t deficit;
};

// A rotating, bounded pass prevents popular or unpriceable materials starving later IDs.
inline std::vector<Request> Plan(std::vector<std::uint32_t> const& materials,
    std::map<std::uint32_t, std::uint32_t> const& available,
    std::uint32_t target, std::uint32_t budget, std::size_t& cursor)
{
    std::vector<Request> result;
    if (materials.empty() || !target || !budget)
        return result;
    cursor %= materials.size();
    std::size_t visited = 0;
    while (visited < materials.size() && result.size() < budget)
    {
        auto item = materials[cursor];
        cursor = (cursor + 1) % materials.size();
        ++visited;
        auto it = available.find(item);
        auto count = it == available.end() ? 0 : it->second;
        if (count < target)
            result.push_back({item, target - count});
    }
    return result;
}

inline void Count(std::map<std::uint32_t, std::uint32_t>& available, std::uint32_t item,
    std::uint32_t count, std::uint32_t target)
{
    auto& total = available[item];
    total += std::min(count, target - std::min(total, target));
}
}
#endif
