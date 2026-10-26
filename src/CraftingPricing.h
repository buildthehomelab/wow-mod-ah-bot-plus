#ifndef AH_BOT_CRAFTING_PRICING_H
#define AH_BOT_CRAFTING_PRICING_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <vector>

// Pure pricing model: no auction/player input, random numbers or database access.
namespace AhBotPricing
{
using ItemId = std::uint32_t;
struct Material { ItemId item; std::uint32_t count; };
struct Recipe
{
    std::uint32_t output;
    std::vector<Material> materials;
    // Average units per cast, die roll and specialization extras included (0 = output).
    double expected = 0;
};
struct Item
{
    double quote = 0;       // Highest ordinary bot unit buyout (before recipe pricing).
    double vendor = 0;      // NPC pays the player this amount.
    std::vector<Recipe> recipes;
    double demandMultiplier = 1;
    // Gathering/loot supplies this commodity without running its optional conversion recipes.
    bool independentSupply = false;
    double npc = 0;         // An NPC sells it for this much gold (0 = none).
};
// buyer: what one unit costs to make from the cheapest sources (NPC, bot reference, own
// crafts at average yield) plus the crafting margin and cut, no demand premium; -1 when the
// item has no recipe. A buyer paying under this can't be fed crafts at a profit.
struct Price { double reference = 0; double floor = 0; double buyer = -1; };

enum class VellumKind { None, Armor, Weapon };
inline bool VellumFits(VellumKind required, VellumKind candidate, std::uint32_t level,
    std::uint32_t baseLevel, bool allowLowLevel)
{
    return required != VellumKind::None && required == candidate && (allowLowLevel || level >= baseLevel);
}

// The core rolls BasePoints + 1..DieSides (exactly +1 for one side), then each
// specialization extra follows on the previous one with the same chance, up to maxExtra.
inline double ExpectedOutput(bool scroll, std::int64_t base, std::int64_t dice, std::uint32_t stack,
    double extraChancePercent, std::int32_t maxExtra)
{
    double quantity = scroll ? 1 : base + (dice ? (1 + dice) / 2.0 : 0);
    quantity = std::min<double>(std::max(quantity, 1.0), std::max<std::uint32_t>(stack, 1));
    double p = std::clamp(extraChancePercent / 100, 0.0, 1.0), step = 1, extras = 1;
    for (std::int32_t i = 0; i < maxExtra; ++i)
        extras += step *= p;
    return quantity * extras;
}

inline std::uint32_t GuaranteedOutput(bool scroll, std::int64_t base, std::int64_t dice, std::uint32_t stack)
{
    auto quantity = scroll ? 1 : base + (dice > 0 ? 1 : dice);
    return std::min<std::uint64_t>(std::max<std::int64_t>(quantity, 1), std::max<std::uint32_t>(stack, 1));
}

class Model
{
public:
    std::map<ItemId, Item> items;
    bool floors = true;     // false: recipes only feed the buyer cost, not seller floors.

    void Build(double margin, double cut)
    {
        prices.clear();
        recipes.clear();
        acquire.clear();
        valid = std::isfinite(margin) && margin >= 0 && std::isfinite(cut) && cut >= 0 && cut < 1;
        if (!valid)
            return;
        factor = (1 + margin) / (1 - cut);
        // Reject only cyclic recipe routes, retaining other recipes for the same item.
        // This deliberately excludes reversible conversions rather than escalating their prices.
        for (auto const& [id, item] : items)
            for (auto const& recipe : item.recipes)
            {
                bool usable = recipe.output && !recipe.materials.empty();
                for (auto const& material : recipe.materials)
                {
                    std::set<ItemId> seen;
                    if (!material.count || !items.count(material.item) || Reaches(material.item, id, seen))
                        usable = false;
                }
                if (usable)
                    recipes[id].push_back(recipe);
            }
        for (auto const& [id, item] : items)
            Resolve(id);
        for (auto& [id, price] : prices)
            price.buyer = BuyerCost(id);
    }

