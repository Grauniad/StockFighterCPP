#ifndef STOCK_FIGHTER_ORDER_H__
#define STOCK_FIGHTER_ORDER_H__

#include <cstdlib>
#include<string>
#include <OrderPost.h>

/***
{
  "ok": true,
  "symbol": "FAC",
  "venue": "OGEX",
  "direction": "buy",
  "originalQty": 100,
  "qty": 20,   // this is the quantity *left outstanding*
  "price":  5100, // the price on the order -- may not match that of fills!
  "type": "limit",
  "id": 12345, // guaranteed unique *on this venue*
  "account" : "OGB12345",
  "ts": "2015-07-05T22:16:18+00:00", // ISO-8601 timestamp for when we received order
  "fills":
    [
      {
        "price": 5050,
        "qty": 50
        "ts": "2015-07-05T22:16:18+00:00"
      }, ... // may have zero or multiple fills.  Note this order presumably has a 

total of 80 shares worth 
    ],
  "totalFilled": 80,
  "open": true
}

*/

/**
 * Static details for an order.
 */

struct OrderDetails: public OrderDefinition {
    /************************************************************
     *                    Data
     ************************************************************/
    std::string   rejectionReason;
    size_t        internalOrderId;
    size_t        exchangeOrderId;

    /************************************************************
     *                    Initialisation
     ************************************************************/
    OrderDetails();
    OrderDetails(size_t internalId, size_t externalId, const OrderDefinition& details);

    OrderDetails(const OrderDetails& rhs) = default;
    OrderDetails& operator=(const OrderDetails& rhs) = default;

    OrderDetails(OrderDetails&& rhs) = default;
    OrderDetails& operator=(OrderDetails&& rhs) = default;
};

struct OrderStatus {
    /************************************************************
     *                    Type Definitions
     ************************************************************/
    enum ORDER_STATUS {
        ORDER_STATUS_DRAFT,
        ORDER_STATUS_SENT_TO_EXCHANGE,
        ORDER_STATUS_ON_EXCHANGE,
        ORDER_STATUS_COMPLETED,
        ORDER_STATUS_CANCEL_SENT_TO_EXCHANGE,
        ORDER_STATUS_EXCHANGE_REJECTED,
        ORDER_STATUS_SYSTEM_REJECTED,
        ORDER_STATUS_ERROR
    };
    typedef EnumValue<ORDER_STATUS,ORDER_STATUS_ERROR> OrderState;

    /************************************************************
     *                    Data
     ************************************************************/

    OrderState  state;
    long        totalFilled;
    long        remainingQuantity;
    /************************************************************
     *                    Initialisation
     ************************************************************/

    OrderStatus();

    OrderStatus(const OrderStatus& rhs) = default;
    OrderStatus& operator=(const OrderStatus& rhs) = default;

    OrderStatus(OrderStatus&& rhs) = default;
    OrderStatus& operator=(OrderStatus&& rhs) = default;
};

#endif
