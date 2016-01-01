#include "OrderManager.h"

OrderManager::OrderManager(
        StockFighterAPI&   api,
        IPostable&         event_loop,
        LevelConfig&       config,
        Exchanges&         exchanges)
    : nextId(1)
{
    for (const AccountConfig& acc : config.AccountsConfig()) {
        const std::string id = acc.AccountCode();
        accounts.emplace(std::piecewise_construct,
                          std::forward_as_tuple(id),
                          std::forward_as_tuple(api,event_loop,acc,exchanges));
    }
}

OrderHdl OrderManager::NewOrder(const OrderDefinition& order) {
    OrderHdl hdl(nullptr);

    auto it = accounts.find(order.account);

    if ( it != accounts.end() ) {
        hdl = it->second.NewOrder(AllocateNextId(),order);
    }

    if ( hdl != nullptr ) {
        orders[hdl->Details().internalOrderId] = hdl;
    }

    return hdl;
}

size_t OrderManager::AllocateNextId() {
    return nextId++;
}

AccountOrderManager* OrderManager::GetAccountManager(
                         const AccountConfig& account)
{
    AccountOrderManager* manager = nullptr;

    auto it = accounts.find(account.AccountCode());

    if ( it != accounts.end() ) {
        manager = &it->second;
    }

    return manager;
}

OrderHdl OrderManager::GetOrder(size_t internalId) {
    OrderHdl order(nullptr);
    auto it = orders.find(internalId);

    if ( it != orders.end() ) {
        order = it->second;
    }

    return order;
}
