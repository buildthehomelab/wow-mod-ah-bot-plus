#include "../src/AuctionHouseBotPriceCatalog.h"
std::uint64_t OtherModuleQuote(std::uint32_t item, std::uint32_t house)
{
    return AhBotPriceCatalog::Quote(item, house);
}

bool OtherModuleBuyerOffer(std::uint32_t item, std::uint32_t house, std::uint64_t& offer)
{
    return AhBotPriceCatalog::BuyerOffer(item, house, offer);
}
