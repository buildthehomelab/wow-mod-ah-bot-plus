#include "../src/MaterialStock.h"
#include <cassert>
#include <limits>

int main()
{
    using namespace AhBotStock;
    std::vector<std::uint32_t> materials{10, 20, 30, 40};
    std::map<std::uint32_t, std::uint32_t> alliance, horde;
    std::size_t ac = 0, hc = 0;
    Count(alliance, 10, 12, 20);
    Count(alliance, 10, 8, 20); // Multiple stacks add up to the unit reserve.
    Count(alliance, 20, 7, 20);
    auto a = Plan(materials, alliance, 20, 2, ac);
    assert(a.size() == 2 && a[0].item == 20 && a[0].deficit == 13 && a[1].item == 30);
    auto h = Plan(materials, horde, 20, 2, hc);
    assert(h.size() == 2 && h[0].item == 10 && h[0].deficit == 20);
    // Even if prior requests cannot be priced/listed, later IDs get their turn.
    a = Plan(materials, alliance, 20, 1, ac);
    assert(a.size() == 1 && a[0].item == 40);
    for (auto id : materials)
        alliance[id] = 20;
    assert(Plan(materials, alliance, 20, 500, ac).empty());
    // A purchase/expiry removes stock from the next live snapshot.
    alliance[30] = 3;
    a = Plan(materials, alliance, 20, 500, ac);
    assert(a.size() == 1 && a[0].item == 30 && a[0].deficit == 17);
    // Saturating counts cannot overflow or count duplicates beyond the target.
    Count(alliance, 30, std::numeric_limits<std::uint32_t>::max(), 20);
    Count(alliance, 30, std::numeric_limits<std::uint32_t>::max(), 20);
    assert(alliance[30] == 20);
    assert(Plan(materials, alliance, 20, 0, ac).empty());
    assert(Plan({}, alliance, 20, 500, ac).empty());
    assert(Plan(materials, alliance, 0, 500, ac).empty());
    ac = 999999; // Catalog reload/shortening never invalidates the cursor.
    assert(Plan({30}, {}, 20, 500, ac).size() == 1);
}
