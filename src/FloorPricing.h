#ifndef AH_BOT_FLOOR_PRICING_H
#define AH_BOT_FLOOR_PRICING_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

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
}
#endif
