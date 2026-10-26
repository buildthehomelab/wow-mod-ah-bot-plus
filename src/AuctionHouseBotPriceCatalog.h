// Shared, optional server API. Quotes are copper per unit, never stock guarantees.
// Both modules must be built into the same worldserver (normal static module build).
#pragma once
#define AH_BOT_PRICE_CATALOG_BUYER_V1 1
#define AH_BOT_PRICE_CATALOG_BIS_DAILY_V1 1
#include <cstdint>

namespace AhBotPriceCatalog
{
    using Provider = std::uint64_t (*)(std::uint32_t item, std::uint32_t house);
    inline Provider provider = nullptr;
    // True means this item is handled by a reference buyer policy (BiS, or every quoted item
    // with Buyer.ReferencePricing); zero then means no offer. False leaves the item on the
    // legacy buyer path.
    using BuyerProvider = bool (*)(std::uint32_t item, std::uint32_t house, std::uint64_t& offer);
    inline BuyerProvider buyerProvider = nullptr;

    inline bool BuyerOffer(std::uint32_t item, std::uint32_t house, std::uint64_t& offer)
    {
        offer = 0;
        return buyerProvider && buyerProvider(item, house, offer);
    }


    // The BiS buyer's daily limit for an item it would buy in this house: units it still takes
    // today, the limit, and seconds until the next UTC day. False when no limit applies.
    using BisDailyProvider = bool (*)(std::uint32_t item, std::uint32_t house, std::uint32_t& left,
        std::uint32_t& limit, std::uint32_t& resetSeconds);
    inline BisDailyProvider bisDailyProvider = nullptr;

    inline bool BisDaily(std::uint32_t item, std::uint32_t house, std::uint32_t& left, std::uint32_t& limit,
        std::uint32_t& resetSeconds)
    {
        left = limit = resetSeconds = 0;
        return bisDailyProvider && bisDailyProvider(item, house, left, limit, resetSeconds);
    }

    inline std::uint64_t Quote(std::uint32_t item, std::uint32_t house)
    {
        return provider ? provider(item, house) : 0;
    }
}
