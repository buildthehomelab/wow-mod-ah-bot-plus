#ifndef AH_BOT_FLOOR_PRICING_H
#define AH_BOT_FLOOR_PRICING_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace AhBotPricing
{
// Raise a quote to Floor (copper, clamped to Cap). Never lowers it.
// The bid keeps its usual discount below the buyout.
inline void RaiseToFloor(double floor, std::uint64_t cap, std::uint64_t& bid, std::uint64_t& buyout)
{
    if (!std::isfinite(floor) || floor <= 0)
        return;
    floor = std::min(std::ceil(floor), static_cast<double>(cap));
    if (floor <= static_cast<double>(buyout))
        return;
    double ratio = buyout ? std::min(1.0, static_cast<double>(bid) / buyout) : 1.0;
    buyout = static_cast<std::uint64_t>(floor);
    bid = std::max<std::uint64_t>(1, static_cast<std::uint64_t>(std::ceil(floor * ratio)));
}

// Epic gear: Base copper at ReferenceLevel, growing as (ItemLevel / ReferenceLevel) ^ Exponent.
inline double EpicGearFloor(std::uint32_t itemLevel, double base, std::uint32_t referenceLevel, double exponent)
{
    if (!itemLevel || !referenceLevel || !std::isfinite(base) || base <= 0 || !std::isfinite(exponent))
        return 0;
    return base * std::pow(double(itemLevel) / referenceLevel, exponent);
}

// Mount price tier from the skill it needs: riding 75/150/225/300 = apprentice, journeyman,
// expert, artisan. Profession-gated mounts (engineering flying machines at 300/375) sit one
// tier lower than their skill number suggests, so subtract 75 for them.
inline std::uint32_t MountTier(std::uint32_t skillRank, bool riding)
{
    if (!riding)
        skillRank = skillRank > 75 ? skillRank - 75 : 0;
    return skillRank >= 300 ? 3 : skillRank >= 225 ? 2 : skillRank >= 150 ? 1 : 0;
}
// Disenchant loot rows (loot template semantics): ungrouped rows roll independently;
// inside a group, explicit-chance rows come first and chance-0 rows share what is left.
struct DropRow { std::uint32_t item; double chance; std::uint32_t group; std::uint32_t minCount; std::uint32_t maxCount; };

// Expected units of each item per loot roll.
inline std::map<std::uint32_t, double> ExpectedDrops(std::vector<DropRow> const& rows)
{
    std::map<std::uint32_t, double> explicitTotal, zeroRows;
    for (auto const& row : rows)
        if (row.group)
        {
            if (row.chance > 0)
                explicitTotal[row.group] += row.chance;
            else
                zeroRows[row.group] += 1;
        }
    std::map<std::uint32_t, double> expected;
    for (auto const& row : rows)
    {
        double chance = row.chance / 100;
        if (row.group && row.chance <= 0)
            chance = std::max(0.0, 100 - explicitTotal[row.group]) / 100 / zeroRows[row.group];
        chance = std::clamp(chance, 0.0, 1.0);
        double count = (double(row.minCount) + std::max(row.minCount, row.maxCount)) / 2;
        if (chance > 0 && count > 0)
            expected[row.item] += chance * count;
    }
    return expected;
}

// Developer placeholder names the stock text filter misses: "Unused ...", "Durability ...",
// "Monster - ..." (NPC-only weapons), "Deprecate...", "90 Green Warrior Axe",
// "Red Leather C03 Boots", "JYoo Random Item Test".
inline bool IsPlaceholderName(std::string const& name)
{
    for (char const* prefix : {"Unused ", "Durability ", "Monster - "})
        if (name.rfind(prefix, 0) == 0)
            return true;
    for (char const* part : {"Deprecate", " C03 ", " D02 ", " D03 ", "Random Item Test"})
        if (name.find(part) != std::string::npos)
            return true;
    std::size_t digits = 0;
    while (digits < name.size() && name[digits] >= '0' && name[digits] <= '9')
        ++digits;
    return digits && name.compare(digits, 7, " Green ") == 0;
}
}
#endif
