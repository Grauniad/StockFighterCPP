#ifndef STOCK_FIGHTER_ORDERS_ORDER_MANAGER_H__
#define STOCK_FIGHTER_ORDERS_ORDER_MANAGER_H__

#include <AccountOrderManager.h>
#include <LevelConfig.h>

/**
 * Manages all orders created in the system.
 */
class OrderManager {
public:
     /**
      * Setup the manager for each account / exchange...
      */
     OrderManager(
         StockFighterAPI&   api,
         IPostable&         event_loop,
         LevelConfig&       config,
         Exchanges&         exchanges);

     /**
      * Create a new draft exchange order, on the specifiec exchange.
      *
      * If order is not for a known stock on a known exchange the order will be
      * rejected by the manager (without sending to the exchange) and a nullptr
      * will be returned.
      *
      * NOTE: The manager will do its best to create a new exchange order,
      *       even if the definition is nonsense. This method should only
      *       really be called by the action handler which will then do
      *       any validation / post-processing required.
      *
      */
     OrderHdl NewOrder(const OrderDefinition& order);

     /**
      * If the account is not know by the manager a null pointer will be
      * returtned.
      */
     AccountOrderManager* GetAccountManager(const AccountConfig& account);

     /**
      * Retrieve the specifed order 
      */
     OrderHdl GetOrder(size_t internalId);

private:  
     size_t AllocateNextId();
     std::unordered_map<std::string,AccountOrderManager>  accounts;
     std::unordered_map<size_t,OrderHdl>  orders;
     size_t nextId;
};

#endif
