#ifndef AH_BOT_BAG_PRICING_H
#define AH_BOT_BAG_PRICING_H

#include "CraftingPricing.h"

namespace AhBotPricing
{
// VendorPurchase is what a player pays the NPC per bag, never SellPrice.
// Reserve the full 20% reputation discount plus 5% further undercut.
inline bool ApplyBagPrice(Price const& craft, bool crafted, double vendorPurchase,
    double vendorSell, double cut, std::uint64_t cap,
    std::uint64_t& bid, std::uint64_t& buyout)
{
    if (!std::isfinite(cut) || cut < 0 || cut >= 1)
        return false;
    // A recognized craft without a resolved floor has only unsupported/cyclic routes.
    if (crafted && (!std::isfinite(craft.floor) || craft.floor <= 0))
        return false;
    double floor = std::ceil(std::max(crafted ? craft.floor : 1.0, vendorSell / (1 - cut)));
    double ceiling = static_cast<double>(cap);
    if (vendorPurchase > 0)
        ceiling = std::min(ceiling, std::floor(vendorPurchase * .76));
    if (!std::isfinite(floor) || floor > ceiling || ceiling < 1)
        return false;
    // Finished bags use their recipe floor, rather than the category markup.
    // Non-crafted bags keep their ordinary quote within the same vendor bound.
    buyout = static_cast<std::uint64_t>(crafted ? floor :
        std::max(floor, std::min(static_cast<double>(buyout), ceiling)));
    bid = std::min(buyout, std::max(static_cast<std::uint64_t>(floor), bid));
    return true;
}
}
#endif
