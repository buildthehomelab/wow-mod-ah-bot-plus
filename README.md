## Auction Bot Plus

This is a fork of the auction house bot for AzerothCore.  This fork gives a much more blizzlike experience in the offerings on the auction house, and it is built with the following ethos: "Maximum understandable customization in the hands of admins via non-SQL configurations".  Here are some of the feature highlights:
 - All configuration is done via config files, with no SQL configuration needed
 - Listing control allowing item inclusion or exclusion rules, category list proportions, stack size management, etc
 - Fully customizable and predicable pricing which can be based on category, subcategory, quality, item level, etc
 - Optional advanced price calculation rules for those that want a more out-of-the-box Blizzlike pricing
 - A customizable buying bot that can range from being fair (like the selling bot) to being stingy or greedy
 - Multiple AH bot support, to allow diverse listing names
 - GM server commands to reload configs and regenerate auctions for swift tuning
 - A bunch of QoL features that don't get their own row here

## Requirements
Requires an AzerothCore version that is caught up to at least change set 3f46e05 (https://github.com/azerothcore/azerothcore-wotlk/commit/3f46e05d3691895b6b8a5b3832d17ecb1e210791)

## Installation

```
1. Simply place the module under the `modules` directory of your AzerothCore source. 
2. Re-run cmake and launch a clean build of AzerothCore.
```

## Usage

**Before you do anything, make at least one character to use as the bot**.  Edit the module configuration and **add 1 or more character GUIDs ID to the "AuctionHouseBot.GUIDs"** variable, which is sourced from your character table in the character database.  These names will be visable in the auction house, so pick good names.  IMPORTANT: If you are using a bot mod (like playerbots), then ensure you use regular non-bot characters for your auctionhouse character(s).  After that, ensure you set **AuctionHouseBot.EnableSeller = true**.  Nothing else is required unless you want to tune stuff or enable the buyer bot, which is done with AuctionHouseBot.Buyer.Enabled.

Notes:
- These accounts do not need any security level and can be a player accounts.
- The characters used by the ahbot are not meant to be used ingame. If you use it to browse the auction house, you might have issues like "Searching for items..." displaying forever.
- It takes a few hours for the auction house to fully populate, as only 75 items gets added by default every 'tick'.  You can change this in the config with the AuctionHouseBot.ItemsPerCycle variable.
- All price multpliers (along with the advanced pricing, see config) are applied multiplicative.  Example: A Category of 1.5x, Quality of 2x, and CategoryQuality of 1.4x would make the multiplier 4.2 (1.5 x 2 x 1.4).  The advanced pricing would then multiply that value further.  Using item level price multpliers, which create a multiplier of itemlevel x value, is also multiplicitive along with the others.  You cannot use item level price multipliers and advanced pricing, as advanced pricing will take priority between the two.
- Bot-listed prices will not exceed 100k gold buyout.  This can be reduced via the configuration if you want.

### In-Game Commands

The AuctionHouseBot module adds the following in-game commands (for GMs only):

| Command | Description |
|----------|--------------|
| `.ahbot status` | Shows enabled features, house routing, live auction counts, material shortages/refill units and buyer diagnostics. Read-only; does not trigger buying or selling. |
| `.ahbot reload` | Reloads the AuctionHouseBot configuration file and updates settings. |
| `.ahbot empty` | Removes all AuctionHouseBot auctions from all auction houses.<br>Player auctions are unaffected. Bids on affected items are returned to players.<br>Use with caution! |
| `.ahbot update` | Forces the bot to refresh or repopulate auction listings immediately.<br>Note: If you have multiple minutes configured between Buy/Sell cycles, you may have to run this additional times before you see results. |


## Buying Bot Behavior

1. **Determining Items to Buy:** Every minute the buyer bot will select (BuyCandidatesPerBuyCycle) items currently up for auction which are listed by players as potential purchase items.
2. **Price Willing to Pay:** The buyer bot will use the same item price calculation the seller bot uses, including the random +/- 25% (configurable), and that calculated price is then multiplied by (AcceptablePriceModifier) which then becomes the price the buyer will be willing to spend.
3. **Buying it:** If the price calculated is higher than the buy out price, then the bot will buy it out.  If not, it will test to see if it has been bidded on by a bot and if not, if the bid price is below the price it is willing to pay.  If so, it will bid (outbid, so just over current bid).

The above behavior is replicated on each enabled auction house.  If left to default settings, 1 item in each auction house will attempt to be bought from, if the price calculation seems favorable. Note that any item buy attempt, even items above buying price, consumes a buy candidate. That means that too many overpriced items can drown out potential sales.


## Additional Resources

[Advanced Pricing Calculator](tools/AdvancedPricingCalculator/Advanced_Pricing_Calculator.xlsx) - If `AuctionHouseBot.AdvancedPricing.<Category>.<Subclass>.Enabled` is enabled, this spreadsheet can help quickly visualize how config settings affect in-game prices.


## Credits

- NathanHandley: Created this rewrite of the one that was ported to AzerothCore
- Zeb139: Lots of great enhancements, like enhanced pricing formulas and performance improvements (plus more)
- Ayase: ported the bot to AzerothCore
- Other contributors (check the contributors list)

## Recipe-aware crafted prices

Enable `AuctionHouseBot.CraftedPricing.Enabled` to stop newly generated crafted stock
undercutting its ingredients. `AuctionHouseBot.CraftedPricing.ProfitMargin = 0.15`
sets a 15% margin over ingredient replacement cost **after the auction cut**. With
10g of ingredients and a 5% auction cut, the minimum listing is 12g 10s 53c.
A neutral house or `Rate.Auction.Cut` change uses the corresponding cut automatically.
Both starting bids and buyouts respect the floor, including complete item overrides.
The existing valuation can still produce a higher price. Failed-sale deposits are
not covered, and a profitable asking price does not guarantee that an item sells.

Ingredient replacement costs use a deterministic upper quote from the bot's existing
configured pricing, including its maximum random markup. This is a stable reference
consistent with **new** bot listings, not a scan of the cheapest current auction.
Player listings cannot change it. Crafted ingredients use their own recipe-aware
reference recursively. For multiple recipes, the cheapest supported recipe wins;
output counts use the guaranteed ordinary yield capped by the item's stack limit.
Bonus procs are excluded so margins do not depend on a lucky craft.

Supported recipes have a profession skill association, one ordinary create-item effect
(or a permanent enchant effect producing a vellum scroll), known positive reagents and
no level-dependent ordinary yield or triggered/random creation.
Cyclic conversion routes are excluded conservatively, while other recipes for the
same item remain eligible. Recipes with only excluded routes keep legacy pricing.
Recipe eligibility is catalog-based, not character-specific: profession specialization,
progression, cooldown availability and learned-recipe requirements are not modeled.
Prices refer to immediate ingredients and recursively priced intermediates; they do
not assume buying vendor ingredients at discounted prices. When an ingredient has no
bot stock, its configured bot valuation remains an estimate, not a promise of availability.

The model is built lazily after spell/item loading and cached by auction cut. `.ahbot
reload` invalidates it so the next seller cycle rebuilds it from current configuration.
It never queries or writes auction/character tables for valuation. Items whose floor
exceeds `AuctionHouseBot.MaxBuyoutPriceInCopper` (or uint32 auction money capacity)
are skipped, and stack sizes are reduced when required to fit the auction's money fields.

The feature defaults off and preserves the legacy RNG/pricing path when disabled.
It affects the seller only. The buyer intentionally keeps its legacy willingness to
pay; seller price increases do not automatically raise buyer payouts. Buyer economics
and vendor resale arbitrage should be reviewed separately before changing that policy.

Existing auctions retain their recorded prices. Deploy the reviewed module and its
server manifest pin, enable the feature in the realm's configuration override, then
plan an explicit refresh of existing bot auctions. `.ahbot empty` removes bot auctions
and refunds bids; it is an operational action, not part of enabling this setting.
No database migration or client patch is needed.

Run the isolated economics checks with:

```sh
c++ -std=c++17 -Wall -Wextra -Werror tests/crafting_pricing_test.cpp -o /tmp/crafting-pricing-test
/tmp/crafting-pricing-test
```

### Enchanting scrolls

Recipe pricing also recognizes permanent enchant-item effects that produce a scroll.
Each scroll costs its full enchant reagents **plus one compatible vellum**. Vellum
class and minimum level follow the core's enchant casting rules; the cheapest valid
vellum recipe is used. Reusable rods are not consumed. Scrolls are priced as one
output, regardless of enchant bonus points. This closes the ordinary create-item
scan's omission of enchanting scrolls. It does not independently raise raw material
prices or change buyer payouts.

### Recipe items

With `AuctionHouseBot.RecipePricing.Enabled`, a recipe item (pattern, plans, formula,
schematic...) lists for at least `ProducedItemPercent` (default 50) of one craft's
seller reference: the crafted item's recipe-aware quote, BiS premium included, times
its guaranteed output. Recipes for items in the progression BiS catalog use
`BisProducedItemPercent` (default 100), so a raid BiS pattern costs about as much as
the item it makes. Formulas follow their enchant scroll. This only raises the ordinary
recipe quote. A recipe anyone can buy from an NPC for gold (no reputation requirement,
unlimited stock, no token cost) never goes above that NPC price. Seller only; it runs
on the same seller path as crafted pricing, so it needs CraftedPricing,
ProgressionPricing or Seller.StablePrices enabled.

```sh
c++ -std=c++17 -Wall -Wextra -Werror tests/recipe_pricing_test.cpp -o /tmp/recipe-pricing-test
/tmp/recipe-pricing-test
```

### Mounts, epic gear, glyphs, ammo, vendor and disenchant floors

Each is off by default; see `conf/mod_ahbot.conf.dist` for the keys.

- `MountPricing`: a fixed gold-sink price per riding tier (75/150/225/300), shared by
  seller and buyer so a bot mount can't be flipped to the buyer. Crafted mounts keep
  their crafting floor.
- `EpicGearFloor`: epic armor and weapons list for at least
  `Gold * (ItemLevel / ItemLevel_ref) ^ Exponent` (100g at item level 66 by default).
- `GlyphPricing`: separate minimums for major and minor glyphs.
- `ProjectilePricing`: ammo never lists below its NPC purchase price per unit.
- `VendorPriceFloor`: anything an NPC sells for gold never lists below that NPC price
  (bags excepted), so the AH doesn't undercut vendor gold sinks.
- `DisenchantFloor`: disenchantable gear never lists below the expected value of its
  disenchant materials at the bot's own prices.
- `ListingCap.MaxPerItem`: the most copies of one item random filling keeps in a house,
  so a class/quality bucket with only a few items can't flood it.

All but `MountPricing` only raise the seller's quote. The epic gear, glyph and ammo floors
also feed recipe pricing through the crafted reference. TCG/promo items are best kept off the AH with `DisabledCustomItemIDs`.

```sh
c++ -std=c++17 -Wall -Wextra -Werror tests/floor_pricing_test.cpp -o /tmp/floor-pricing-test
/tmp/floor-pricing-test
```

## Progression BiS demand

`AuctionHouseBot.ProgressionPricing.Enabled = true` applies a configurable premium
to items in the [published phase-BiS catalog](data/progression-bis/README.md).
`AuctionHouseBot.ProgressionPricing.BisMultiplier = 1.5` means a 50% premium and is
an initial tuning choice, not a measured market equilibrium. The premium applies
to crafted and noncrafted items, to bids and buyouts, after ordinary random pricing
and item overrides. Crafted floors and intermediate references include each item's
own premium once. Recommending an item for multiple specs/phases never stacks it.

Individual Progression is per character while auctions are shared, so the catalog
uses the **union of all phases** rather than a realm-wide current phase or the buyer's
personal progress. The stored phase/state tags make the lists auditable; they do not
change progression gates. This is published Classic/TBC/Wrath guidance, as requested,
not a simulation of Evermore's custom talents/itemization. Source omissions and identity
normalization are documented alongside the catalog. Only the existing seller candidate
rules determine whether an item can be listed; including a BoP item in the reference
list does not enable it on the AH. Buyer valuations remain unchanged.

Both features can be enabled independently, reload their caches with `.ahbot reload`,
and leave recorded auctions unchanged until an explicit refresh. Over-cap prices are
skipped and stacks remain within uint32 auction totals. New code still requires a
reviewed server pin, rebuild and restart before a live refresh. No schema/client changes.

To check the generated runtime membership against the source catalog:

```sh
python3 tools/build_bis_header.py --check
python3 -m unittest discover -s tests -p 'test_*.py'
```

## Stable faction prices and naturally supplied commodities

`AuctionHouseBot.Seller.StablePrices = true` gives new Alliance and Horde listings
identical per-unit bids and buyouts from the deterministic upper configured quote.
Stack counts may differ, so compare unit prices rather than whole-stack totals. Both
faction houses use the higher of their auction cuts for crafting floors; neutral
auctions keep their own fee. Craft/progression settings continue to apply and buyer
willingness to pay is unchanged. With all seller features off, legacy RNG ordering
is preserved. This setting defaults off for other realms and reloads normally.

Crafted pricing excludes optional conversion floors for trade goods that have an
independent loot/gathering supply route. Skinning leather is priced from its normal
commodity quote rather than requiring upgrades all the way from ruined scraps.
Otherwise the scraps-to-light-to-medium-to-heavy-to-thick-to-rugged chain compounds
conversion costs/margins and makes ordinary leather gear cost thousands of gold.
Finished gear and crafted-only intermediates still receive ingredient-cost floors.

Independent supply is detected from world skinning, fishing, disenchanting, prospecting,
milling, creature, gameobject and reference loot templates, restricted to trade goods.
These records establish a catalog supply route, not a guarantee of current player
access, stock or demand. If no supply rows are returned, commodity conversion floors
are disabled conservatively for that cache build; finished goods still use stable
commodity references. The source query runs only when rebuilding the pricing cache.

This change corrects future valuations. Recorded old/inflated auctions must be
refreshed after rebuilding/restarting the reviewed module and applying its server
pin/settings. Until that operational refresh is done, the professions addon can still
show large asking-price margins from old stock. Such margins are estimates, not
proof that a player or the AH buyer bot will pay that price.

## Shared seller price catalog

`src/AuctionHouseBotPriceCatalog.h` exposes an optional in-process API to other static
worldserver modules. `Quote(item, house)` returns copper per unit from the exact stable
seller calculation, including crafted floors and BiS demand. It returns zero when stable
selling is disabled, the item is excluded from candidate/proportion lists, its house is
disabled or its price cannot be listed. Quotes are cached by item and house and cleared
on configuration/candidate reload. No price-table SQL or duplicate pricing formula is used.

RetailAH's additive `KC` request shares these references with addons away from an
auctioneer. They describe future bot listings, not current availability or guaranteed
player sale proceeds. Existing auctions keep their persisted prices until refreshed.
Both modules must be built into the same worldserver; independent dynamic libraries are
not supported by this inline provider API. Buyer payouts are unchanged.

## BiS buyer pricing

Opt in with `AuctionHouseBot.Buyer.ProgressionPricing.Enabled = true`; set
`AuctionHouseBot.Buyer.ProgressionPricing.MarketPercent = 90` for an inclusive gross
per-unit offer of floor(90% of the bot's stable seller reference). Valid integer percentages
are 1..99; invalid values revert to 90. Stable seller pricing, progression pricing and an
active buyer are required. Targeted BiS items without a valid seller reference fail closed;
non-BiS items and disabled policy keep the legacy buyer calculation. Player asking prices
cannot raise the reference. AcceptablePriceModifier is not multiplied into this new offer.

The actual buying path and optional shared `BuyerOffer` API use the same method. Vendor
and zero-vendor-price guards apply, and targeted vendor caps are per unit for stacks. Bid
totals remain within uint32 auction money limits. Exact-threshold buyouts are accepted; one
copper above the stack offer is not. The reference still includes crafting floors and BiS
demand. A 15% crafting floor and 90% offer leave about 3.5% after the matching auction cut
before deposits and copper rounding, with more where a demand premium dominates. This is
not a guarantee for every recipe, live material availability or arbitrary player material asks.

Buying still follows configured buyer cycles/candidate limits and normal mail delivery. It
is not an instant-sale endpoint. Existing lower-priced bot stock should be refreshed during
coordinated deployment. Ship the matched RetailAH and RetailProfessions updates so displayed
offers and BiS craft profits agree with actual buying behavior. No SQL/client patch is needed.

## Crafting material supply

`AuctionHouseBot.MaterialStock.Enabled` stocks actual consumed profession ingredients
and enchanting vellum independently in each enabled house, using loaded server
spell/skill/item stores. This includes meat, fish, gems, vendor ingredients and
intermediate crafts. Bound/quest-bound, missing, temporary and conjured items are
excluded and logged. Script-only ingredients absent from spell metadata need separate
support. Use `MaterialStock.ExcludedItemIDs` for intentional exclusions: the dedicated
catalog bypasses restrictions intended for random stock.

### Material targets and batch replenishment

Without material targets, `MinUnits`/`RefillUnits` default to 20/20 for compatibility.
With `UseMaterialTargets=true`, normal/uncommon ingredients use Common targets
(default minimum 40, refill 100); rare-or-higher quality ingredients use Rare targets
(default 5/20). Quality is a starting proxy, not a claim about live demand or scarcity.
`RecipeAwareTargets=true` raises each minimum to at least the largest quantity of
that ingredient consumed by a profession spell and each refill target to at least
twice that quantity. Repeated reagent slots add together. Quantities cap at 10000.
Explicit `ItemTargets` records (`2589:80:200,12363:5:20`) override calculated targets;
use item:min:refill with no whitespace, 1<=min<=refill<=10000. Invalid records warn;
last duplicate wins. Overrides apply only to eligible recipe ingredients/vellum.

Falling **below** the minimum starts a refill batch. A per-house latch keeps adding
up to 20 units per material per cycle, respecting lower template/configured caps,
until the high target is reached. This continues across cycles even after crossing
the minimum, avoiding a tiny replacement every sale. After reaching the target,
no batch restarts until below the minimum again. Reload/restart resets batch latches;
stock between minimum and refill is sufficient until it next drops below minimum.
A rotating cursor prevents unpriceable or busy materials starving later ingredients.

### Give player sellers room

With `RespectPlayerListings=true` and `Seller.StablePrices=true`, unexpired player
buyout quantities priced **at or below** the existing stable unit reference count
alongside bot supply. Exact stack comparison avoids rounding down an overpriced ask.
The combined quantity reduces the refill deficit; player supply alone reaching the
minimum cancels an active refill batch. Expensive or bid-only player listings cannot
block baseline availability. No auction price is copied into the pricing model and
bots do not reactively undercut player asks. Without stable pricing, player protection
warns and remains inactive. Existing bot stock is left to sell/expire naturally.

Managed materials are excluded from **all random filling**, including multiplied
listings, so random stock cannot bypass material targets or player protection. Other
items retain random filling. The faction MaxItems remains the random-stock cap;
bounded material reserves can exceed it if a house full of duplicates would otherwise
block ingredients. All additions share ItemsPerCycle, with normal deficits first.
Neutral remains disabled when its maximum is zero. Shared cross-faction houses require
the existing corresponding shared-house configuration rather than separate stocks.

### Occasional discoveries

Opt-in `MaterialStock.PricingErrors.Enabled` occasionally lists **one** random material
stack at the NPC **sell** value per unit. `VendorPercent=100` means exactly that value;
1..99 permits lower prices, rounded to at least 1 copper. No-vendor-value materials are
excluded. This deliberate listing-only exception bypasses recipe floors; normal shared
reference quotes and buyer valuations remain unchanged. The default is a 5% chance per
eligible sell cycle after a 60-minute per-house cooldown, with at most one deliberate live
pricing-error listing. Any live or queued vendor-value-or-less normal bot supply
suppresses a new pricing-error event; normal supply itself is not restricted by
the event cap. Bargain stacks contain at most20 units and
respect lower configured/template stack limits.

Opt-in `MaterialStock.LargeStacks.Enabled` occasionally creates **one or two** random
material listings with 21..100 units at normal stable prices and crafting floors.
Default chance is 2% after a 180-minute per-house cooldown; at most two oversized bot
material listings may be live in a house. `MaxUnits` controls the upper size. These
explicitly bypass the ordinary AH listing cap of 20, but never the loaded item template
or auction money limits. A stack-size module can make those larger templates available;
items that cannot stack beyond 20 never produce oversized listings.

Both events avoid any ingredient with an active player buyout, require stable prices,
wait behind normal replenishment, and default to 15-minute listings. Their cooldowns
start with an initial delay after restart and survive config reload. Active vendor-value
or oversized bot stock is recognized conservatively after restart, so legacy supply
can postpone new events. Special stock counts in later supply checks and can temporarily
exceed a material's normal refill target. No existing auctions are deleted or repriced.
"Pricing error" is used in config/logs; item names and auction UI labels stay unchanged.

These are periodic supply rules, not an instantaneous unlimited availability guarantee.
No SQL, client patch or addon update is required. Build/restart and in-game verification
remain deployment steps; inspect logs and live faction stock after deployment.

### Buyer diagnostics and input validation

`.ahbot status` is available to GMs and the console. Stock counts use unexpired
buyout listings and the same affordable-player-supply rules as replenishment.
Refill-needed counts include new shortages and unfinished batches; they describe
remaining demand, not a promise to list everything in the next cycle. Seller cap
zero disables selling in that house. Legacy buying can continue, but targeted BiS
buying fails closed because its reference quote requires a positive seller cap.

Buyer counters accumulate in memory since process start, including across config
reloads. Scan exclusions can count the same player auction on multiple cycles.
`ineligible` includes expired/empty listings, protected player bids, missing templates
and items excluded by the vendor-value setting. `no offer` counts unavailable BiS
offers; `missing` counts absent item instances. Attempted rejections distinguish the
vendor safeguard from an unaffordable asking price. Bot-owned/bot-bid auctions are
excluded from these diagnostics. Purchases and bids count submitted actions, not
independently audited database commits.

Buyer candidate filtering happens before random selection, so expired or known
ineligible auctions do not consume the configured candidate budget. Vendor limits
compare the whole stack against unit vendor value times stack size, for both legacy
and BiS buying. Existing vendor-value policy is otherwise unchanged.

Cycle and candidate ranges accept an unsigned integer or `min:max`, with optional
surrounding spaces. Empty, negative, reversed, trailing-junk and overflowing values
are rejected with a log message and retain the previous valid value (startup default:
1). Cycle intervals must be positive. Buyer candidates accept `0` or `0:0` to disable
attempts; mixed zero/positive ranges are rejected.

### Bag seller prices

With crafted pricing enabled, finished crafted bags use their recipe margin floor
instead of the ordinary container/quality markup. Material replacement references,
recursive intermediate costs, configured crafting margin and effective auction cut
remain authoritative; this is a listing-price estimate, not a guaranteed player sale.

Actual gold vendor offers impose a second limit on all container bags: 76% of the
cheapest vendor purchase price for the same slot count, bag family and subclass.
This beats a 20% reputation-discounted purchase by another 5%. The comparison uses
loaded BuyPrice/BuyCount and actual zero-ExtendedCost npc_vendor rows, never SellPrice.
Token purchases and bags with no actual vendor offer do not create a gold ceiling.
Specialty bags cannot be treated as ordinary general storage. Vendor accessibility
is deliberately conservative: faction, limited stock and progression can make a
particular vendor unavailable to an individual player.

If the material-profit floor exceeds the vendor ceiling, the bot withholds that bag
from new listings and returns no catalog quote. It never caps below the floor to
pretend crafting is profitable. Some small cosmetic bags need cheaper ingredients
or a deliberate premium exception before both goals can be met. Ingredient pricing
is unchanged by this policy. Unsupported/cyclic bag recipes are also withheld.
Existing auctions retain their stored prices until expired or explicitly refreshed
after deployment. Buyer pricing is unchanged. This requires a reviewed module merge,
an exact merged-commit wow-server pin and a rebuild/restart before taking effect.
