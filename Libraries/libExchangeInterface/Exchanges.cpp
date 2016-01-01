#include <Exchanges.h>
#include <tuple>

Exchanges::Exchanges(
    StockFighterAPI& api,
    std::vector<VenueConfig>& venues)
{
    SetupExchanges(api,venues);
    WaitForInitialiseAll();
}

Exchange* Exchanges::GetExchange(const std::string& code) {
    Exchange* exch = nullptr;

    auto it = exchangeMap.find(code);

    if ( it != exchangeMap.end()) {
        exch = &(it->second);
    }

    return exch;
}

void Exchanges::SetupExchanges(
         StockFighterAPI& api,
         std::vector<VenueConfig>& venues)
{
    exchanges.reserve(venues.size());
    for (VenueConfig& cfg: venues) {

        auto result = exchangeMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(cfg.ExchangeCode()),
            std::forward_as_tuple(api,cfg));

        if (result.second) {
            Exchange& e = result.first->second;
            exchanges.push_back(&e);
        }
    }
}

void Exchanges::WaitForInitialiseAll() {
    for (Exchange* e : exchanges) {
        e->WaitForInitialisation();
    }
}

