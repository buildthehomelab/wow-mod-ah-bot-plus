#include "../src/MaterialStock.h"
#include <cassert>
#include <limits>

int main()
{
    using namespace AhBotStock;
    std::vector<std::uint32_t> materials{10,20,30,40};
    std::map<std::uint32_t,Policy> policies{{10,{40,100}},{20,{40,100}},{30,{5,20}},{40,{40,100}}};
    std::map<std::uint32_t,std::uint32_t> alliance{{10,40},{20,39},{30,4}},horde,players;
    std::set<std::uint32_t> ar,hr;
    std::size_t ac=0,hc=0;
    auto a=Plan(materials,policies,alliance,players,ar,2,ac);
    assert(a.size()==2 && a[0].item==20 && a[0].deficit==61 && a[1].item==30 && a[1].deficit==16);
    auto h=Plan(materials,policies,horde,players,hr,2,hc);
    assert(h.size()==2 && h[0].item==10 && h[0].deficit==100);
    a=Plan(materials,policies,alliance,players,ar,1,ac);
    assert(a.size()==1 && a[0].item==40); // Unlistable early IDs cannot starve later ones.
    alliance[20]=59; ac=0;
    a=Plan({20},policies,alliance,players,ar,500,ac);
    assert(a.size()==1 && a[0].deficit==41); // Continue a capped refill above low threshold.
    alliance[20]=100;
    assert(Plan({20},policies,alliance,players,ar,500,ac).empty() && !ar.count(20));
    alliance[20]=99;
    assert(Plan({20},policies,alliance,players,ar,500,ac).empty());
    alliance[20]=39;
    assert(Plan({20},policies,alliance,players,ar,500,ac).front().deficit==61);
    players[20]=40;
    assert(Plan({20},policies,alliance,players,ar,500,ac).empty() && !ar.count(20)); // Cancel for player supply.
    players[20]=10;
    assert(Plan({20},policies,alliance,players,ar,500,ac).empty());
    alliance[20]=20;
    assert(Plan({20},policies,alliance,players,ar,500,ac).front().deficit==70);
    players.clear(); // A sale/expiry permits replenishment on the next live snapshot.
    assert(Plan({20},policies,alliance,players,ar,500,ac).front().deficit==80);
    Count(alliance,20,UINT32_MAX,100); Count(alliance,20,UINT32_MAX,100);
    assert(alliance[20]==100);
    assert(Plan(materials,policies,alliance,players,ar,0,ac).empty());
    assert(Plan({},policies,alliance,players,ar,500,ac).empty());
    assert(Plan({999},policies,alliance,players,ar,500,ac).empty());
    ac=999999;
    assert(Plan({30},policies,{}, {},ar,500,ac).size()==1);
    auto bulk=Normalize(40,100,60),rare=Normalize(5,20,12),bounded=Normalize(UINT32_MAX,UINT32_MAX,UINT32_MAX);
    assert(bulk.minimum==60 && bulk.refill==120 && rare.minimum==12 && rare.refill==24);
    assert(Normalize(100,1).refill==100 && Normalize(0,0).minimum==1);
    assert(bounded.minimum==10000 && bounded.refill==10000);
    std::uint32_t id=0; Policy custom{};
    assert(ParseOverride("123:40:100",id,custom) && id==123 && custom.minimum==40 && custom.refill==100);
    for (auto invalid : {"", "1:0:20", "0:1:20", "1:40:20", "1:1:10001", "1:1:20:2", "-1:1:20", "1:1:20junk", "1::20", "4294967296:1:20"})
        assert(!ParseOverride(invalid,id,custom));
    assert(Affordable(200,20,10) && Affordable(199,20,10));
    assert(!Affordable(201,20,10)); // Reject one copper over reference, without truncation.
    assert(!Affordable(1,0,10) && !Affordable(0,1,10) && !Affordable(10,1,0));
    assert(Affordable(UINT64_MAX,UINT32_MAX,UINT64_MAX));
    assert(ListingUnits(Kind::Reserve,100,999,20,100)==20);
    assert(ListingUnits(Kind::PricingError,20,5,20,100)==5);
    assert(ListingUnits(Kind::LargeStack,100,999,20,100)==100);
    assert(ListingUnits(Kind::LargeStack,100,60,20,100)==60);
    assert(ListingUnits(Kind::LargeStack,100,20,20,100)==0);
    assert(ListingUnits(Kind::LargeStack,100,999,20,UINT32_MAX/10)==0);
    assert(ListingUnits(Kind::LargeStack,100,999,20,UINT32_MAX/40)==40);
    assert(ListingUnits(Kind::Reserve,20,999,20,0)==0);
    assert(VendorBargain(100,100)==100 && VendorBargain(100,50)==50);
    assert(VendorBargain(1,1)==1 && VendorBargain(0,100)==0 && VendorBargain(UINT32_MAX,100)==UINT32_MAX);
}
