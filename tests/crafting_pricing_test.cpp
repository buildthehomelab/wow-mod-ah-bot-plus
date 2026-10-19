#include "../src/CraftingPricing.h"
#include <cassert>
#include <iostream>

using namespace AhBotPricing;
int main()
{
    Model model;
    model.items[1] = {100000, 1000, {}}; // materials: 10g upper bot quote
    model.items[2] = {100, 10, {{1, {{1, 1}}}}};
    model.Build(.15, .05);
    auto price = model.Get(2);
    assert(price.floor == 121053); // 15% net profit after a 5% cut, rounded upward
    std::uint64_t bid = 20, buyout = 25;
    assert(Model::Apply(price, 1000000, bid, buyout));
    assert(bid == 121053 && buyout == bid); // Random discounts and overrides cannot undercut it
    assert(buyout * .95 >= 115000);
    bid = 10; buyout = 20;
    assert(!Model::Apply(price, 100000, bid, buyout)); // Skip an impossible cap, never clamp below floor

    model.items[3] = {10, 1, {{5, {{1, 1}}}, {1, {{1, 2}}}}};
    model.items[4] = {1, 1, {{1, {{2, 2}}}}};
    model.Build(.15, .15);
    assert(model.Get(3).floor == 27059); // Cheapest alternate, five outputs, neutral cut
    assert(model.Get(4).floor >= 366090); // Crafted intermediates use their effective quote
    assert(model.Get(1).floor == 0); // Raw material prices do not change

    model.items[5] = {300, 30, {{1, {{6, 1}}}, {1, {{1, 1}}}}};
    model.items[6] = {400, 40, {{1, {{5, 1}}}}};
    model.Build(0, 0);
    assert(model.Get(5).floor == 100000); // Cyclic route removed, valid alternate survives
    assert(model.Get(6).floor == 0); // Purely cyclic conversion falls back to legacy
    assert(model.Get(2).floor == 100000);
    model.Build(.15, 1);
    assert(!model.IsValid());
    model.Build(.15, .05); // Reload clears invalid/cached state
    assert(model.IsValid() && model.Get(2).floor == 121053);

    Price overflow{0, 1e30};
    assert(!Model::Apply(overflow, UINT32_MAX, bid, buyout));
    Model vendor;
    vendor.items[1] = {5, 100, {}};
    vendor.items[2] = {1, 1000, {{1, {{1, 1}}}}};
    vendor.Build(.15, .05);
    assert(vendor.Get(2).floor == 1211); // Finished item's own vendor value stays protected
    std::cout << "crafting pricing tests passed\n";
}
