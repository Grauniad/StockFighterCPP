#ifndef STOCK_FIGHTER_ACCOUNT_ORDER_ACCOUNT_ORDERS_H__
#define STOCK_FIGHTER_ACCOUNT_ORDER_ACCOUNT_ORDERS_H__

#include <ExchangeOrders.h>
#include <Exchanges.h>

/**
 * Manages all orders created under a given account.
 * 
 * This is essentially just pass through since the API interface
 * is all at the exchange-account level.
 */
class AccountOrderManager {
public:
     /**
      * Setup the manager for each exchange...
      */
     AccountOrderManager(
         StockFighterAPI& api,
         IPostable&       event_loop,
         const AccountConfig& account,
         Exchanges& exchanges);

     /**
      * Create a new draft exchange order, on the specifiec exchange.
      *
      * If order is not for a known stock on a known exchange the order will be
      * rejected by the manager (without sending to the exchange) and a nullptr
      * will be returned.
      */
     OrderHdl NewOrder(size_t internalId, const OrderDefinition& order);

     /**
      * If the exchange is not know by the manager a null pointer will be
      * returtned.
      */
     ExchangeOrderManager* GetExchangeManager(const Exchange& exchange);
private:
     const AccountConfig& account;
     std::unordered_map<std::string,ExchangeOrderManager>  exchanges;
};

#endif
