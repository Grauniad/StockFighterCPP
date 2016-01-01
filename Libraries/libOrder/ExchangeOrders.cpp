#include "ExchangeOrders.h"

ExchangeOrderManager::ExchangeOrderManager(
    StockFighterAPI&     _api,
    IPostable&           _event_loop,
    const AccountConfig& _account,
    Exchange&            _exchange)
    : api(_api), event_loop(_event_loop), account(_account), exchange(_exchange)
{
}

OrderHdl ExchangeOrderManager::NewOrder(size_t internalId,
                                        const OrderDefinition& order)
{
    OrderHdl hdl (nullptr);

    if ( order.venue == exchange.Id()) {
        if (exchange.GetStock(order.symbol) != nullptr ) {
            hdl.reset(new ExchangeOrder(api, event_loop, internalId, order));
        }
    }

    if ( hdl != nullptr ) {
        HandleOrderUpdates(hdl);
    }
    

    return hdl;
}

OrderHdl ExchangeOrderManager::GetExchangeOrder(size_t exchangeOrderId) {
    OrderHdl order(nullptr);
    auto it = exchangeOrders.find(exchangeOrderId);

    if ( it != exchangeOrders.end() ) {
        order = it->second;
    }

    return order;
}

void ExchangeOrderManager::MapOrder(const OrderHdl& order) {
    size_t exchangeOrderId = order->Details().exchangeOrderId;
    exchangeOrders[exchangeOrderId] = order;
}

void ExchangeOrderManager::HandleOrderUpdates(OrderHdl order) {
    auto listenForExchangeAck = 
        [this, order] (const ExchangeOrder::OrderUpdate& update) -> void {
            if (update.oldStatus.state == 
                    OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE
                &&
                update.newStatus.state == 
                    OrderStatus::ORDER_STATUS_ON_EXCHANGE)
            {
                this->MapOrder(order);
            }

            if (update.newStatus.state == 
                    OrderStatus::ORDER_STATUS_ON_EXCHANGE ||
                update.newStatus.state ==
                    OrderStatus::ORDER_STATUS_EXCHANGE_REJECTED ||
                update.newStatus.state ==
                    OrderStatus::ORDER_STATUS_SYSTEM_REJECTED)
            {
                // No more data required, but we can't modify the client list
                // under publihser lock, so we need to setup a callback.
                auto stop = [this,order] () -> void {
                    this->StopHandlingOrderUpdates(order);
                };
                event_loop.PostTask(stop);
            }
                
        };
    auto& client = updateClients[order->Details().internalOrderId] = 
         UpdateClient{order, order->NewUpdatesClient()};
    client.client->OnNewMessage(listenForExchangeAck);
}

void ExchangeOrderManager::StopHandlingOrderUpdates(const OrderHdl& order) {
    auto it = updateClients.find(order->Details().internalOrderId);

    if ( it != updateClients.end() ) {
        updateClients.erase(it);
    }
}
