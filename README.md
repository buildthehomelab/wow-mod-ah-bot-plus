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

Supported recipes have a profession skill association, one ordinary create-item effect,
known positive reagents and no level-dependent yield or triggered/random creation.
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