    Price Get(ItemId id) const
    {
        auto it = prices.find(id);
        return it == prices.end() ? Price{} : it->second;
    }

    bool IsValid() const { return valid; }

    static bool Apply(Price const& price, std::uint64_t cap, std::uint64_t& bid, std::uint64_t& buyout)
    {
        if (price.floor <= 0)
            return bid <= cap && buyout <= cap;
        if (!std::isfinite(price.floor) || price.floor > static_cast<double>(cap))
            return false;
        auto floor = static_cast<std::uint64_t>(std::ceil(price.floor));
        buyout = std::max(buyout, floor);
        bid = std::min(buyout, std::max(bid, floor));
        return buyout <= cap;
    }

private:
    double factor = 1;
    bool valid = false;
    std::map<ItemId, std::vector<Recipe>> recipes;
    std::map<ItemId, Price> prices;
    std::map<ItemId, double> acquire;

    bool Reaches(ItemId from, ItemId target, std::set<ItemId>& seen) const
    {
        if (from == target)
            return true;
        if (!seen.insert(from).second)
            return false;
        auto it = items.find(from);
        if (it != items.end())
            for (auto const& recipe : it->second.recipes)
                for (auto const& material : recipe.materials)
                    if (Reaches(material.item, target, seen))
                        return true;
        return false;
    }

    Price Resolve(ItemId id)
    {
        auto found = prices.find(id);
        if (found != prices.end())
            return found->second;
        auto const& item = items.at(id);
        double cheapest = std::numeric_limits<double>::infinity();
        for (auto const& recipe : recipes[id])
        {
            double cost = 0;
            for (auto const& material : recipe.materials)
                cost += Resolve(material.item).reference * material.count;
            cheapest = std::min(cheapest, cost / recipe.output);
        }
        double demand = std::isfinite(item.demandMultiplier) && item.demandMultiplier >= 1 ? item.demandMultiplier : 1;
        Price price{std::max(item.quote, item.vendor) * demand, 0};
        if (floors && !item.independentSupply && std::isfinite(cheapest))
        {
            price.floor = std::ceil(std::max(cheapest, item.vendor) * factor * demand);
            price.reference = std::max(price.reference, price.floor);
        }
        prices[id] = price;
        return price;
    }

    static double Yield(Recipe const& recipe)
    {
        return recipe.expected > 0 ? recipe.expected : recipe.output;
    }

    // Cheapest unit cost without margin: bot reference, NPC price, or crafting it from
    // acyclic recipes at average yield.
    double Acquire(ItemId id)
    {
        auto found = acquire.find(id);
        if (found != acquire.end())
            return found->second;
        auto const& item = items.at(id);
        double best = prices.at(id).reference;
        if (item.npc > 0)
            best = std::min(best, item.npc);
        for (auto const& recipe : recipes[id])
        {
            double cost = 0;
            for (auto const& material : recipe.materials)
                cost += Acquire(material.item) * material.count;
            best = std::min(best, cost / Yield(recipe));
        }
        acquire[id] = best;
        return best;
    }

    // Every listed recipe counts here, cyclic conversions too: their materials are costed
    // without the loop, so converting bot stock and selling the result back can't pay.
    double BuyerCost(ItemId id)
    {
        auto const& item = items.at(id);
        double cheapest = std::numeric_limits<double>::infinity();
        for (auto const& recipe : item.recipes)
        {
            if (!recipe.output || recipe.materials.empty())
                continue;
            double cost = 0;
            for (auto const& material : recipe.materials)
                cost += items.count(material.item) ? Acquire(material.item) * material.count
                    : std::numeric_limits<double>::infinity();
            cheapest = std::min(cheapest, cost / Yield(recipe));
        }
        if (!std::isfinite(cheapest))
            return -1;
        return std::min(prices.at(id).reference, std::ceil(std::max(cheapest, item.vendor) * factor));
    }
};
}
#endif
