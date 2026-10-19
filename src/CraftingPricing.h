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
struct Recipe { std::uint32_t output; std::vector<Material> materials; };
struct Item
{
    double quote = 0;       // Highest ordinary bot unit buyout (before recipe pricing).
    double vendor = 0;      // NPC pays the player this amount.
    std::vector<Recipe> recipes;
};
struct Price { double reference = 0; double floor = 0; };

class Model
{
public:
    std::map<ItemId, Item> items;

    void Build(double margin, double cut)
    {
        prices.clear();
        recipes.clear();
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
        Price price{std::max(item.quote, item.vendor), 0};
        if (std::isfinite(cheapest))
        {
            price.floor = std::ceil(std::max(cheapest, item.vendor) * factor);
            price.reference = std::max(item.quote, price.floor);
        }
        prices[id] = price;
        return price;
    }
};
}
#endif
