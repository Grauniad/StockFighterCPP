#include "MarketDataProvider.h"

MarketDataProvider::MarketDataProvider(
    StockFighterAPI& api,
    const AccountConfig& _account,
    const Exchanges::ExchangeRefList& _exchanges,
    IPostable& event_loop)
    : account(_account.AccountCode())
{
    for (Exchange* pexchange: _exchanges) {
        Exchange& e = *pexchange;
        exchanges.emplace(std::piecewise_construct,
                          std::forward_as_tuple(e.Id()),
                          std::forward_as_tuple(api,_account,e,event_loop));
    }
}

MarketDataClient MarketDataProvider::NewUpdatesClient(
                         const std::string& exch,
                         const std::string& stock)
{
    MarketDataClient client(nullptr);

    auto it = exchanges.find(exch);

    if (it != exchanges.end()) {
        client = it->second.NewUpdatesClient(stock);
    }

    return client;
}
