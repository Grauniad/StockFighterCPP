#include "AccountOrderManager.h"

AccountOrderManager::AccountOrderManager(StockFighterAPI& api,
                                         IPostable&       event_loop,
                                         const AccountConfig& _account,
                                         Exchanges& _exchanges)
    : account(_account)
{
    for (Exchange* e : _exchanges.ExchangeList()) {
        const std::string id = e->Id();
        exchanges.emplace(std::piecewise_construct,
                          std::forward_as_tuple(id),
                          std::forward_as_tuple(api,event_loop,account,*e));
    }
}

OrderHdl AccountOrderManager::NewOrder(size_t internalId, const OrderDefinition& order) 
{
    OrderHdl newOrder(nullptr);

    if ( account.AccountCode() == order.account) {
        auto it = exchanges.find(order.venue);

        if ( it != exchanges.end() ) {
            newOrder = it->second.NewOrder(internalId,order);
        }
    }

    return newOrder;
}

ExchangeOrderManager* AccountOrderManager::GetExchangeManager(
                          const Exchange& exchange)
{
    ExchangeOrderManager* e = nullptr;

    auto it = exchanges.find(exchange.Id());

    if ( it != exchanges.end() ) {
        e = &it->second;
    }

    return e;
}
