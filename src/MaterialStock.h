#ifndef AH_BOT_MATERIAL_STOCK_H
#define AH_BOT_MATERIAL_STOCK_H

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace AhBotStock
{
struct Policy
{
    std::uint32_t minimum;
    std::uint32_t refill;
};
enum class Kind { Reserve, PricingError, LargeStack };
struct Request
{
    std::uint32_t item;
    std::uint32_t deficit;
    Kind kind = Kind::Reserve;
};
inline Policy Normalize(std::uint32_t minimum, std::uint32_t refill, std::uint32_t recipeUnits = 0)
{
    minimum = std::clamp(std::max(minimum, recipeUnits), 1u, 10000u);
    auto recipeBatch = std::min<std::uint64_t>(std::uint64_t(recipeUnits) * 2, 10000);
    return {minimum, std::max(minimum, std::uint32_t(std::min<std::uint64_t>(10000,
        std::max<std::uint64_t>(refill, recipeBatch))))};
}

// Compare exact stack prices, without truncating per-unit asks or overflowing products.
inline bool Affordable(std::uint64_t buyout, std::uint32_t count, std::uint64_t reference)
{
    if (!buyout || !count || !reference)
        return false;
    auto unit = buyout / count;
    return unit < reference || (unit == reference && buyout % count == 0);
}

// Observe both live auctions and queued normal refills before planning specials.
// Vendor-value normal supply suppresses a new deliberate pricing-error event.
struct SpecialSupply
{
    bool vendorValueSupply = false;
    std::uint32_t largeListings = 0;
    void Observe(std::uint64_t buyout, std::uint32_t count, std::uint32_t vendor)
    {
        vendorValueSupply = vendorValueSupply || Affordable(buyout, count, vendor);
        if (count > 20)
            largeListings = std::min(2u, largeListings + 1);
    }
};

// Exceptional large stacks bypass only the normal listing cap, never the loaded
// template or uint32 auction money limits. Reject an event that cannot exceed 20.
inline std::uint32_t ListingUnits(Kind kind, std::uint32_t requested,
    std::uint32_t templateLimit, std::uint32_t normalLimit, std::uint64_t unitPrice)
{
    if (!unitPrice)
        return 0;
    auto limit = kind == Kind::LargeStack ? templateLimit : std::min(templateLimit, std::min(20u, normalLimit));
    auto units = std::min<std::uint64_t>(std::min(requested, limit), UINT32_MAX / unitPrice);
    return kind == Kind::LargeStack && units <= 20 ? 0 : std::uint32_t(units);
}

inline std::uint32_t VendorBargain(std::uint32_t vendor, std::uint32_t percent)
{
    if (!vendor)
        return 0;
    return std::max<std::uint32_t>(1, std::uint64_t(vendor) * std::clamp(percent, 1u, 100u) / 100);
}

// Strict item:min:refill overrides; malformed records never silently weaken reserves.
inline bool ParseOverride(std::string const& text, std::uint32_t& item, Policy& policy)
{
    std::uint32_t values[3]{};
    std::size_t begin = 0;
    for (unsigned i = 0; i < 3; ++i)
    {
        auto end = text.find(':', begin);
        if ((i < 2 && end == std::string::npos) || (i == 2 && end != std::string::npos))
            return false;
        if (end == std::string::npos)
            end = text.size();
        auto result = std::from_chars(text.data() + begin, text.data() + end, values[i]);
        if (result.ec != std::errc{} || result.ptr != text.data() + end)
            return false;
        begin = end + 1;
    }
    if (!values[0] || !values[1] || values[1] > values[2] || values[2] > 10000)
        return false;
    item = values[0];
    policy = {values[1], values[2]};
    return true;
}

inline std::uint32_t Units(std::map<std::uint32_t, std::uint32_t> const& supply, std::uint32_t item)
{
    auto it = supply.find(item);
    return it == supply.end() ? 0 : it->second;
}

// Keep refilling across capped-stack cycles until the high target is reached.
// Sufficient affordable player supply cancels the batch, leaving those sales to players.
inline std::vector<Request> Plan(std::vector<std::uint32_t> const& materials,
    std::map<std::uint32_t, Policy> const& policies,
    std::map<std::uint32_t, std::uint32_t> const& bots,
    std::map<std::uint32_t, std::uint32_t> const& players,
    std::set<std::uint32_t>& refilling, std::uint32_t budget, std::size_t& cursor)
{
    std::vector<Request> result;
    if (materials.empty() || !budget)
        return result;
    cursor %= materials.size();
    std::size_t visited = 0;
    while (visited < materials.size() && result.size() < budget)
    {
        auto item = materials[cursor];
        cursor = (cursor + 1) % materials.size();
        ++visited;
        auto it = policies.find(item);
        if (it == policies.end())
            continue;
        auto policy = it->second;
        auto playerUnits = Units(players, item);
        auto total = std::min<std::uint64_t>(policy.refill, std::uint64_t(Units(bots, item)) + playerUnits);
        if (playerUnits >= policy.minimum || total >= policy.refill)
        {
            refilling.erase(item);
            continue;
        }
        if (total < policy.minimum)
            refilling.insert(item);
        if (refilling.count(item))
            result.push_back({item, policy.refill - std::uint32_t(total)});
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
