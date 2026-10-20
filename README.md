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

## Crafting material reserves

Enable `AuctionHouseBot.MaterialStock.Enabled` to replenish actual consumed profession
reagents and enchanting vellum on **each enabled auction house**, independently. The
catalog comes from the loaded server spell/skill/item stores, so realm recipe overrides
are included. This covers material ingredients, including vendor reagents, meat, fish,
gems and intermediate crafts; it does not add every finished craft or every trade-good
item merely because of its class. Script-only ingredients absent from spell data need
separate support. Bound/quest-bound, missing, temporary and conjured items are excluded.
Startup/reload logs show catalog and excluded IDs; failed pricing is logged per house.

`MaterialStock.MinUnits` defaults to 20 (range 1–10000). Each sell cycle scans that
house's live in-memory auctions once and counts unexpired bot-owned buyout quantities.
Player asking prices cannot prevent the reserve. Missing materials take priority over
random stock; a rotating cursor prevents busy or unpriceable materials starving others.
Each deficient material gets at most one replenishment listing per cycle, up to its
unit deficit and the template/configured stack cap. Large deficits or unstackable
materials take several cycles. Total additions still respect `ItemsPerCycle`.

The material reserve can exceed faction `MaxItems`; that setting continues to cap
random filling, so a house full of duplicates cannot block material coverage. No old
or player listings are deleted. Existing plentiful material listings count toward the
reserve; buying or expiry makes stock eligible for replacement on a later sell cycle.
Neutral remains disabled when its maximum is zero. With cross-faction auctions enabled,
the core uses the shared house rather than separate faction stocks.

The dedicated material catalog deliberately bypasses exclusions intended for random
stock (`DisabledItemIDs`, category weights, item levels and no-vendor-price filtering),
so ordinary recipe ingredients are not silently omitted. Use the separate
`MaterialStock.ExcludedItemIDs` to intentionally exclude a material. Existing pricing,
crafting floors and money limits still apply; an unpriceable item is reported and
skipped instead of sold at an invalid price. Keep stable seller pricing enabled to
make new reserve prices match addon reference quotes. This is periodic replenishment,
not an instantaneous or unlimited availability guarantee.
