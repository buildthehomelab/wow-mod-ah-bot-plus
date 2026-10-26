#include "../src/FloorPricing.h"
#include <cassert>
#include <cmath>
#include <limits>
using namespace AhBotPricing;
int main()
{
    // Raise keeps the bid discount; never lowers; clamps to the cap.
    std::uint64_t bid = 900, buyout = 1000;
    RaiseToFloor(5000, UINT32_MAX, bid, buyout);
    assert(buyout == 5000 && bid == 4500);
    RaiseToFloor(2000, UINT32_MAX, bid, buyout);
    assert(buyout == 5000 && bid == 4500);
    RaiseToFloor(1e12, UINT32_MAX, bid, buyout);
    assert(buyout == UINT32_MAX);
    bid = 7; buyout = 10;
    RaiseToFloor(0, UINT32_MAX, bid, buyout);
    RaiseToFloor(-5, UINT32_MAX, bid, buyout);
    RaiseToFloor(std::numeric_limits<double>::quiet_NaN(), UINT32_MAX, bid, buyout);
    assert(buyout == 10 && bid == 7);
    bid = buyout = 0;
    RaiseToFloor(12.2, UINT32_MAX, bid, buyout);
    assert(buyout == 13 && bid == 13);

    // Epic gear: 100g at item level 66, cubic growth.
    assert(EpicGearFloor(66, 1000000, 66, 3) == 1000000);
    assert(std::lround(EpicGearFloor(132, 1000000, 66, 3)) == 8000000);
    assert(EpicGearFloor(64, 1000000, 66, 3) < 1000000);
    assert(EpicGearFloor(0, 1000000, 66, 3) == 0);
    assert(EpicGearFloor(66, 1000000, 0, 3) == 0);
    assert(EpicGearFloor(66, 0, 66, 3) == 0);

    // Mount tiers: riding thresholds, engineering machines one tier down.
    assert(MountTier(75, true) == 0 && MountTier(149, true) == 0);
    assert(MountTier(150, true) == 1 && MountTier(225, true) == 2 && MountTier(300, true) == 3);
    assert(MountTier(0, true) == 0);
    assert(MountTier(300, false) == 2 && MountTier(375, false) == 3 && MountTier(50, false) == 0);

    // Level 60 blue: one Large Brilliant Shard (group 1, explicit 99.5%) or a Nexus Crystal (0.5%).
    auto blue = ExpectedDrops({{14344, 99.5, 1, 1, 1}, {20725, 0.5, 1, 1, 1}});
    assert(std::fabs(blue[14344] - 0.995) < 1e-9 && std::fabs(blue[20725] - 0.005) < 1e-9);
    // Green: 75% dust 2-5 explicit; the two chance-0 rows share the remaining 25%.
    auto green = ExpectedDrops({{1, 75, 1, 2, 5}, {2, 0, 1, 1, 2}, {3, 0, 1, 1, 1}});
    assert(std::fabs(green[1] - 2.625) < 1e-9 && std::fabs(green[2] - .1875) < 1e-9 && std::fabs(green[3] - .125) < 1e-9);
    // Ungrouped rows are independent; min > max counts as min.
    auto loose = ExpectedDrops({{4, 50, 0, 2, 2}, {4, 10, 0, 3, 1}, {5, 0, 0, 1, 1}});
    assert(std::fabs(loose[4] - 1.3) < 1e-9 && !loose.count(5));
    // Over-full group never yields a negative share.
    auto over = ExpectedDrops({{6, 120, 2, 1, 1}, {7, 0, 2, 1, 1}});
    assert(over[6] == 1 && !over.count(7));

    // Containers: a Reference row rolls its reference table MaxCount times.
    DropTables references{
        {1000, {{10, 50, 1, 1, 1}, {11, 0, 1, 1, 1}}},              // one gem per roll, 50/50
        {2000, {{12, 100, 0, 3, 5}}},                              // always 3-5 ore
        {3000, {{13, 100, 0, 1, 1}, {0, 100, 0, 1, 1, 3000}}}};    // refers to itself
    auto sack = ExpectedDrops({{0, 100, 0, 1, 2, 1000}, {14, 25, 0, 2, 2}}, &references);
    assert(std::fabs(sack[10] - 1) < 1e-9 && std::fabs(sack[11] - 1) < 1e-9 && std::fabs(sack[14] - .5) < 1e-9);
    // Grouped references share the group like items do; missing tables add nothing.
    auto box = ExpectedDrops({{0, 40, 1, 1, 1, 1000}, {0, 0, 1, 1, 1, 2000}, {0, 100, 0, 1, 1, 9999}}, &references);
    assert(std::fabs(box[10] - .2) < 1e-9 && std::fabs(box[12] - 2.4) < 1e-9 && box.size() == 3);
    // Without the reference store, reference rows are skipped; reference loops stop.
    assert(ExpectedDrops({{0, 100, 0, 1, 1, 1000}}).empty());
    auto loop = ExpectedDrops({{0, 100, 0, 1, 1, 3000}}, &references);
    assert(std::fabs(loop[13] - 8) < 1e-9);

    // Container value: contents at their price plus the money roll.
    auto price = [](std::uint32_t item) { return item == 10 ? 1000.0 : item == 11 ? -5.0 : std::nan(""); };
    assert(ContainerValue({{10, 1.5}, {11, 2}, {12, 3}}, 0, 0, price) == 1500);
    assert(ContainerValue({}, 100, 300, price) == 200);
    assert(ContainerValue({{10, 1}}, 500, 200, price) == 1200);

    // Placeholder names.
    for (char const* name : {"Unused Red Leather C03 Gloves", "Durability Gloves", "Monster - Fire Arrow",
        "Deprecate Snare Kit", "90 Green Warrior Axe", "63 Green Rogue Belt", "Red Leather C03 Boots",
        "Black Leather D02 Bracers", "JYoo Random Item Test"})
        assert(IsPlaceholderName(name));
    for (char const* name : {"Greenstone Circle", "Green Dragonscale Leggings", "Lawbringer Belt", "Monster Cloak",
        "Durable Gloves", "2 Green Beans", "D02", ""})
        assert(IsPlaceholderName(name) == (std::string(name) == "2 Green Beans"));
}
