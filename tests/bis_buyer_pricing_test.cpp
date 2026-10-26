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

    // Reference buyer for every item: same integer share, capped by the NPC gold price.
    assert(ReferenceBuyerOffer(reference, 80, 0) == 1696000);
    assert(ReferenceBuyerOffer(reference, 80, 1000000) == 800000); // NPC sells it for 100g.
    assert(ReferenceBuyerOffer(reference, 80, 1e12) == 1696000);   // NPC price far above: no effect.
    assert(ReferenceBuyerOffer(0, 80, 1000) == 0);
    // Buying from the NPC and selling to the bot always loses gold, even before the cut.
    assert(ReferenceBuyerOffer(5000000, 99, 1234) < 1234);
    // BiS crafts pay making cost with the 1.5x premium: Lionheart Helm, ~539g of materials.
    constexpr double lionheartCost = 6520000; // ~652g: 539g x 1.15 / 0.95.
    auto lionheart = BisCraftReference(13004000, lionheartCost, 1.5);
    assert(lionheart == 9780000);
    assert(BisBuyerOffer(lionheart, 90) == 8802000); // ~880g offer, ~836g after the cut.
    assert(BisCraftReference(5000000, lionheartCost, 1.5) == 5000000); // Never above the quote.
    assert(BisCraftReference(13004000, -1, 1.5) == 13004000);          // No recipe: dropped BiS.
    assert(BisCraftReference(13004000, 0, 1.5) == 0);                  // Free to make: worth nothing.

    // Daily limit per house and item; a new day starts over, 0 means no limit.
    DailyCount day;
    assert(DailyAllows(day, 100, 1, 2));
    DailyRecord(day, 100, 1);
    DailyRecord(day, 100, 1);
    assert(!DailyAllows(day, 100, 1, 2) && DailyAllows(day, 100, 1, 0));
    assert(DailyAllows(day, 101, 2, 2) && !DailyAllows(day, 101, 3, 2));
    DailyRecord(day, 101, 1);
    assert(day.day == 101 && day.units == 1 && DailyAllows(day, 101, 1, 2) && !DailyAllows(day, 101, 2, 2));

    // At 80%, a craft from bot-priced ingredients returns about 92% after the 5% cut.
    auto craftNet = std::floor(ReferenceBuyerOffer(floorPrice, 80, 0) * .95);
    assert(craftNet < cost && craftNet >= 919999);
}
