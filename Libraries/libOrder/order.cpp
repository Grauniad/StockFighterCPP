#include "order.h"

OrderStatus::OrderStatus()
    : totalFilled(0),
      remainingQuantity(0)
{
}

OrderDetails::OrderDetails(
     size_t internalId,
     size_t externalId,
     const OrderDefinition& details)
   : OrderDefinition(details), internalOrderId(internalId), exchangeOrderId(externalId)
{
}

OrderDetails::OrderDetails()
    : internalOrderId(0),
      exchangeOrderId(0)
{
}

template<>
OrderStatus::OrderState::Map OrderStatus::OrderState::string_map = { 
    {"draft", OrderStatus::ORDER_STATUS_DRAFT},
    {"sent", OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE},
    {"live", OrderStatus::ORDER_STATUS_ON_EXCHANGE},
    {"complete", OrderStatus::ORDER_STATUS_COMPLETED},
    {"cancelling", OrderStatus::ORDER_STATUS_CANCEL_SENT_TO_EXCHANGE},
    {"rejected", OrderStatus::ORDER_STATUS_EXCHANGE_REJECTED},
    {"error", OrderStatus::ORDER_STATUS_SYSTEM_REJECTED},
    {"", OrderStatus::ORDER_STATUS_ERROR}
};
