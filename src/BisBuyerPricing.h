#pragma once
#include <algorithm>
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

    inline bool WithinBisBuyerOffer(std::uint64_t buyout, std::uint64_t offer, std::uint32_t count)
    {
        return buyout && offer && offer <= UINT32_MAX && count && buyout <= offer * count;
    }
}
