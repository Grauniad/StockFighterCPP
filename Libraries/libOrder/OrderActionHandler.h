#ifndef STOCK_FIGHTER_LIBS_ORDERS_ACTION_HANDLER_H__
#define STOCK_FIGHTER_LIBS_ORDERS_ACTION_HANDLER_H__

#include <OrderManager.h>
#include <OrderValidation.h>

class OrderActionHandler {
public:
     OrderActionHandler(OrderManager& manager);

     /**
      *  Attempt to create a new order, and send it to the exchange.
      *
      *  If validation fails on the draft order, a system reject will
      *  be performed on the order.
      *
      *  If the order definition is so broken a draft couldn't be created a
      *  nullptr will be returned. (This is likely caused by attempting to
      *  trade a non-existant stock).
      */
     OrderHdl NewOrder(const OrderDefinition& order, std::string& error);

     typedef std::shared_ptr<ExchangeOrderValidator> ValidatorHdl;

     /**
      * Install a new validator into the action handler.
      */
     void InstallValidator(ValidatorHdl& validator);

private:
     OrderManager&          manager;
     std::set<ValidatorHdl> validators;
};

#endif
