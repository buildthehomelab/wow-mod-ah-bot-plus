#include "../src/BisBuyerPricing.h"
#include "../src/CraftingPricing.h"
#include "../src/ProgressionBis.h"
#include <cassert>
#include <cmath>

int main()
{
    using namespace AhBotPricing;
    assert(IsProgressionBis(12783)); // Heartseeker: Classic pre-BiS combat daggers.
    assert(!IsProgressionBis(8170)); // Rugged Leather remains on legacy buying policy.
    constexpr std::uint64_t reference = 2120000;
    auto offer = BisBuyerOffer(reference, 90);
    assert(offer == 1908000);
    assert(WithinBisBuyerOffer(offer, offer, 1));
    assert(!WithinBisBuyerOffer(offer + 1, offer, 1));
    assert(WithinBisBuyerOffer(offer * 20, offer, 20));
    assert(!WithinBisBuyerOffer(offer * 20 + 1, offer, 20));
    assert(!WithinBisBuyerOffer(0, offer, 1));
    assert(!WithinBisBuyerOffer(offer, offer, 0));
    assert(BisBuyerOffer(0, 90) == 0);
    assert(BisBuyerOffer(reference, 0) == 0);
    assert(BisBuyerOffer(reference, 100) == 0);
    assert(BisBuyerOffer(std::uint64_t(UINT32_MAX) + 1, 90) == 0);
    assert(BisBuyerOffer(1001, 90) == 900); // Exact integer floor, no float rounding drift.
    assert(BisBuyerOffer(UINT32_MAX, 90) == std::uint64_t(UINT32_MAX) * 90 / 100);
    assert(BisBuyerOffer(reference, 99) < reference); // Buying freshly posted bot stock cannot pay back more.

    // A 15% recipe floor leaves at least 3.5% after the 90% offer and 5% cut,
    // less at most one copper due to rounding. Failed deposits are separate.
    constexpr std::uint64_t cost = 1000000;
    auto floorPrice = std::uint64_t(std::ceil(cost * 1.15 / .95));
    auto net = std::uint64_t(std::floor(BisBuyerOffer(floorPrice, 90) * .95));
    assert(net >= 1034999);
}
