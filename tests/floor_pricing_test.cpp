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
}
