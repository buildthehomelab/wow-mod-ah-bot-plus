#ifndef AH_BOT_RECIPE_PRICING_H
#define AH_BOT_RECIPE_PRICING_H

#include "FloorPricing.h"

namespace AhBotPricing
{
// A recipe is worth a share of what one craft yields (Produced = reference per craft).
// Only raises the ordinary quote. A recipe anyone can buy from an NPC for gold never
// climbs above that NPC price (VendorPurchase, 0 when there is no such offer).
// The bid keeps its usual discount below the buyout.
inline void ApplyRecipePrice(double produced, std::uint32_t percent, double vendorPurchase,
    std::uint64_t cap, std::uint64_t& bid, std::uint64_t& buyout)
{
    if (!std::isfinite(produced) || produced <= 0 || !percent)
        return;
    double floor = std::ceil(produced * percent / 100);
    if (vendorPurchase > 0)
        floor = std::min(floor, std::floor(vendorPurchase));
    RaiseToFloor(floor, cap, bid, buyout);
}
}
#endif
