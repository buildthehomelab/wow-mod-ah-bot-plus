#ifndef AH_BOT_BUYER_SAFETY_H
#define AH_BOT_BUYER_SAFETY_H

#include <charconv>
#include <cstdint>
#include <string_view>

namespace AhBotSafety
{
// Parse the entire field, retaining the caller's previous values on any error.
// Zero is supported for disabling buyer candidates, but never for cycle intervals.
inline bool ParseRange(std::string_view text, std::uint32_t& low, std::uint32_t& high, bool allowZero)
{
    auto trim = [](std::string_view value) {
        auto first = value.find_first_not_of(" \t");
        if (first == std::string_view::npos)
            return std::string_view{};
        return value.substr(first, value.find_last_not_of(" \t") - first + 1);
    };
    auto parse = [&](std::string_view value, std::uint32_t& output) {
        value = trim(value);
        if (value.empty())
            return false;
        auto result = std::from_chars(value.data(), value.data() + value.size(), output);
        return result.ec == std::errc{} && result.ptr == value.data() + value.size();
    };
    auto colon = text.find(':');
    std::uint32_t parsedLow = 0, parsedHigh = 0;
    if (!parse(text.substr(0, colon), parsedLow))
        return false;
    if (colon == std::string_view::npos)
        parsedHigh = parsedLow;
    else if (!parse(text.substr(colon + 1), parsedHigh))
        return false;
    // Candidate ranges cannot mix disabled and enabled values.
    if (parsedHigh < parsedLow || (!allowZero && !parsedLow) || (!parsedLow && parsedHigh))
        return false;
    low = parsedLow;
    high = parsedHigh;
    return true;
}

inline bool AboveVendorLimit(std::uint64_t price, std::uint32_t unitValue, std::uint32_t count)
{
    return unitValue && unitValue != UINT32_MAX && price > std::uint64_t(unitValue) * count;
}

inline bool CandidateEligible(std::int64_t expires, std::int64_t now, std::uint32_t count,
    bool botOwnerOrBidder, bool hasBid, bool bidAgainstPlayers, bool hasTemplate,
    bool hasVendorValue, bool skipWithoutVendorValue)
{
    return expires > now && count && !botOwnerOrBidder && (!hasBid || bidAgainstPlayers)
        && hasTemplate && (hasVendorValue || !skipWithoutVendorValue);
}
}
#endif
