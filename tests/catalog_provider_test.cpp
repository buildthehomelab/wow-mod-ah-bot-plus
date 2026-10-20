#include "../src/AuctionHouseBotPriceCatalog.h"
#include <cassert>
std::uint64_t OtherModuleQuote(std::uint32_t item, std::uint32_t house);
bool OtherModuleBuyerOffer(std::uint32_t item, std::uint32_t house, std::uint64_t& offer);
int main()
{
    assert(OtherModuleQuote(15094, 2) == 0);
    AhBotPriceCatalog::provider = [](std::uint32_t item, std::uint32_t house) -> std::uint64_t {
        return item == 15094 && (house == 2 || house == 6) ? 147099 : 0;
    };
    assert(OtherModuleQuote(15094, 2) == 147099);
    assert(OtherModuleQuote(15094, 6) == 147099);
    assert(OtherModuleQuote(15094, 7) == 0);
    std::uint64_t offer = 999;
    assert(!OtherModuleBuyerOffer(12783, 2, offer) && offer == 0);
    AhBotPriceCatalog::buyerProvider = [](std::uint32_t item, std::uint32_t house, std::uint64_t& value) {
        value = item == 12783 && (house == 2 || house == 6) ? 1908000 : 0;
        return item == 12783;
    };
    assert(OtherModuleBuyerOffer(12783, 2, offer) && offer == 1908000);
    assert(OtherModuleBuyerOffer(12783, 6, offer) && offer == 1908000);
    assert(OtherModuleBuyerOffer(12783, 7, offer) && offer == 0); // Fail closed; don't use legacy.
    assert(!OtherModuleBuyerOffer(8170, 2, offer) && offer == 0); // Legacy fallback for ordinary items.
    AhBotPriceCatalog::buyerProvider = nullptr;
    AhBotPriceCatalog::provider = nullptr;
    assert(OtherModuleQuote(15094, 2) == 0);
}
