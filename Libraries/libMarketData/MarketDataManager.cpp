#include "MarketDataManager.h"

MarketDataManager::MarketDataManager(
        StockFighterAPI& _api,
        Exchanges& _exchanges,
        IPostable& _event_loop)
    : api(_api), exchanges(_exchanges), event_loop(_event_loop)
{
}

MarketDataProvider& MarketDataManager::GetMarketDataProvider(
                         const AccountConfig& account)
{
    const std::string acc = account.AccountCode();
    auto it = providers.find(acc);

    if (it == providers.end()) {
        auto& exchangeList = exchanges.ExchangeList();
        it = providers.emplace(
                 std::piecewise_construct,
                 std::forward_as_tuple(acc),
                 std::forward_as_tuple(api,account,exchangeList,event_loop)
             ).first;
    }

    return it->second;
}
