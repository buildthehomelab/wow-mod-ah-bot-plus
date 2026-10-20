#include "../src/BuyerSafety.h"
#include <cassert>

int main()
{
    using namespace AhBotSafety;
    std::uint32_t low = 3, high = 5;
    for (auto text : {"", " ", "-1", "+1", "1x", "1:", ":2", "1:2:3", "6:5", "4294967296", "0", "0:2"})
    {
        assert(!ParseRange(text, low, high, false));
        assert(low == 3 && high == 5);
    }
    assert(ParseRange(" 2 : 7 ", low, high, false) && low == 2 && high == 7);
    assert(ParseRange("9", low, high, false) && low == 9 && high == 9);
    assert(ParseRange("4294967295", low, high, false) && low == UINT32_MAX && high == UINT32_MAX);
    assert(ParseRange("0:0", low, high, true) && low == 0 && high == 0);
    assert(!ParseRange("0:1", low, high, true) && low == 0 && high == 0);
    assert(ParseRange("0", low, high, true));
    // A 20-unit stack at 9 copper/unit is safe under a 10-copper vendor unit cap.
    assert(!AboveVendorLimit(180, 10, 20));
    assert(!AboveVendorLimit(200, 10, 20));
    assert(AboveVendorLimit(201, 10, 20));
    assert(AboveVendorLimit(11, 10, 1));
    assert(!AboveVendorLimit(11, 0, 1));
    assert(!AboveVendorLimit(UINT64_MAX, UINT32_MAX, 20)); // sentinel: no vendor
    assert(!AboveVendorLimit(std::uint64_t(UINT32_MAX - 1) * 100, UINT32_MAX - 1, 100));
    assert(CandidateEligible(101, 100, 20, false, false, false, true, true, true));
    assert(!CandidateEligible(100, 100, 20, false, false, false, true, true, true));
    assert(!CandidateEligible(99, 100, 20, false, false, false, true, true, true));
    assert(!CandidateEligible(101, 100, 0, false, false, false, true, true, true));
    assert(!CandidateEligible(101, 100, 20, true, false, false, true, true, true));
    assert(!CandidateEligible(101, 100, 20, false, true, false, true, true, true));
    assert(CandidateEligible(101, 100, 20, false, true, true, true, true, true));
    assert(!CandidateEligible(101, 100, 20, false, false, false, false, true, true));
    assert(!CandidateEligible(101, 100, 20, false, false, false, true, false, true));
    assert(CandidateEligible(101, 100, 20, false, false, false, true, false, false));
}
