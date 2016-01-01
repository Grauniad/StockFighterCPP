#ifndef STOCK_FIGHTER_EXCHANGE_ORDER_EXCHANGE_ORDERS_H__
#define STOCK_FIGHTER_EXCHANGE_ORDER_EXCHANGE_ORDERS_H__

#include <StockFighterConnection.h>
#include <unordered_map>
#include <AccountConfig.h> 
#include <ExchangeInterface.h> 
#include <ExchangeOrder.h>
#include <IPostable.h>

class ExchangeOrderManager {
public:
     ExchangeOrderManager(
         StockFighterAPI& api,
         IPostable&       event_loop,
         const AccountConfig& account,
         Exchange& exchange);

     /**
      * Create a new draft exchange order, on the current exchange.
      *
      * If order is not for a known stock on the exchange the order will
      * be rejected by the manager (without sending to the exchange) and 
      * a nullptr will be returned.
      */
     OrderHdl NewOrder(size_t internalId, const OrderDefinition& order);

     /**
      * Lookup an order as identified by the exchange.
      *
      * If we don't know about the order yet, a nullptr is returned.
      */
     OrderHdl GetExchangeOrder(size_t exchangeOrderId);
private:
     /**
      * Utility method to map an order from its exchange order Id to its
      * order instance.
      */
     void MapOrder(const OrderHdl& order);

     /**
      * Handles updates to this order, updating our internal data as
      * appropriate.
      */
      void HandleOrderUpdates(OrderHdl order);

      /**
       * Stop processing updates for this order. 
       *
       * Called to prvent unessecary work once we have all information we
       * require.
       */
      void StopHandlingOrderUpdates(const OrderHdl& order);

     StockFighterAPI&     api;
     IPostable&           event_loop;
     const AccountConfig& account;
     Exchange&            exchange;
     // Maps from the *exchange* identifier to the order.
     std::map<size_t,OrderHdl>  exchangeOrders;

     // Maps from the *internal* identifier to the update client. Note that we must 
     // capture the order reference to keep it alive until we are done with it.
     struct UpdateClient{
         // Order here is important, the client must be cleaned up *before* the order
         OrderHdl  order;
         OrderUpdateClient client;
     };
     std::map<size_t,UpdateClient>  updateClients;
};

#endif
