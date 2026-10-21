#include "../src/RecipePricing.h"
#include <cassert>
#include <limits>
using namespace AhBotPricing;
int main()
{
    // Plans: Sulfuron Hammer at the generic epic-recipe quote, hammer worth 2320g.
    std::uint64_t bid = 945000, buyout = 1050000;
    ApplyRecipePrice(23205000, 50, 0, UINT32_MAX, bid, buyout);
    assert(buyout == 11602500 && bid == 10442250);
    // BiS output at 100%: the recipe matches one crafted item.
    bid = buyout = 1312500;
    ApplyRecipePrice(10329000, 100, 0, UINT32_MAX, bid, buyout);
    assert(buyout == 10329000 && bid == buyout);
    // Only raises: a recipe already above its share keeps its quote.
    bid = 1800000; buyout = 2100000;
    ApplyRecipePrice(444000, 100, 0, UINT32_MAX, bid, buyout);
    assert(buyout == 2100000 && bid == 1800000);
    // Open gold vendor offer caps the raise, never lowers the quote.
    bid = buyout = 1000;
    ApplyRecipePrice(500000, 50, 4000, UINT32_MAX, bid, buyout);
    assert(buyout == 4000 && bid == 4000);
    bid = buyout = 9000;
    ApplyRecipePrice(500000, 50, 4000, UINT32_MAX, bid, buyout);
    assert(buyout == 9000);
    // Price cap clamps instead of refusing the listing.
    bid = buyout = 1000;
    ApplyRecipePrice(1e12, 100, 0, UINT32_MAX, bid, buyout);
    assert(buyout == UINT32_MAX && bid == buyout);
    // Unknown output value, zero percent and bad input leave the quote alone.
    bid = 900; buyout = 1000;
    ApplyRecipePrice(0, 50, 0, UINT32_MAX, bid, buyout);
    ApplyRecipePrice(500000, 0, 0, UINT32_MAX, bid, buyout);
    ApplyRecipePrice(std::numeric_limits<double>::infinity(), 50, 0, UINT32_MAX, bid, buyout);
    ApplyRecipePrice(std::numeric_limits<double>::quiet_NaN(), 50, 0, UINT32_MAX, bid, buyout);
    assert(buyout == 1000 && bid == 900);
    // A zero quote still gets a sane bid.
    bid = buyout = 0;
    ApplyRecipePrice(300, 50, 0, UINT32_MAX, bid, buyout);
    assert(buyout == 150 && bid == 150);
}
