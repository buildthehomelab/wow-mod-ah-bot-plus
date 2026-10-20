#include "../src/BagPricing.h"
#include <cassert>
#include <limits>
using namespace AhBotPricing;
int main()
{
    std::uint64_t bid = 9000, buyout = 10857;
    // Woolen bag: remove the inflated category quote, keep a net recipe margin.
    assert(ApplyBagPrice({10857, 1700}, true, 2500, 300, .05, UINT32_MAX, bid, buyout));
    assert(buyout == 1700 && bid == buyout && buyout < 2500 * .8);
    // Linen bag: a price cap cannot erase material replacement costs.
    bid = buyout = 8379;
    assert(!ApplyBagPrice({8379, 618}, true, 500, 200, .05, UINT32_MAX, bid, buyout));
    // No same-size gold vendor: crafted floor still replaces arbitrary markup.
    assert(ApplyBagPrice({270000, 62000}, true, 0, 20000, .05, UINT32_MAX, bid, buyout));
    assert(buyout == 62000 && bid == buyout);
    // Loot bags stay bounded without becoming vendor resale arbitrage.
    bid = buyout = 10000;
    assert(ApplyBagPrice({}, false, 20000, 2500, .05, UINT32_MAX, bid, buyout));
    assert(buyout == 10000);
    bid = buyout = 30000;
    assert(ApplyBagPrice({}, false, 20000, 2500, .05, UINT32_MAX, bid, buyout));
    assert(buyout == 15200 && bid == buyout);
    assert(!ApplyBagPrice({}, true, 20000, 2500, .05, UINT32_MAX, bid, buyout));
    assert(!ApplyBagPrice({0, 1700}, true, 2500, 300, 1, UINT32_MAX, bid, buyout));
    assert(!ApplyBagPrice({0, 1700}, true, 2500, 300, .05, 1600, bid, buyout));
    assert(!ApplyBagPrice({0, std::numeric_limits<double>::infinity()}, true, 0, 0, .05, UINT32_MAX, bid, buyout));
    // A low random bid and custom auction cut cannot sell below vendor opportunity value.
    bid = 1; buyout = 2;
    assert(ApplyBagPrice({}, false, 0, 1000, .15, UINT32_MAX, bid, buyout));
    assert(buyout == 1177 && bid == buyout);
}
