// Shared, optional server API. Quotes are copper per unit, never stock or buyer offers.
// Both modules must be built into the same worldserver (normal static module build).
#pragma once
#include <cstdint>

namespace AhBotPriceCatalog
{
    using Provider = std::uint64_t (*)(std::uint32_t item, std::uint32_t house);
    inline Provider provider = nullptr;

    inline std::uint64_t Quote(std::uint32_t item, std::uint32_t house)
    {
        return provider ? provider(item, house) : 0;
    }
}
