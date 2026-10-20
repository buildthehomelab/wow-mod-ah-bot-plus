#include "../src/AuctionHouseBotPriceCatalog.h"
#include <cassert>
std::uint64_t OtherModuleQuote(std::uint32_t item, std::uint32_t house);
int main()
{
    assert(OtherModuleQuote(15094, 2) == 0);
    AhBotPriceCatalog::provider = [](std::uint32_t item, std::uint32_t house) -> std::uint64_t {
        return item == 15094 && (house == 2 || house == 6) ? 147099 : 0;
    };
    assert(OtherModuleQuote(15094, 2) == 147099);
    assert(OtherModuleQuote(15094, 6) == 147099);
    assert(OtherModuleQuote(15094, 7) == 0);
    AhBotPriceCatalog::provider = nullptr;
    assert(OtherModuleQuote(15094, 2) == 0);
}
