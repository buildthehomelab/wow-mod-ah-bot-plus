#include "../src/CraftingPricing.h"
#include "../src/ProgressionBis.h"
#include <cassert>
#include <cmath>
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
    // Enchant effects yield one scroll; bonus points do not reduce its ingredient cost.
    assert(GuaranteedOutput(true, 50, 1, 20) == 1);
    assert(GuaranteedOutput(false, 4, 1, 20) == 5);
    assert(GuaranteedOutput(false, 99, 1, 20) == 20);
    assert(GuaranteedOutput(false, -3, 0, 20) == 1);
    assert(!VellumFits(VellumKind::Armor, VellumKind::Weapon, 65, 35, false));
    assert(!VellumFits(VellumKind::Weapon, VellumKind::Weapon, 40, 60, false));
    assert(VellumFits(VellumKind::Weapon, VellumKind::Weapon, 65, 60, false));
    assert(VellumFits(VellumKind::Armor, VellumKind::Armor, 1, 60, true));
    Model bis;
    bis.items[1] = {100, 1, {}, 1.5}; // Noncrafted phase BiS: premium once.
    bis.items[2] = {10, 1, {{1, {{1, 2}}}}, 1.5}; // Crafted phase BiS.
    bis.items[3] = {10, 1, {{1, {{2, 1}}}}};
    bis.items[4] = {25, 0, {}}; // Vellum.
    bis.items[5] = {1, 0, {{1, {{1, 2}, {4, 1}}}}}; // Scroll includes vellum.
    bis.Build(0, 0);
    assert(bis.Get(1).reference == 150 && bis.Get(1).floor == 0);
    assert(bis.Get(2).floor == 450 && bis.Get(2).reference == 450);
    assert(bis.Get(3).floor == 450); // No second application of intermediate premium.
    assert(bis.Get(5).floor == 325); // Full reagent + vellum price, not 1/50 of it.
    // Independent toggles: removing crafting recipes retains raw demand, removing demand restores legacy reference.
    bis.items[2].recipes.clear(); bis.Build(0, 0);
    assert(bis.Get(2).floor == 0 && bis.Get(2).reference == 15);
    bis.items[2].demandMultiplier = 1; bis.Build(0, 0);
    assert(bis.Get(2).reference == 10);
    assert(IsProgressionBis(12640)); // Published Lionheart Helm.
    assert(!IsProgressionBis(0) && !IsProgressionBis(UINT32_MAX));
    for (std::size_t i = 1; i < ProgressionBis.size(); ++i)
        assert(ProgressionBis[i-1].item < ProgressionBis[i].item);
    // Regression: optional leather upgrades must not be the required cost of skinned leather.
    Model leather;
    leather.items[1] = {1181, 15, {}, 1, true}; // Light Leather, naturally supplied.
    leather.items[2] = {1983, 50, {{1, {{1, 4}}}}, 1, true};
    leather.items[3] = {3337, 150, {{1, {{2, 5}}}}, 1, true};
    leather.items[4] = {4683, 300, {{1, {{3, 6}}}}, 1, true};
    leather.items[5] = {5894, 500, {{1, {{4, 6}}}}, 1, true}; // Rugged Leather.
    leather.items[6] = {3400, 400, {}, 1, true}; // Runecloth.
    leather.items[7] = {5000, 1250, {}}; // Thread reference.
    leather.items[8] = {31849, 14155, {{1, {{5, 14}, {6, 10}, {7, 1}}}}}; // Runic Headband.
    leather.items[9] = {1000, 500, {{1, {{5, 1}, {7, 1}}}}}; // Crafted-only intermediate retains its floor.
    leather.Build(.15, .05);
    assert(leather.Get(5).reference == 5894 && leather.Get(5).floor == 0);
    assert(leather.Get(8).floor == 147099); // ~14.7g, not ~3,000g.
    assert(leather.Get(9).floor > 1000);
    leather.items[5].independentSupply = false;
    leather.Build(.15, .05);
    assert(leather.Get(5).floor > 5894); // Without independent supply, a real crafted route still costs materials.

    // Average yields: die roll (1..DieSides, +1 for one side) and chained specialization extras.
    assert(ExpectedOutput(false, 0, 1, 20, 100, 1) == 2); // Ebonweave specialist: always two.
    assert(ExpectedOutput(false, 0, 5, 20, 0, 0) == 3);   // 1..5 dynamite.
    assert(ExpectedOutput(true, 5, 0, 1, 50, 3) == 1.0 * (1 + .5 + .25 + .125)); // Scrolls make one.
    assert(ExpectedOutput(false, 9, 0, 5, 0, 0) == 5);    // Stack size clamps the roll.
    assert(std::abs(ExpectedOutput(false, 0, 1, 20, 15, 4) - 1.17638125) < 1e-9); // Alchemy mastery.

    // Reference buyer cost: a craft is worth its cheapest making, never its seller-only floors.
    Model buyer;
    buyer.items[1] = {10000, 100, {}};                                // 1g bot reference.
    buyer.items[2] = {30000000, 0, {{1, {{1, 10}}}}};                 // 3000g epic floor, 10g of materials.
    buyer.items[3] = {20000, 0, {{1, {{1, 1}}, 2}}};                  // A specialist always makes two.
    buyer.items[4] = {885800, 0, {{1, {{1, 50}}}}, 1, false, 120000}; // NPC sells it for 12g.
    buyer.items[5] = {3000000, 0, {{1, {{4, 2}}}}};                    // Made from that NPC part.
    buyer.items[6] = {50000, 0, {{1, {{7, 3}}}}};                      // Greater essence <- 3 lesser...
    buyer.items[7] = {3000, 0, {{3, {{6, 1}}}}};                       // ...and back: a cyclic pair.
    buyer.items[8] = {100000000, 0, {{1, {{1, 10}}}}, 1.5};            // BiS craft.
    buyer.items[9] = {500, 0, {}};                                     // Not craftable.
    buyer.Build(.15, .05);
    assert(buyer.Get(2).buyer == 121053 && buyer.Get(2).reference == 30000000);
    assert(buyer.Get(3).buyer == 6053 && buyer.Get(3).reference == 20000);
    // Two units sold to an 80% buyer after the 5% cut return less than the materials.
    assert(2 * std::uint64_t(buyer.Get(3).buyer * 80 / 100) * .95 < 10000);
    assert(buyer.Get(4).floor == 605264 && buyer.Get(5).buyer == 290527); // NPC part at 12g, not 88g.
    assert(buyer.Get(6).buyer == 10895);  // Cyclic recipes still bound the buyer...
    assert(buyer.Get(7).buyer == 3000);   // ...never above the reference.
    assert(buyer.Get(8).floor == 181579 && buyer.Get(8).buyer == 121053); // No demand premium.
    assert(buyer.Get(9).buyer == -1 && buyer.Get(1).buyer == -1); // No recipe: seller quote applies.
    Model free;
    free.items[1] = {0, 0, {}};
    free.items[2] = {5000000, 0, {{1, {{1, 1}}}}};
    free.Build(.15, .05);
    assert(free.Get(2).buyer == 0); // Free materials: a real zero cost, not "no recipe".
    assert(Model().Get(42).buyer == -1);
    buyer.floors = false; // Reference buyer without CraftedPricing: no seller floors, same buyer cost.
    buyer.Build(.15, .05);
    assert(buyer.Get(2).floor == 0 && buyer.Get(2).buyer == 121053 && buyer.Get(2).reference == 30000000);
    std::cout << "crafting pricing tests passed\n";
}
