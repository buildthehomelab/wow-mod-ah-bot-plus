#pragma once
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

    inline bool WithinBisBuyerOffer(std::uint64_t buyout, std::uint64_t offer, std::uint32_t count)
    {
        return buyout && offer && offer <= UINT32_MAX && count && buyout <= offer * count;
    }
}
