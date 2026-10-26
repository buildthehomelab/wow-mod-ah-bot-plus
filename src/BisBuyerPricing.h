#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace AhBotPricing
{
    // Integer arithmetic preserves the advertised inclusive per-unit threshold exactly.
    inline std::uint64_t BisBuyerOffer(std::uint64_t reference, std::uint32_t percent)
    {
        if (!reference || reference > UINT32_MAX || percent == 0 || percent >= 100)
            return 0;
        return reference * percent / 100;
    }

    // Reference buyer: a share of the seller reference, never above the same share of what
    // an NPC charges for the item (npcPrice 0 = no NPC sells it for gold).
    inline std::uint64_t ReferenceBuyerOffer(std::uint64_t reference, std::uint32_t percent, double npcPrice)
    {
        auto offer = BisBuyerOffer(reference, percent);
        if (npcPrice > 0)
            offer = std::min<std::uint64_t>(offer, std::uint64_t(npcPrice * percent / 100));
        return offer;
    }

    // A craftable BiS item pays its making cost (cost >= 0) with the BiS demand premium, so
    // crafting one is worth it; never above the seller quote. No recipe: the seller quote.
    inline std::uint64_t BisCraftReference(std::uint64_t quote, double cost, double premium)
    {
        if (!(cost >= 0))
            return quote;
        return std::min<std::uint64_t>(quote, std::uint64_t(std::ceil(cost * premium)));
    }

    // Units of one BiS item the bot bought in one house on one server day.
    struct DailyCount { std::int64_t day = -1; std::uint32_t units = 0; };

    inline bool DailyAllows(DailyCount const& count, std::int64_t day, std::uint32_t units, std::uint32_t limit)
    {
        return !limit || (count.day == day ? count.units : 0) + std::uint64_t(units) <= limit;
    }

    inline void DailyRecord(DailyCount& count, std::int64_t day, std::uint32_t units)
    {
        if (count.day != day)
            count = {day, 0};
        count.units = std::uint32_t(std::min<std::uint64_t>(UINT32_MAX, std::uint64_t(count.units) + units));
    }

    inline bool WithinBisBuyerOffer(std::uint64_t buyout, std::uint64_t offer, std::uint32_t count)
    {
        return buyout && offer && offer <= UINT32_MAX && count && buyout <= offer * count;
    }
}
